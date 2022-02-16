#include <ArduinoBLE.h>

BLEService ledService("180A");//BLE LED Service

//BLE LED Switch characteristic 
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed");
    while(1);
  }

  //set advertised local name and service UUID
  BLE.setLocalName("Nano 33 IoT");
  BLE.setAdvertisedService(ledService);

  //add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  //add service
  BLE.addService(ledService);

  //initial value for charachteristic
  switchCharacteristic.writeValue(0);

  //start advertising
  BLE.advertise();
}

void loop() {
  //listen for BLE centrals to connect
  BLEDevice central = BLE.central();

  if (central) { //we have connection to central
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while(central.connected()) { //while loop to use the values to control LED
      if (switchCharacteristic.written()) {
        switch (switchCharacteristic.value()) {
          case 01:
            Serial.println("LED on");
            digitalWrite(LED_BUILTIN, HIGH);
            break;
          case 02:
            Serial.println("LED fast blink");
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            break;
          case 03:
            Serial.println("LED normal blink");
            digitalWrite(LED_BUILTIN, HIGH);
            delay(1000);
            digitalWrite(LED_BUILTIN, LOW);
            delay(1000);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(1000);
            digitalWrite(LED_BUILTIN, LOW);
            break;
          default:
            Serial.println("LED off");
            digitalWrite(LED_BUILTIN, LOW);
            break;
        }
      }
    }

    //when central disconnects...
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);
  }
}
