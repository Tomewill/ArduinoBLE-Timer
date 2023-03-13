#include <ArduinoBLE.h>
#include <Wire.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <algorithm>
#include "TimerLib.h"

#define SPI_CS_PIN 4

BLEService bleService("7c694000-268a-46e3-99f8-04ebc1fb81a4");

//changing value of led (ultimatly RGB led)
BLEIntCharacteristic ledCharact("7c694001-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite);
//sending day times "header;day U:..." 
BLECharacteristic dayDataCharact("7c694002-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify, 100);
//instructions to properly send saved data
BLECharacteristic instrCharact("7c694003-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite, 15);
//sending cube orientation (UP, DOWN, ...)
BLECharacteristic orientDescription("7c694004-268a-46e3-99f8-04ebc1fb81a4", BLENotify, 10);
//buffer for central.connected()
BLEDevice centralBuffor; 
//IMU definition of herited class LSM6DS3Class
Orientator MyIMU;

//buffers for deltas
unsigned long previousSendingTime=0;
unsigned long previousOrientTime=0;

SecondsOn secondsOn;
RTClib myRTC;
DateTime now;
char filename[13]; // 12 chars without \0 
uint8_t actualState, previousState; //initialization in setup
unsigned long actualTimeStamp, previousTimeStamp; 
strVec filenames;

void setup() {
  Serial.begin(9600);
  //while(!Serial);
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

  //initalize RTC
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
  
  //set uuid to connect and name diplayed in bluetooth pairing 
  BLE.setLocalName("Cube timer");
  BLE.setAdvertisedService(bleService);

  //add charachterics to send and recive values
  bleService.addCharacteristic(ledCharact);
  bleService.addCharacteristic(dayDataCharact);
  bleService.addCharacteristic(instrCharact);
  bleService.addCharacteristic(orientDescription);

  BLE.addService(bleService);

  //functions for connection and disconnection callbacks
  BLE.setEventHandler(BLEConnected, connectionCallback);
  BLE.setEventHandler(BLEDisconnected, disconnectionCallback);
  //BLE.setEventHandler(BLE);

  //function for written value callbacks
  ledCharact.setEventHandler(BLEWritten, ledCharactWritten);
  instrCharact.setEventHandler(BLEWritten, instrCharactWritten);
  //initialize starting values
  ledCharact.writeValue(0x1fcba03); //setValue deprecated
  dayDataCharact.writeValue("Accel values");
  instrCharact.writeValue("124;20000330");
  orientDescription.writeValue("Side");

  BLE.advertise();

  Serial.println("CONFIGURATION COMPLITED");
}


void loop() {
  BLE.poll();

  //counting time
  if (millis() - previousOrientTime > 500UL) {
    MyIMU.readOrientation();
    actualState = MyIMU.checkOrientation();
    if (actualState != UNDEFINED && actualState != previousState) {
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
      previousState = actualState; //write to file after changing side
    }
    previousOrientTime = millis();
  }

  
  if (centralBuffor.connected()) {
    if (millis() - previousSendingTime > 1000UL){

      if (orientDescription.subscribed()) {
        MyIMU.readOrientation();
        MyIMU.checkOrientation();
        String value = MyIMU.positionToEnumStr();
        Serial.println(value);
        orientDescription.writeValue(value.c_str());
      }
      previousSendingTime = millis();
    }
  }
}

void connectionCallback (BLEDevice central) {
  centralBuffor = central;
  Serial.println("Connected");
}

void disconnectionCallback (BLEDevice central) {
  Serial.println("Disconnected");
  digitalWrite(LED_BUILTIN, LOW);
}

void ledCharactWritten (BLEDevice central, BLECharacteristic characteristic) {
  if (ledCharact.written()) {
    uint32_t val = ledCharact.value();
    Serial.print("LED written: ");
    Serial.println(val, HEX);
    if (val>0) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
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






