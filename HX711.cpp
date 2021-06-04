//
//    FILE: HX711.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.3
// PURPOSE: Library for Loadcells for UNO
//     URL: https://github.com/RobTillaart/HX711
//
//  HISTORY:
//  0.1.0   2019-09-04  initial release
//  0.1.1   2019-09-09  change long to float (reduce footprint)
//  0.2.0   2020-06-15  refactor; add price functions;
//  0.2.1   2020-12-28  add arduino-ci + unit test
//  0.2.2   2021-05-10  add read_median(), fix typo, add mode operandi
//  0.2.3   2021-05-26  add running_average() mode


#include "HX711.h"
#include <assert.h>

HX711::HX711(float* measurements, uint8_t measurementsCount, uint8_t dataPin, uint8_t clockPin)
  : _dataPin{dataPin}
  , _clockPin{clockPin}
  , measurements_{measurements}
  , measurementsCount_{measurementsCount}
{
  assert(0 < measurementsCount && measurementsCount <= 15);
  assert(measurements != nullptr);
  reset();
}


void HX711::begin()
{
  pinMode(_dataPin, INPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_clockPin, LOW);

  reset();
}


void HX711::reset()
{
  _offset   = 0;
  _scale    = 1;
  _gain     = 128;
  _lastRead = 0;
  _mode     = HX711_AVERAGE_MODE;
}


bool HX711::is_ready()
{
  return digitalRead(_dataPin) == LOW;
}


float HX711::read() 
{
  // this waiting takes most time...
  while (digitalRead(_dataPin) == HIGH) yield();
  
  union
  {
    long value = 0;
    uint8_t data[4];
  } v;

  noInterrupts();

  // Pulse the clock pin 24 times to read the data.
  v.data[2] = shiftIn(_dataPin, _clockPin, MSBFIRST);
  v.data[1] = shiftIn(_dataPin, _clockPin, MSBFIRST);
  v.data[0] = shiftIn(_dataPin, _clockPin, MSBFIRST);

  // TABLE 3 page 4 datasheet
  // only default verified, so other values not supported yet
  uint8_t m = 1;   // default gain == 128
  if (_gain == 64) m = 3;
  if (_gain == 32) m = 2;

  while (m > 0)
  {
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_clockPin, LOW);
    m--;
  }

  interrupts();

  // SIGN extend
  if (v.data[2] & 0x80) v.data[3] = 0xFF;

  _lastRead = millis();
  float const res = measurements_[measurementIndex_] = 1.f * v.value;
  if (++measurementIndex_ >= measurementsCount_)
    measurementIndex_ = 0;
  return res;
}


// assumes tare() has been set.
void HX711::calibrate_scale(uint16_t weight)
{
  _scale = (1.0 * weight) / (read_average() - _offset);
}


void HX711::wait_ready(uint32_t ms) 
{
  while (!is_ready())
  {
    delay(ms);
  }
}


bool HX711::wait_ready_retry(uint8_t retries, uint32_t ms) 
{
  while (retries--)
  {
    if (is_ready()) return true;
    delay(ms);
  }
  return false;
}

bool HX711::wait_ready_timeout(uint32_t timeout, uint32_t ms)
{
  uint32_t start = millis();
  while (millis() - start < timeout) 
  {
    if (is_ready()) return true;
    delay(ms);
  }
  return false;
}


float HX711::read_average()
{
  assert(measurementsCount_ >= 1);
  float sum = 0;
  for (uint8_t i = 0; i < measurementsCount_; ++i) 
  {
    sum += measurements_[i];
  }
  return sum / measurementsCount_;
}

static void heap_push(float* heap, uint8_t& heap_size, float value) noexcept;
static float heap_pop(float* heap, uint8_t& heap_size) noexcept;
static float heap_pushpop(float* heap, uint8_t heap_size, float value) noexcept;

float HX711::read_median()
{
  assert(measurementsCount_ <= 15);
  assert(measurementsCount_ >= 3);
  float s[8];
  uint8_t heap_size = 0;
  for (uint8_t i = 0; i < (measurementsCount_)/2; ++i) 
  {
    heap_push(s, heap_size, measurements_[i]);
  }
  for (uint8_t i = (measurementsCount_)/2; i+1 < measurementsCount_; ++i)
  {
    heap_pushpop(s, heap_size, measurements_[i]);
  }
  float res = heap_pushpop(s, heap_size, measurements_[measurementsCount_-1]);
  return (res + s[0]) / 2.f;
}


float HX711::read_medavg()
{
  assert(measurementsCount_ <= 15);
  assert(measurementsCount_ >= 3);
  float s[15];
  uint8_t heap_size = 0;
  for (uint8_t i = 0; i < measurementsCount_; ++i) 
  {
    heap_push(s, heap_size, measurements_[i]);
  }
  float sum = 0;
  // iterate over 1/4 to 3/4 of the array
  uint8_t const first = (measurementsCount_ + 2) / 4;
  for (uint8_t i = 0; i < first; ++i)
    heap_pop(s, heap_size);
  uint8_t const last  = measurementsCount_ - first;
  for (uint8_t i = first; i < last; i++)
  {
    sum += heap_pop(s, heap_size);
  }
  uint8_t const cnt = last-first;
  return sum/cnt;
}



float HX711::get_value() 
{
  float const raw = [this]() {
    switch(_mode)
    {
      case HX711_MEDAVG_MODE:
        return read_medavg();

      case HX711_MEDIAN_MODE:
        return read_median();

      case HX711_AVERAGE_MODE:
      default:
        return read_average();
    }
  }();
  return raw - _offset;
}


float HX711::get_units()
{
  return get_value() * _scale;
}


void HX711::power_down() 
{
  digitalWrite(_clockPin, LOW);
  digitalWrite(_clockPin, HIGH);
}


void HX711::power_up() 
{
  digitalWrite(_clockPin, LOW);
}

static constexpr uint8_t parent_of(uint8_t node) noexcept
{
  return (node-1)/2;
}

static constexpr void swap(float& a, float& b) noexcept
{
  float tmp = a;
  a = b;
  b = tmp;
}

static void heap_push(float* heap, uint8_t& heap_size, float value) noexcept
{
  heap[heap_size++] = value;

  uint8_t i = heap_size-1;
  while (i > 0)
  {
    uint8_t parent = parent_of(i);
    if (heap[parent] < heap[i])
    {
      swap(heap[parent], heap[i]);
      i = parent;
    }
    else
      break;
  }
}

static constexpr uint8_t left_child(uint8_t node) noexcept
{
  return 2*node+1;
}

static constexpr uint8_t right_child(uint8_t node) noexcept
{
  return 2*node+2;
}

static void heap_sink(float* heap, uint8_t heap_size) noexcept
{
  uint8_t i = 0;
  while (true)
  {
    uint8_t const l = left_child(i);
    if (l >= heap_size)
      break;
    uint8_t max_child_idx = l;
    uint8_t const r = right_child(i);
    if (r < heap_size && heap[r] > heap[l])
      max_child_idx = r;
    if (heap[i] < heap[max_child_idx])
      swap(heap[i], heap[max_child_idx]);
    i = max_child_idx;
  }
}

static float heap_pop(float* heap, uint8_t& heap_size) noexcept
{
  float const res = heap[0];
  heap[0] = heap[--heap_size];
  heap_sink(heap, heap_size);
  return res;
}

static float heap_pushpop(float* heap, uint8_t heap_size, float value) noexcept
{
  if (heap_size == 0 || value > heap[0])
    return value;

  float const res = heap[0];
  heap[0] = value;
  heap_sink(heap, heap_size);
  return res;
}

// -- END OF FILE --
