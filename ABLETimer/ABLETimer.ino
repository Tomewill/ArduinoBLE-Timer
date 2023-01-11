#include <ArduinoBLE.h>
#include <Wire.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
#include "TimerLib.h"

#define SPI_CS_PIN 4

BLEService bleService("7c694000-268a-46e3-99f8-04ebc1fb81a4");

//changing value of led (ultimatly RGB led)
BLEIntCharacteristic ledCharact("7c694001-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite);
//sending raw accelerometer values
BLECharacteristic accelCharact("7c694002-268a-46e3-99f8-04ebc1fb81a4", BLENotify, 64);
//sending cube orientation (0, 1, ...)
BLEByteCharacteristic orientCharact("7c694003-268a-46e3-99f8-04ebc1fb81a4", BLENotify);
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
char filename[12]; // 11 chars and \0

void setup() {
  Serial.begin(9600);
  while(!Serial);
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
    while (1);
  }

  //initalize RTC
  Wire.begin();
  now = myRTC.now();
  getFileName(filename, now); //filename modified in parameter

  //initialize SD
  if (!SD.begin(SPI_CS_PIN)) {
    Serial.println("Failed to initialize SD!");
    while (1);
  }
  if (SD.exists(filename)) {
    Serial.println("Reading config");
    secondsOn = readConfig(filename);
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
  bleService.addCharacteristic(accelCharact);
  bleService.addCharacteristic(orientCharact);
  bleService.addCharacteristic(orientDescription);

  BLE.addService(bleService);

  //functions for connection and disconnection callbacks
  BLE.setEventHandler(BLEConnected, connectionCallback);
  BLE.setEventHandler(BLEDisconnected, disconnectionCallback);

  //function for written value callbacks
  ledCharact.setEventHandler(BLEWritten, ledCharactWritten);

  //initialize starting values
  ledCharact.writeValue(0x1fcba03); //setValue deprecated
  accelCharact.writeValue("Accel values");
  orientCharact.writeValue(UNDEFINED);
  orientDescription.writeValue("Side");

  BLE.advertise();

  Serial.println("CONFIGURATION COMPLITED");
}


void loop() {
  BLE.poll();

  if (millis() - previousOrientTime > 500) {
      MyIMU.readOrientation();
      uint8_t sideInInt = MyIMU.checkOrientation();
      switch(sideInInt) {
        case UP:
          secondsOn.up += 500;
          break;
        case DOWN:
          secondsOn.down += 500;
          break;
        case LEFT:
          secondsOn.left += 500;
          break;
        case RIGHT:
          secondsOn.right += 500;
          break;
        case FRONT:
          secondsOn.front += 500;
          break;
        case BACK:
          secondsOn.back += 500;
          break;
      }
      previousOrientTime = millis();
    }
  
  if (centralBuffor.connected()) {
    if (millis() - previousSendingTime > 1000){
      if (accelCharact.subscribed()) {
        String time="";
        time.concat(secondsOn.up/1000);
        time.concat(' ');
        time.concat(secondsOn.down/1000);
        time.concat(' ');
        time.concat(secondsOn.left/1000);
        time.concat(' ');
        time.concat(secondsOn.right/1000);
        time.concat(' ');
        time.concat(secondsOn.front/1000);
        time.concat(' ');
        time.concat(secondsOn.back/1000);
        accelCharact.writeValue(time.c_str());
        
      }
      if (orientDescription) {
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
    Serial.println("LED written");
    if (ledCharact.value()>0) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
  }
}