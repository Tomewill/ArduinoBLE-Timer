#include <ArduinoBLE.h>
#include "TimerLib.h"

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

//buffers for deltas
unsigned long previousSendingTime=0;
unsigned long previousOrientTime=0;


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

  Serial.println("Cube timer peripheral");
}

unsigned long timeUP=0;
unsigned long timeDOWN=0;
unsigned long timeLEFT=0;
unsigned long timeRIGHT=0;
unsigned long timeFRONT=0;
unsigned long timeBACK=0;

void loop() {
  BLE.poll();

  if (millis() - previousOrientTime > 100) {
      
      MyIMU.readOrientation();

      uint8_t sideInInt = MyIMU.checkOrientation();
      switch(sideInInt) {
        case UP:
          timeUP += 100;
          break;
        case DOWN:
          timeDOWN += 100;
          break;
        case LEFT:
          timeLEFT += 100;
          break;
        case RIGHT:
          timeRIGHT += 100;
          break;
        case FRONT:
          timeFRONT += 100;
          break;
        case BACK:
          timeBACK += 100;
          break;
      }
      
      previousOrientTime = millis();
    }
  
  if (centralBuffor.connected()) {
    if (millis() - previousSendingTime > 1000){
      if (accelCharact.subscribed()) {
        String time="";
        time.concat(timeUP);
        time.concat(' ');
        time.concat(timeDOWN);
        time.concat(' ');
        time.concat(timeLEFT);
        time.concat(' ');
        time.concat(timeRIGHT);
        time.concat(' ');
        time.concat(timeFRONT);
        time.concat(' ');
        time.concat(timeBACK);
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