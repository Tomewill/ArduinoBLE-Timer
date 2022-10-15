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
unsigned long previousRaw=0;
unsigned long previousOrient=0;

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

String bufferStr;

void loop() {
  BLE.poll();
  
  while (centralBuffor.connected()) {
    //check if MyIMU is needed
    /*if (accelCharact.subscribed() || orientCharact.subscribed() || orientDescription.subscribed()) {
      MyIMU.readOrientation();
      Serial.println(MyIMU.concatRawPos());
      Serial.println(MyIMU.concatLogicPos());
    }*/

    //send raw accelerometer values if subscribed

    //send cube orientation (0, 1, ...) if subscribed

    //send cube orientation (UP, DOWN, ...) if subscribed
    if (orientDescription.subscribed() && (millis() - previousOrient > 50)) {
      MyIMU.readOrientation();
      MyIMU.checkOrientation();
      String value = MyIMU.positionToEnumStr();
      Serial.println(value);
      orientDescription.writeValue(value.c_str());
      previousOrient = millis();
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
  digitalWrite(LED_BUILTIN, LOW);
}

void ledCharactWritten (BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("LED written");
  if (ledCharact.value()>0) digitalWrite(LED_BUILTIN, HIGH);
  else digitalWrite(LED_BUILTIN, LOW);
}