#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <sys/time.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define HEART_RATE_SERVICE_UUID             "0000180d-0000-1000-8000-00805f9b34fb" // Heart Rate service UUID
#define CHARACTERISTIC_HEART_RATE           "00002a37-0000-1000-8000-00805f9b34fb"

#define FITNESS_MACHINE_SERVICE_UUID        "00001826-0000-1000-8000-00805f9b34fb" // Fitness Machine service UUID
#define CHARACTERISTIC_INDOOR_BIKE          "00002ad2-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_TREADMILL            "00002acd-0000-1000-8000-00805f9b34fb"

#define DATETIME_SERVICE_UUID               "0000aaa0-0000-1000-8000-00805f9b34fb" // DateTime service UUID
#define CHARACTERISTIC_DATE_TIME_SYNC       "00000001-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_DATE_TIME            "00000002-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_RESULT_CHAR          "0000fff0-0000-1000-8000-00805f9b34fb"

#define AUTH_SERVICE_UUID                   "0000eee0-0000-1000-8000-00805f9b34fb" // Device Auth service UUID
#define CHARACTERISTIC_AUTH                 "0000eee1-0000-1000-8000-00805f9b34fb"

#define DATA_SYNC_SERVICE_UUID              "0000fff0-0000-1000-8000-00805f9b34fb" // DataSync service UUID
#define CHARACTERISTIC_CONTROL              "0000fff1-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_SYNC                 "0000fff2-0000-1000-8000-00805f9b34fb"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic; //HeartRate

BLECharacteristic *pIndoorBikeCharacteristic;
BLECharacteristic *pTreadmillCharacteristic;

struct timeval tv;
struct timeval mytime;

bool deviceConnected = false;
bool oldDeviceConnected = false;


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {14, 0, false};
Button button2 = {15, 0, false};

void IRAM_ATTR isr() {
  button2.numberKeyPresses += 1;
  button2.pressed = true;
}


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(5, false);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(5, true);
      //모든 플레그 변수 초기화
      //      firstPhase  = false;
      //      secondPhase = false;
      //      checkAuth = false;
      //
      //      realtimeFirstPhase = false;
      //      realtimeSecondPhase = false;
      //      realTimeCheckAuth = false;
      //      realTimeFinalPhase = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      uint8_t tmp[rxValue.length()];
      Serial.print("콜백으로 들어온 데이터 길이 : ");  Serial.println(rxValue.length());
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
          tmp[i] = rxValue[i];
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};

//#define CHARACTERISTIC_UUID_REALTIME        "0000ffe3-0000-1000-8000-00805f9b34fb"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  tv.tv_sec = 1540885090;
  settimeofday(&tv, NULL);

  pinMode(button1.PIN, INPUT_PULLUP);
  //    attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, isr, FALLING);

  //--------------------------------------------------------------

  BLEDevice::init("ESP32_KNU_IBK");  // Create the BLE Device
  pServer = BLEDevice::createServer();  // Create the BLE Server
  pServer -> setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer -> createService(HEART_RATE_SERVICE_UUID);
  pTxCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_HEART_RATE,
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_READ);
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLEService *pFitnessMachineService = pServer -> createService(FITNESS_MACHINE_SERVICE_UUID);
  pIndoorBikeCharacteristic  = pFitnessMachineService -> createCharacteristic(CHARACTERISTIC_INDOOR_BIKE,
                               BLECharacteristic::PROPERTY_NOTIFY);
  pIndoorBikeCharacteristic->addDescriptor(new BLE2902());
  pTreadmillCharacteristic  = pFitnessMachineService -> createCharacteristic(CHARACTERISTIC_TREADMILL,
                              BLECharacteristic::PROPERTY_NOTIFY);
  pTreadmillCharacteristic->addDescriptor(new BLE2902());

  pService->start();// Start the service
  pFitnessMachineService -> start();
  pServer->getAdvertising()->start();// Start advertising
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  // put your main code here, to run repeatedly:

  if (button1.pressed) {
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }
  if (button2.pressed) {
    Serial.printf("Button 2 has been pressed %u times\n", button2.numberKeyPresses);
    button2.pressed = false;
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

}
