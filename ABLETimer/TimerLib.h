#ifndef _TIMERLIB_H_
#define _TIMERLIB_H_

#include <LSM6DS3.h>


enum CubeOrient { //on which side borad/cube is laying
  UP    = 0, //board where reset and LEDs are seen
  DOWN  = 1,
  FRONT = 2,
  BACK  = 3, //board where USB port is seen
  LEFT  = 4,
  RIGHT = 5,
  UNDEFINED = 6
};

class Orientator : public LSM6DS3Class {
public:
  Orientator(void); // initialize LSM6DS3

  void readAcceleration(void); //read values form IMU and save it in *Pos

  void quantize(void); // change *Pos to 1, -1, 0 logic and save it in *Logic; returns 2 when values are between (quantLow;quantHighLow), that means board/cube is not in desired position

  void readOrientation(void); //readAcceleration() and quantize() in one function
  
  uint8_t checkOrientation(void); //return value from CubeOrient enum under proper conditions;

  float getX() {return xPos;}
  float getY() {return yPos;}
  float getZ() {return zPos;}

  int8_t getXLogic() {return xLogic;}
  int8_t getYLogic() {return yLogic;}
  int8_t getZLogic() {return zLogic;}
  uint8_t getPosition() {return position;}

  String concatRawPos(void); //return raw accelerometer values in one string
  String concatLogicPos(void); //return logic accelerometer values in one string
  String positionToEnumStr(void); //return (UP, DOWN, ...)

private:
  //raw data
  float xPos, yPos, zPos;

  //0 1 logic data
  const float quantHighMin=0.75, quantHighMax=1.25; // quantisation intervals 
  const float quantLow=0.25;
  //if any axis is between quantLow and quantHighMin, then cube position isn't in desired posiotion

  bool stabilizedOrient; //is board/cube in predefined positions CubeOrient

  int8_t xLogic, yLogic, zLogic;
  
  uint8_t position;
};

#endif //_TIMERLIB_H_