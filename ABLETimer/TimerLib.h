#ifndef _TIMERLIB_H_
#define _TIMERLIB_H_

#include <LSM6DS3.h>


enum CubeOrient {
  UP   = 0,
  DOWN = 1,
  FRONT = 2,
  BACK = 3,
  LEFT = 4,
  RIGHT =5
};
class Accelerometer; 
class AccelerometerManager;

class AccelerometerManager {
public:
  /*
  bool compare();
  ?type? check position 
  */
private:
  float xPos, yPos, zPos;
};

class Accelerometer : public LSM6DS3Class{
public:
  Accelerometer();

  void showPos();

  void readAcceleration();
  /*
  bool compare();
  ?type? check position 
  */
private:
  float xPos, yPos, zPos;
};

#endif //_TIMERLIB_H_