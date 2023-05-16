#include <ArduinoBLE.h>
#include <Wire.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <algorithm>
#include "TimerLib.h"

#define SPI_CS_PIN 3
#define PIN_NEO_PIXEL 2 
#define NUM_PIXELS 8 
#define PIN_BATTERY A1

BLEService cubeService("7c694000-268a-46e3-99f8-04ebc1fb81a4");
BLEService batteryService("180F");

BLEUnsignedCharCharacteristic batteryCharact("2A19", BLERead | BLENotify); 

//changing value of RGB led)
BLEIntCharacteristic ledCharact("7c694001-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite);
//sending day times "header;day U:..." 
BLECharacteristic dayDataCharact("7c694002-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify, 100);
//instructions to properly send saved data
BLECharacteristic instrCharact("7c694003-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite, 15);
//sending cube orientation (UP, DOWN, ...)
BLECharacteristic orientDescription("7c694004-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify, 10);
//buffer for central.connected()
BLEDevice centralBuffor; 
//IMU definition of herited class LSM6DS3Class
Orientator MyIMU;

//buffers for deltas
unsigned long previousSendingTime=0;
unsigned long previousOrientTime=0;
unsigned long previousSavingTime=300000;

SecondsOn secondsOn;
RTClib myRTC;
DateTime now;
char filename[13]; // 12 chars without \0 
uint8_t actualState, previousState; //initialization in setup
String value;
unsigned long actualTimeStamp, previousTimeStamp;
uint16_t batteryOld=0;
void updateBattery();

Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);


using strVec = std::vector<String>;
strVec filenames;

void setup() {
  Serial.begin(9600);
  // while(!Serial);
  delay(200);
  Serial.println("CONFIGURATION STARTED");
  pinMode(LED_BUILTIN, OUTPUT);

  //initialize BLE
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while(1);
  }

  //initialize IMU  
  if (!MyIMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while(1);
  }
  MyIMU.readOrientation();
  actualState = MyIMU.checkOrientation();
  previousState = -1;

  //initalize RTC and file
  Wire.begin();
  now = myRTC.now();
  createFileName(filename, now); //filename modified in parameter
  actualTimeStamp = now.unixtime();
  previousTimeStamp = actualTimeStamp;

  //initialize SD
  if (!SD.begin(SPI_CS_PIN)) {
    Serial.println("Failed to initialize SD!");
    while(1);
  }
  filenames = listDaysData();
  for (String& file : filenames){
    Serial.println(file);
  }
  if (SD.exists(filename)) {
    Serial.println("Reading config");
    readConfig(secondsOn, filename);
  } else {
    Serial.println("File doesn't exist");
  }
  Serial.println(secondsOn.up);
  Serial.println(secondsOn.down);
  Serial.println(secondsOn.left);
  Serial.println(secondsOn.right);
  Serial.println(secondsOn.front);
  Serial.println(secondsOn.back);

  //initialize LEDs
  pixels.begin();
  setColors(0x0000ff);
  
  //set uuid to connect and name diplayed in bluetooth pairing 
  BLE.setLocalName("Cube timer");
  BLE.setAdvertisedService(cubeService);

  //add charachterics to send and recive values
  cubeService.addCharacteristic(ledCharact);
  cubeService.addCharacteristic(dayDataCharact);
  cubeService.addCharacteristic(instrCharact);
  cubeService.addCharacteristic(orientDescription);
  BLE.addService(cubeService);
  
  batteryService.addCharacteristic(batteryCharact);
  BLE.addService(batteryService);

  //functions for connection and disconnection callbacks
  BLE.setEventHandler(BLEConnected, connectionCallback);
  BLE.setEventHandler(BLEDisconnected, disconnectionCallback);
  //BLE.setEventHandler(BLE);

  //function for written value callbacks
  ledCharact.setEventHandler(BLEWritten, ledCharactWritten);
  instrCharact.setEventHandler(BLEWritten, instrCharactWritten);
  //initialize starting values
  ledCharact.writeValue(0x0000ff); //blue color to connect
  dayDataCharact.writeValue("Accel values");
  instrCharact.writeValue("124;20000330");
  MyIMU.checkOrientation();
  value = MyIMU.positionToEnumStr();
  orientDescription.writeValue(value.c_str());
  Serial.println("Side: "+value);
  batteryCharact.writeValue(batteryOld);

  BLE.advertise();
  Serial.println("CONFIGURATION COMPLITED");
}

void loop() {

  BLE.poll();

  //counting time
  if (millis() - previousOrientTime > 500UL) {
    MyIMU.readOrientation();
    actualState = MyIMU.checkOrientation();
    value = MyIMU.positionToEnumStr();
    if ((actualState != UNDEFINED && actualState != previousState) || (millis() - previousSavingTime > 300000UL)) { //write to file after changing side or when passed 5 mins

      if (centralBuffor.connected() && orientDescription.subscribed()) { //send side when its changed
        Serial.println(value);
        orientDescription.writeValue(value.c_str());
      }
      now = myRTC.now();
      actualTimeStamp = now.unixtime();
      unsigned long delta = actualTimeStamp - previousTimeStamp;
      switch(previousState) {
        case UP:
          secondsOn.up += delta;
          break;
        case DOWN:
          secondsOn.down += delta;
          break;
        case LEFT:
          secondsOn.left += delta;
          break;
        case RIGHT:
          secondsOn.right += delta;
          break;
        case FRONT:
          secondsOn.front += delta;
          break;
        case BACK:
          secondsOn.back += delta;
          break;
      }
      if (writeToFile(filename, secondsOn)) {
        Serial.println("Wrote to file succesfully");
      } else Serial.println("Failed to write to file");
      previousTimeStamp = actualTimeStamp;
      previousState = actualState; 
      previousSavingTime = millis();
    }
    previousOrientTime = millis();
  }

  //BLE operations
  if (centralBuffor.connected()) {
    if (millis() - previousSendingTime > 1000UL){
      updateBattery();
      previousSendingTime = millis();
    }
  }
}

void connectionCallback (BLEDevice central) {
  centralBuffor = central;
  ledCharact.writeValue(0x00ff00);
  setColors(0x00ff00);
  Serial.println("Connected");
  updateBattery();
}

void disconnectionCallback (BLEDevice central) {
  setColors(0xff0000);
  Serial.println("Disconnected");
  delay(500);
  setColors(0x0000ff);
}

void ledCharactWritten (BLEDevice central, BLECharacteristic characteristic) {
  if (ledCharact.written()) {
    uint32_t val = ledCharact.value();
    if ((val>>24 & 0xff) > 0) setColors(val);
    else turnOff();
    Serial.print("LED written: ");
    Serial.println(val, HEX);
  }
}

void instrCharactWritten (BLEDevice central, BLECharacteristic characteristic) {
  if (instrCharact.written()) {
    String instr = (const char*)instrCharact.value();
    Serial.print("-----------Instruction written: ");
    Serial.println(instr);
    char values[100];
    if (instr.equals("20000101.DAT")) {
      Serial.println("Sending first day");
      readValuesToSend(values, (char*)filenames.back().c_str());
      Serial.println(filenames.back());
      dayDataCharact.writeValue(values);
    } else if (std::find(filenames.begin(), filenames.end(), instr) != filenames.end()) {
      Serial.print("Sending day: ");
      Serial.println(instr);
      if (readValuesToSend(values, (char*)instr.c_str()))
        dayDataCharact.writeValue(values);
    } else {
      Serial.println("Sending empty");
      dayDataCharact.writeValue("Empty,nowork");
    }
    Serial.println("-----------Instruction done!");
  }  
}

void updateBattery() {
  int battery=analogRead(PIN_BATTERY);
  static const int batteryMin=484, batteryMax=637;
  int batteryPerC=map(battery, batteryMin, batteryMax, 0, 100);

  if (battery != batteryOld) {      
    Serial.print("Battery: ");
    Serial.println(battery);
    Serial.print("Battery in %: ");
    Serial.println(batteryPerC);
    batteryCharact.writeValue(battery);
    batteryOld=battery;
  }
}

void setColors(uint32_t color) {
  for (uint8_t i=0; i<NUM_PIXELS; i++)
    pixels.setPixelColor(i, (color | 0xff << 24)); // max white color
  pixels.show();
}

void turnOff() {
  pixels.clear();
  pixels.show();
}

void pingLowBatt(long _delay, uint8_t n) {
  for (uint8_t i=0; i<n; i++) {
    setColors(0xff0000);
    delay(_delay);
    turnOff();
    delay(_delay);
  }
  setColors(0xff0000);
}


