#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed");
    while(1);
  }

  BLE.setLocalName("LED Callback");

  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(switchCharacteristic);

  //add service(probably for ble callbacks)
  BLE.addService(ledService);

  //functions for connection and disconnection callbacks
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  //function for written value
  switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);

  switchCharacteristic.setValue(0);

  BLE.advertise();

  Serial.println("BLE Callback LED Peripheral");
}

void loop() {
  BLE.poll();

}

void blePeripheralConnectHandler(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, LOW);
}

void switchCharacteristicWritten (BLEDevice central, BLECharacteristic characteristic) {
  //central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written: ");

  if (switchCharacteristic.value()) {
    Serial.println("LED on");
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("LED off");
    digitalWrite(LED_BUILTIN, LOW);
  }
}


