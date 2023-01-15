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
char filename[12]; // 12 chars without \0 
uint8_t actualState, previousState; //initialization in setup
unsigned long actualTimeStamp, previousTimeStamp; 

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
  orientCharact.writeValue(MyIMU.getPosition());
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