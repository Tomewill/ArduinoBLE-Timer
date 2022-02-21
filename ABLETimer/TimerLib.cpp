#include "TimerLib.h"
#include <Arduino.h>

Orientator::Orientator()
:LSM6DS3Class(Wire, 0x6A) //LSM6DS3_ADDRESS defined in LSM6DS3.cpp
{

}

void Orientator::showPos() {
  Serial.begin(9600);
  String buffer="X: ";
  buffer.concat(xPos);
  buffer.concat(" Y: ");
  buffer.concat(yPos);
  buffer.concat(" Z: ");
  buffer.concat(zPos);
  Serial.println(buffer);
  Serial.end();
}

void Orientator::readAcceleration() {
  LSM6DS3Class::readAcceleration(xPos, yPos, zPos);
}

void OrientatorManager::quantize() {
  stabilizedOrient = true;
  if ( xPos >= quantHighMin && xPos <= quantHighMax)   xLogic = 1;
  else if ( xPos >= -quantHighMax && xPos <= -quantHighMin) xLogic =-1;
  else if ( xPos >= -quantLow && xPos <= quantLow)     xLogic = 0;
  else {
    stabilizedOrient = false;
    xLogic = 2;
  }

  if ( yPos >= quantHighMin && yPos <= quantHighMax)   yLogic = 1;
  else if ( yPos >= -quantHighMax && yPos <= -quantHighMin) yLogic =-1;
  else if ( yPos >= -quantLow && yPos <= quantLow)     yLogic = 0;
  else {
    stabilizedOrient = false;
    yLogic = 2;
  }

  if ( zPos >= quantHighMin && zPos <= quantHighMax)   zLogic = 1;
  else if ( zPos >= -quantHighMax && zPos <= -quantHighMin) zLogic =-1;
  else if ( zPos >= -quantLow && zPos <= quantLow)     zLogic = 0;
  else {
    stabilizedOrient = false;
    zLogic = 2;
  }
}

uint8_t OrientatorManager::checkOrientation() {
  if      (xLogic == 1 && yLogic == 0 && zLogic == 0 && stabilizedOrient == true) {
    position = FRONT;
    return     FRONT;
  }
  else if (xLogic == -1 && yLogic == 0 && zLogic == 0 && stabilizedOrient == true) {
    position = BACK;
    return     BACK;
  }

  else if (xLogic == 0 && yLogic == 1 && zLogic == 0 && stabilizedOrient == true)  {
    position = LEFT;
    return     LEFT;
  }
  else if (xLogic == 0 && yLogic == -1 && zLogic == 0 && stabilizedOrient == true) {
    position = RIGHT;
    return     RIGHT;
  }

  else if (xLogic == 0 && yLogic == 0 && zLogic == 1 && stabilizedOrient == true)  {
    position = DOWN;
    return     DOWN;
  }
  else if (xLogic == 0 && yLogic == 0 && zLogic == -1 && stabilizedOrient == true)  {
    position = UP;
    return     UP;
  }
  else {
    position = UNDEFINED;
    return UNDEFINED;
  }
}

/*bool OrientatorManager::compare(Accelerometer acc) {
  this->xPos;
}*/