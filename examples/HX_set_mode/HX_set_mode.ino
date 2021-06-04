//
//    FILE: HX_set_mode.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711
//
// HISTORY:
// 0.1.0    2021-05-13 initial version
//

#include "HX711.h"

uint8_t const dataPin = 6;
uint8_t const clockPin = 7;
float buffer[7];
HX711 scale{buffer, 7, dataPin, clockPin};

uint32_t start, stop;
volatile float f;

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  scale.begin();

  // TODO find a nice solution for this calibration..
  // loadcell factor 20 KG
  scale.set_scale(127.15);

  // loadcell factor 5 KG
  // scale.set_scale(420.0983);
  // reset the scale to zero = 0
  scale.tare();
  

  Serial.println();
  scale.set_average_mode();
  Serial.print(scale.get_mode());
  Serial.println("\tPERFORMANCE set_average_mode");
  start = micros();
  f = 0;
  for (int i = 0; i < 100; i++)
  {
    f = scale.get_value();
  }
  stop = micros();
  Serial.print("100x set_average_mode = ");
  Serial.println(stop - start);
  Serial.print("  VAL: ");
  Serial.println(f, 2);


  Serial.println();
  scale.set_median_mode();
  Serial.print(scale.get_mode());
  Serial.println("\tPERFORMANCE set_median_mode");
  start = micros();
  f = 0;
  for (int i = 0; i < 100; i++)
  {
    f = scale.get_value();
  }
  stop = micros();
  Serial.print("100x set_median_mode = ");
  Serial.println(stop - start);
  Serial.print("  VAL: ");
  Serial.println(f, 2);


  Serial.println();
  scale.set_medavg_mode();
  Serial.print(scale.get_mode());
  Serial.println("\tPERFORMANCE set_medavg_mode");
  start = micros();
  f = 0;
  for (int i = 0; i < 100; i++)
  {
    f = scale.get_value();
  }
  stop = micros();
  Serial.print("100x set_medavg_mode = ");
  Serial.println(stop - start);
  Serial.print("  VAL: ");
  Serial.println(f, 2);
  Serial.println("\ndone...");
}

void loop()
{
}

// -- END OF FILE --
