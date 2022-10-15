#include "TimerLib.h"
#include <Arduino.h>

Orientator::Orientator()
:LSM6DS3Class(Wire, 0x6A){} //LSM6DS3_ADDRESS defined in LSM6DS3.cpp

void Orientator::readAcceleration() {
  LSM6DS3Class::readAcceleration(xPos, yPos, zPos);
}

void Orientator::quantize() {
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

void Orientator::readOrientation() {
  Orientator::readAcceleration();
  Orientator::quantize();
}

uint8_t Orientator::checkOrientation() {
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

String Orientator::concatRawPos() {
  String buffer="X: ";
  buffer.concat(xPos);
  buffer.concat(" Y: ");
  buffer.concat(yPos);
  buffer.concat(" Z: ");
  buffer.concat(zPos);
  
  return buffer;
}

String Orientator::concatLogicPos() {
  String buffer="X: ";
  buffer.concat(xLogic);
  buffer.concat(" Y: ");
  buffer.concat(yLogic);
  buffer.concat(" Z: ");
  buffer.concat(zLogic);
  
  return buffer;
}

String Orientator::positionToEnumStr() {
  switch (position) {
        case UP:
          return "UP";
        break;
        case DOWN:
          return "DOWN";
        break;
        case RIGHT:
          return "RIGHT";
        break;
        case LEFT:
          return "LEFT";
        break;
        case BACK:
          return "BACK";
        break;
        case FRONT:
          return "FRONT";
        break;
        case UNDEFINED:
          return "UNDEFINED";
        break;
  }
}