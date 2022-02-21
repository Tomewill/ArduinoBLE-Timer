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
class Orientator; 
class OrientatorManager;

class OrientatorManager { // !!!!!!!!!!! POMYSLEC CZY NIE USUNAC OrientatorManager I PRZECHOWYWAC OSTATNI STAN PLYTKI W ZMIENNEJ -> BEZSENSOWNE KOMPLIKOWANIE KODU  --  zajac sie dokumentacja (dodac png plytki z opisem, opisy, opis github itp)
public:
  void quantize(); // change *Pos to 1, -1, 0 logic and save it in *Logic; returns 2 when values are between (quantLow;quantHighLow),that means board/cube is not in desired position
  
  uint8_t checkOrientation(); //return value from CubeOrient enum under proper conditions

  bool compare(Orientator);
  bool compare(OrientatorManager);// !!!!!!!!!!!!!!!!!!!! DO WYWALENIA

  float getX() {return xPos;}
  float getY() {return yPos;}
  float getZ() {return zPos;}

  int8_t getXLogic() {return xLogic;}
  int8_t getYLogic() {return yLogic;}
  int8_t getZLogic() {return zLogic;}
  uint8_t getPosition() {return position;}
  
protected:
  float xPos, yPos, zPos;

  const float quantHighMin=0.75, quantHighMax=1.25; // quantisation intervals 
  const float quantLow=0.25;
  bool stabilizedOrient;

  int8_t xLogic, yLogic, zLogic;
  
  uint8_t position;
};

class Orientator : public LSM6DS3Class, public OrientatorManager{
public:
  Orientator();

  void showPos(); //display accelerometer values via Serial
  

  void readAcceleration(); 
  
};

#endif //_TIMERLIB_H_