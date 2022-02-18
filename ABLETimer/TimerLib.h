#ifndef _TIMERLIB_H_
#define _TIMERLIB_H_

enum CubeOrient {
  UP   = 0,
  DOWN = 1,
  FRONT = 2,
  BACK = 3,
  LEFT = 4,
  RIGHT =5
};

class AccelerometerManager {
public:
  
private:
  float xPos, yPos, zPos;
};

#endif //_TIMERLIB_H_