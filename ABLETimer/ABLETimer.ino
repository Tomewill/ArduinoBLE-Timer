#include <ArduinoBLE.h>

BLEService bleService("7c694000-268a-46e3-99f8-04ebc1fb81a4");

//changing value of led (ultimatly RGB led)
BLEIntCharacteristic ledCharact("7c694001-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLEWrite);
//sending raw accelerometer values
BLECharacteristic accelCharact("7c694002-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify, 12);
//sending cube orientation (UP, DOWN, ...)
BLEByteCharacteristic orientCharact("7c694003-268a-46e3-99f8-04ebc1fb81a4", BLERead | BLENotify);

//buffer for central.connected()
BLEDevice centralBuffor; 

//buffors for deltas
unsigned long previous=0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed");
    while(1);
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
int i=0;
void loop() {
  BLE.poll();
  while (centralBuffor.connected()) {
    if (millis() - previous > 500) {
      Serial.print("Cos sie dzieje");
      Serial.println(++i);
      orientCharact.writeValue(i); // max number to send is 255, then overflow
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