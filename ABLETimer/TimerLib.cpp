#include "TimerLib.h"
#include <Arduino.h>

Accelerometer::Accelerometer()
:LSM6DS3Class(Wire, 0x6A) //LSM6DS3_ADDRESS defined in LSM6DS3.cpp
{

}

void Accelerometer::showPos() {
  Serial.begin(9600);
  String buffer="Cos tam";
  buffer.concat(xPos);
  buffer.concat(" ");
  buffer.concat(yPos);
  buffer.concat(" ");
  buffer.concat(zPos);
  Serial.println(buffer);
  Serial.end();
}

void Accelerometer::readAcceleration() {
  LSM6DS3Class::readAcceleration(xPos, yPos, zPos);
}