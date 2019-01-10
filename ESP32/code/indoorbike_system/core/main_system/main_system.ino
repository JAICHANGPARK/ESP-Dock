#include "FS.h"
#include "SD.h"
#include "SPI.h"

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
BLECharacteristic * pTxCharacteristic;
BLECharacteristic * pRealTimeCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(5, false);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(5, true);
      //모든 플레그 변수 초기화
      firstPhase  = false;
      secondPhase = false;
      checkAuth = false;

      realtimeFirstPhase = false;
      realtimeSecondPhase = false;
      realTimeCheckAuth = false;
      realTimeFinalPhase = false;
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
  //--------------------------------------------------------------

  BLEDevice::init("ESP32_KNU_IBK");  // Create the BLE Device
  pServer = BLEDevice::createServer();  // Create the BLE Server
 pServer -> setCallbacks(new MyServerCallbacks());
 
}

void loop() {
  // put your main code here, to run repeatedly:

}
