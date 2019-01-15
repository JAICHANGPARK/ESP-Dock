
/**
 * 
 * 실내 자전거 운동 정보 자동 획득 장치
 * Device : ESP - 32 
 * Code : Dreamwalker( 박제창 )
 * codeVersion : 1.0.0
 * 
 */
 
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <SPI.h>
#include <MFRC522.h>

#include <sys/time.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SS_PIN 12
#define RST_PIN 13


#define ONE_ROUND_DISTANCE 2.198F
#define LEXPA_DISTANCE 5.0F
#define REAL_TIME_STOP_MILLIS 3000      // 실시간 3초 반응 없을 시 값 초기화
#define WORKOUT_DONE_TIME_MILLIS 30000   // 운동 자동 종료 시간 30초후 모든 정보 초기화 및 변수 저장.

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

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

bool readRfidFlag = false; // 테그 정보 요청이 들어왔을 때 처리하는 플래그
bool deleteRfidFlag = false;
byte nuidPICC[4];

struct timeval tv;
struct timeval mytime;

bool deviceConnected = false;
bool oldDeviceConnected = false;


volatile uint32_t count = 0;
volatile float distance = 0.0f;
volatile float distanceUnitKm = 0.0f;
volatile float speedNow = 0.0f;
volatile uint16_t uintSpeedNow = 0;
volatile uint32_t uintTotalDistance = 0;

volatile uint16_t uintDistanceKm = 0;
volatile uint32_t sumDistanceKm = 0x0000;  // 평균을 구하기 위한 이동거리 변수 ( 더 해짐)
volatile uint32_t sumSpeed = 0x0000;  // 평균 속도를 구하기 위한 속도 합 변수 (더 해짐)
volatile uint32_t sumHeartRate = 0;   // 평균 심박수를 구하기 위한 변수

long t = 0;  // 센서 입력 외부 인터럽트 시간 변수 .
volatile float InstantTime = 0.0f;

uint8_t heartRateData[] = {0b00000010, 0x00};
uint8_t treadmillData[] = {0x05, 0x00, 0x00, 0x00, 0x00};
uint8_t indoorBikeData[] = {0x00, 0x00, 0x00, 0x00};
uint8_t authData[] = {0x00, 0x00, 0x00};
uint8_t resultPacket[] = {0x00, 0x00, 0x00}; // 결과 값 반환 패킷 데이터 버퍼
boolean deviceConnectedFlag = false;  //장비가 블루투스 연결이 되었는지 확인하는 플래그
boolean fitnessStartOrEndFlag = false; // 운동의 시작과 종료를 판단하는 플레그

long previousMillis = 0;  // last time the battery level was checked, in ms
long realTimePreviousMillis = 0;  // 실시간운동 정보 초기화를 위한 한계 시간 약 3초 이상 인터럽트가 없으면 초기화한다.

volatile long startFitnessTime = 0;
volatile long endFitnessTime = 0;
volatile long workoutTime = 0;

//심박수 처리
uint8_t globalHeartRate = 0; // 심박수 전역 변수
uint16_t heartRateCount = 0; // 심박수 평균을 구하기위한 계수 카운터 변수
boolean heartRateMeasureFlag = false; // 심박 센서 착용여부 플레그
boolean heartRateLocationFlag = false; // 심박 센서 위치 확인플레그

boolean bleDateTimeSycnFlag = false; // 블루투스를 통해 시간 동기화가 되었을시 처리하는 플래그
boolean bleAuthCheckFlag = false; // 사용자 인증을 위한 플래그 실패시 false 성공시 true
boolean bleDataSyncFlag = false; // 데이터 전송 요청 이 들어왔을겨우 올바른 데이터 형식이면 true 아니면 false

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {14, 0, false};  // 심박 센서 
Button button2 = {15, 0, false};  // 자계 감지 센서 

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
