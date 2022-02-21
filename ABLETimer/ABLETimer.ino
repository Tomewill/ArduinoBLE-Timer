#include <ArduinoBLE.h>
#include "TimerLib.h"

BLEService bleService("7c694000-268a-46e3-99f8-04ebc1fb81a4");

//changing value of led (ultimatly RGB led)
BLEIntCharacteristic ledCharact("7c694001-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite);
//sending raw accelerometer values
BLECharacteristic accelCharact("7c694002-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify, 64);
//sending cube orientation (UP, DOWN, ...)
BLEByteCharacteristic orientCharact("7c694003-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify);

//buffer for central.connected()
BLEDevice centralBuffor; 

//buffers for deltas
unsigned long previous=0;

//IMU definition of herited class LSM6DS3Class
Orientator MyIMU;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  //initialize ble
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed");
    while(1);
  }

  //initialize IMU  
  if (!MyIMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  //set uuid to connect and name diplayed in bluetooth pairing 
  BLE.setLocalName("Cube timer");
  BLE.setAdvertisedService(bleService);

  //add charachterics to send and recive values
  bleService.addCharacteristic(ledCharact);
  bleService.addCharacteristic(accelCharact);
  bleService.addCharacteristic(orientCharact);

  BLE.addService(bleService);

  //functions for connection and disconnection callbacks
  BLE.setEventHandler(BLEConnected, connectionCallback);
  BLE.setEventHandler(BLEDisconnected, disconnectionCallback);

  //function for written value callbacks
  ledCharact.setEventHandler(BLEWritten, ledCharactWritten);

  //initialize starting values
  ledCharact.writeValue(3); //setValue deprecated
  accelCharact.writeValue((uint8_t)5);
  orientCharact.writeValue(10);

  BLE.advertise();

  Serial.println("Cube timer peripheral");
}

String bufferStr;

void loop() {
  BLE.poll();
  
  while (centralBuffor.connected()) {
    if (millis() - previous > 50) {
      MyIMU.readAcceleration();
      MyIMU.showPos();
      MyIMU.quantize();

      Serial.print(MyIMU.getXLogic());
      Serial.print("\t");
      Serial.print(MyIMU.getYLogic());
      Serial.print("\t");
      Serial.println(MyIMU.getZLogic());
      switch (MyIMU.checkOrientation()) {
        case UP:
          Serial.println("UP");
          accelCharact.writeValue("UP");
        break;
        case DOWN:
          Serial.println("DOWN");
          accelCharact.writeValue("DOWN");
        break;
        case RIGHT:
          Serial.println("RIGHT");
          accelCharact.writeValue("RIGHT");
        break;
        case LEFT:
          Serial.println("LEFT");
          accelCharact.writeValue("LEFT");
        break;
        case BACK:
          Serial.println("BACK");
          accelCharact.writeValue("BACK");
        break;
        case FRONT:
          Serial.println("FRONT");
          accelCharact.writeValue("FRONT");
        break;
        case UNDEFINED:
          Serial.println("UNDEFINED");
          accelCharact.writeValue("UNDEFINED");
        break;
      }

      /*bufferStr="X:";
      bufferStr.concat(MyIMU.getX());
      bufferStr.concat("Y:");
      bufferStr.concat(MyIMU.getY());
      bufferStr.concat("Z:");
      bufferStr.concat(MyIMU.getZ());

      accelCharact.writeValue(bufferStr.c_str());*/

      previous = millis();
    }
    BLE.poll();
  }
}

void connectionCallback (BLEDevice central) {
  centralBuffor = central;
  Serial.println("Connected");
}

void disconnectionCallback (BLEDevice central) {
  Serial.println("Disconnected");
}

void ledCharactWritten (BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("LED written");
}