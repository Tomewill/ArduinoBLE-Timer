#ifndef _TIMERLIB_H_
#define _TIMERLIB_H_

#include <LSM6DS3.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include <Wire.h>
#include <vector>

enum CubeOrient { //on which side borad/cube is laying
  UP    = 0, //board where reset and LEDs are seen
  DOWN  = 1,
  LEFT  = 2,
  RIGHT = 3,
  FRONT = 4,
  BACK  = 5, //board where USB port is seen
  UNDEFINED = 6
};

struct SecondsOn{
  unsigned long up;
  unsigned long down;
  unsigned long left;
  unsigned long right;
  unsigned long front;
  unsigned long back;
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
  const float quantHighMin=0.75, quantHighMax=1.25; //quantisation intervals 
  const float quantLow=0.25;
  //if any axis is between quantLow and quantHighMin, then cube position isn't in desired posiotion

  bool stabilizedOrient; //is board/cube in predefined positions CubeOrient

  int8_t xLogic, yLogic, zLogic;
  
  uint8_t position;
};

class ColorSystem {
public:
  ColorSystem(uint8_t, uint8_t);
  // void HSVtoRGB;
  // void RGBtoHSV;
  Adafruit_NeoPixel pixels;

private:
  uint8_t redVal, greenVal, blueVal;
  unsigned long speedInterval;
  unsigned long lastSpeedTime;

};

void createFileName(char* filename, DateTime now); //return filename in parameter
bool writeToFile(char* fileName, SecondsOn sideTimeOn);
void readConfig(SecondsOn &timeTmp, char* fileName);
bool readValuesToSend(char *values, char* fileName);
<<<<<<< HEAD
using strVec = std::vector<String>;
strVec listDaysData();
=======
>>>>>>> 6a0d036925f4626ed9f5ed8413b14f958f80689d


#endif //_TIMERLIB_H_