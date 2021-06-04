//
//    FILE: HX_kitchen_scale.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711
//
// HISTORY:
// 0.1.0    2020-06-16 initial version
//

// to be tested 

#include "HX711.h"

uint8_t const dataPin = 6;
uint8_t const clockPin = 7;
float buffer[10];
HX711 scale{ buffer, 10, dataPin, clockPin };


float w1, w2, previous = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  scale.begin();

  Serial.print("UNITS: ");
  Serial.println(scale.get_units());

  // loadcell factor 20 KG
  // scale.set_scale(127.15);
  // loadcell factor 5 KG
  scale.set_scale(420.0983);
  scale.tare();

  Serial.print("UNITS: ");
  Serial.println(scale.get_units());
}

void loop()
{
  // read until stable
  w1 = scale.get_units();
  delay(100);
  w2 = scale.get_units();
  while (abs(w1 - w2) > 10)
  {
     w1 = w2;
     w2 = scale.get_units();
     delay(100);
  }
  
  Serial.print("UNITS: ");
  Serial.print(w1);
  if (w1 == 0)
  {
    Serial.println();
  }
  else
  {
    Serial.print("\t\tDELTA: ");
    Serial.println(w1 - previous);
    previous = w1;
  }
  delay(100);
}

// END OF FILE
