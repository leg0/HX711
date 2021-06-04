//
//    FILE: HX_grocery_scale.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711
//
// HISTORY:
// 0.1.0    2020-06-16 initial version
//

#include "HX711.h"

uint8_t const dataPin = 6;
uint8_t const clockPin = 7;
float buffer[10];
HX711 scale{ buffer, 10, dataPin, clockPin };


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
  
  Serial.println("\nEmpty the scale, press a key to continue");
  while(!Serial.available());
  while(Serial.available()) Serial.read();
  
  scale.tare();
  Serial.print("UNITS: ");
  Serial.println(scale.get_units());


  Serial.println("\nPut 1000 gr in the scale, press a key to continue");
  while(!Serial.available());
  while(Serial.available()) Serial.read();

  scale.calibrate_scale(1000);
  Serial.print("UNITS: ");
  Serial.println(scale.get_units());

  Serial.println("\nScale is calibrated, press a key to continue");
  while(!Serial.available());
  while(Serial.available()) Serial.read();

  scale.set_unit_price(0.031415);  // we only have one price
}

void loop()
{
  Serial.print("UNITS: ");
  Serial.print(scale.get_units());
  Serial.print("\t\tPRICE: ");
  Serial.println(scale.get_price());
  delay(250);
}

// END OF FILE