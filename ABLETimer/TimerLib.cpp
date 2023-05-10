#include "TimerLib.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <string.h>
#include <vector>
#include <algorithm>

using strVec = std::vector<String>;

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


// void setColors(Adafruit_NeoPixel pixels, uint32_t color, uint8_t numLEDs) {
//   for (uint8_t i=0; i<numLEDs; i++)
//     pixels.setPixelColor(i, (color | 0xff << 24)); // max white color
//   pixels.show();
// }

// void turnOff(Adafruit_NeoPixel pixels) {
//   pixels.clear();
//   pixels.show();
// }

// void pingLowBatt(Adafruit_NeoPixel pixels, long _delay, uint8_t n) {
//   for (uint8_t i=0; i<n; i++) {
//     setColors(pixels, 0x0000ff, numLEDs);
//     delay(_delay);
//     turnOff(pixels);
//     delay(_delay);
//   }
//   setColors(pixels, 0xff0000, numLEDs);
// }


void createFileName(char* filename, DateTime now) {
  String line;
  line.reserve(13);
  line = String(now.year());
  uint8_t n = now.month();
  if (n<10) line += '0';
  line += String(n);
  n = now.day();
  if (n<10) line += '0';
  line += String(n);
  line += ".dat";
  strcpy(filename, line.c_str());
  // Serial.print(filename);
  // Serial.println(strlen(filename));
}

bool writeToFile(char* fileName, SecondsOn sideTimeOn) {
  File save = SD.open(fileName, (FILE_WRITE | O_TRUNC));  //override file 
  //if (save) {
    char date[11];
    for (uint8_t i=0; i<4;i++) date[i]=fileName[i]; //create properly formated timestamp
    for (uint8_t i=5; i<7;i++) date[i]=fileName[i-1];
    for (uint8_t i=8; i<10;i++) date[i]=fileName[i-2];
    date[4]='-'; date[7]='-'; date[10]='\0';
    save.print("header;");save.println(date); 
    save.print("U;");save.println(sideTimeOn.up);
    save.print("D;");save.println(sideTimeOn.down);
    save.print("L;");save.println(sideTimeOn.left);
    save.print("R;");save.println(sideTimeOn.right);
    save.print("F;");save.println(sideTimeOn.front);
    save.print("B;");save.print(sideTimeOn.back);
    save.close();
    return true;
  //}
  save.close();
  return false;
}

void readConfig(SecondsOn &timeTmp, char* fileName) {
  String tmp;
  tmp.reserve(80);
  
  File file = SD.open(fileName, FILE_READ);
  while(file.available()){
    char ch = file.read();
    tmp += ch;
  }
  file.close();

  const uint8_t DATA_SEP_NUM = 6;
  char separ = '\n';
	uint8_t encounteredSep = 0;
	uint8_t i = 1, startPos = 0;  // iterator over string
	uint8_t lastIdx = tmp.length() - 1;
	
	String tempData[DATA_SEP_NUM+1] = {"1", "2", "3", "4", "5", "6", "7"};
	
	while (encounteredSep < DATA_SEP_NUM && i <= lastIdx) {  
		if (tmp[i] == separ) {
			tempData[encounteredSep] = tmp.substring(startPos,i);
			startPos = i + 1;
			encounteredSep++;
			i++;
		}
		i++;
	}
	i--;
	tempData[encounteredSep] = tmp.substring(i);
  
  timeTmp.up    = tempData[1].substring(2).toInt();
  timeTmp.down  = tempData[2].substring(2).toInt();
  timeTmp.left  = tempData[3].substring(2).toInt();
  timeTmp.right = tempData[4].substring(2).toInt();
  timeTmp.front = tempData[5].substring(2).toInt();
  timeTmp.back  = tempData[6].substring(2).toInt();
}

bool readValuesToSend(char *values, char* fileName) {
  File file = SD.open(fileName, FILE_READ);
  if (!file) {
    file.close();
    return false; // File doesn't exist
  }

  uint8_t i=0;
  char ch;
  while(file.available()){
    ch = file.read();
    values[i++] = ch;
  }
  file.close();
  values[i] = '\0';
  return true;
  String dupa="dupa";
}

strVec listDaysData() {
  File root = SD.open("/");
  strVec filenames;
  File entry;
  while(true) {
    entry = root.openNextFile();
    if (!entry) break;
    //Serial.print(strlen(entry.name()));
    if (strlen(entry.name()) == 12 && entry.name()[0]>='0' && entry.name()[0]<='9')
      filenames.emplace_back(entry.name()); // getting only my data files
  }
  std::sort(filenames.begin(), filenames.end(), [](String a, String b) { return a > b; }); //lambda expresion, sort asc, newest in the end
  
  return filenames;
}
