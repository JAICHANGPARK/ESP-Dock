
/**

   실내 자전거 운동 정보 자동 획득 장치
   Device : ESP - 32
   Code : Dreamwalker( 박제창 )
   codeVersion : 1.0.0

*/

#include "FS.h"
#include "SD.h"
#include "SPI.h"


#include <MFRC522.h>

#include <sys/time.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SS_PIN 12
#define RST_PIN 13

#define DEBUG
#define ERGOMETER_LEXPA
#define USE_OLED

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

BLECharacteristic *pIndoorBikeCharacteristic;     // 실내자전거 실시간 :  해당 특성 설정은 속도를 추출하도록 flag를 설정해 놓음
BLECharacteristic *pTreadmillCharacteristic;      // 트레드밀 실시간 : 해당 특성 설정은 이동 거리를 추출하도록 flag를 설정해 놓음
BLECharacteristic *pDateTimeSyncCharacteristic;   // 시간 동기화 설정
BLECharacteristic *pResultCharacteristic;         // BLE 신호 결과 처리
BLECharacteristic *pAuthCharacteristic;           // 장치 인증
BLECharacteristic *pControlCharacteristic;        // 데이터 동기화 컨드롤 처리
BLECharacteristic *pSyncCharacteristic;           // 데이터 동기화 실제 정보 처리

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

bool readRfidFlag = false;                        // 테그 정보 요청이 들어왔을 때 처리하는 플래그
bool deleteRfidFlag = false;                      // 테그 정보 삭제 요청이 들어왔을 때 처리하는 플래그
byte nuidPICC[4];                                 // RFID 태그 정보 저장 플래그

struct timeval tv;
struct timeval mytime;

bool deviceConnected = false;                     // ble 연결 시 스위치 역할을 하는 flag
bool oldDeviceConnected = false;                  // ble 연결 종료시 flag


volatile uint32_t count = 0;                       // 자계 감지 센서 인터럽트 카운트 변수 - 인터럽트 발생시 1씩 증가
volatile float distance = 0.0f;                    // 이동 거리 변수 (카운트와 1회전 이동거리와 곱해짐)
volatile float distanceUnitKm = 0.0f;              // km 단위로 환산하기 위한 변수 , 위의 distance를 활용함.
volatile float speedNow = 0.0f;
volatile uint16_t uintSpeedNow = 0;
volatile uint32_t uintTotalDistance = 0;

volatile uint16_t uintDistanceKm = 0;              // 이동 거리 float 자료형을 usigned로 변경해 무선 전송하기 위함. 또는 메모리에 저장.
volatile uint32_t sumDistanceKm = 0x0000;          // 평균을 구하기 위한 이동거리 변수 ( 더 해짐)
volatile uint32_t sumSpeed = 0x0000;               // 평균 속도를 구하기 위한 속도 합 변수 (더 해짐)
volatile uint32_t sumHeartRate = 0;                // 평균 심박수를 구하기 위한 변수

long t = 0;                                        // 센서 입력 외부 인터럽트 시간 변수 .
volatile float InstantTime = 0.0f;                 // 순간 속도 연산을 위한 델타 t 시간 변수

uint8_t heartRateData[] = {0b00000010, 0x00};
uint8_t treadmillData[] = {0x05, 0x00, 0x00, 0x00, 0x00};
uint8_t indoorBikeData[] = {0x00, 0x00, 0x00, 0x00};
uint8_t authData[] = {0x00, 0x00, 0x00};
uint8_t resultPacket[] = {0x00, 0x00, 0x00};        // 결과 값 반환 패킷 데이터 버퍼
boolean deviceConnectedFlag = false;                //장비가 블루투스 연결이 되었는지 확인하는 플래그
boolean fitnessStartOrEndFlag = false;              // 운동의 시작과 종료를 판단하는 플레그

long previousMillis = 0;                            // last time the battery level was checked, in ms
long realTimePreviousMillis = 0;                    // 실시간운동 정보 초기화를 위한 한계 시간 약 3초 이상 인터럽트가 없으면 초기화한다.

volatile long startFitnessTime = 0;                 // 운동 시작 시각 저장 변수
volatile long endFitnessTime = 0;                   // 운동 종료 시각 저장 변수
volatile long workoutTime = 0;                      // 운동 시간 저장 변수

//심박수 처리
uint8_t globalHeartRate = 0;                        // 심박수 전역 변수
uint16_t heartRateCount = 0;                        // 심박수 평균을 구하기위한 계수 카운터 변수
boolean heartRateMeasureFlag = false;               // 심박 센서 착용여부 플레그
boolean heartRateLocationFlag = false;              // 심박 센서 위치 확인플레그

boolean bleDateTimeSycnFlag = false;                // 블루투스를 통해 시간 동기화가 되었을시 처리하는 플래그
boolean bleAuthCheckFlag = false;                   // 사용자 인증을 위한 플래그 실패시 false 성공시 true
boolean bleDataSyncFlag = false;                    // 데이터 전송 요청 이 들어왔을겨우 올바른 데이터 형식이면 true 아니면 false


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {14, 0, false};                      // 심박 센서
Button button2 = {15, 0, false};                      // 자계 감지 센서

void IRAM_ATTR isr() {                                // 자계감지 센서 External Interrupt Function
  button2.numberKeyPresses += 1;
  button2.pressed = true;

  if (count == 0) {
    fitnessStartOrEndFlag = true;
    startFitnessTime = millis();
  }
  long intterrupt_time = millis();

  // ISR에 들어온 시간 - 이전 저장 값
  // 클럭 카운트의 계수가 2개식 증가하는 현상을 잡아준다.
  if ( intterrupt_time - t >  50 ) {
    count++; //자계 센서로 부터 외부인터럽트가 발생하면 1씩 증가시키도록
#ifdef ERGOMETER_LEXPA
    distance = (LEXPA_DISTANCE * count); // 총 이동 거리 --> 단위 넣을것
    distanceUnitKm = distance / 1000.0f; // 단위 km
    uintDistanceKm = (distanceUnitKm * 100);

    InstantTime = (float)(intterrupt_time - t) / 1000.0f;
    speedNow = 3.6f * (LEXPA_DISTANCE / InstantTime);  // 현재 속도
    uintSpeedNow = (speedNow * 100);  // 애플리케이션으로 실시간 전송을 위해 100을 곱한다.
    uintTotalDistance = (uint32_t) distance; // 애플리케이션으로 실시간 운동 거리 전송을 위해 케스팅한다. 단위 m
    workoutTime = intterrupt_time - startFitnessTime;

    //평균 구하기위한 더하기 연산
    sumSpeed += uintSpeedNow;   //속도 합
    sumDistanceKm += uintDistanceKm; // 거리합 km를 100 곱한 값

#else
    distance = (ONE_ROUND_DISTANCE * count); // 총 이동 거리
    distanceUnitKm = distance / 1000.0f;
    InstantTime = (float)(intterrupt_time - t) / 1000.0f;
    speedNow = 3.6f * (ONE_ROUND_DISTANCE / InstantTime);
    uintSpeedNow = (speedNow * 100);
    uintTotalDistance = distance * 100;
    workoutTime = intterrupt_time - startFitnessTime;
#endif

    //  speedNow = 3.6f * (distance / (float)(intterrupt_time - t));

    t = millis(); //시간 저장
  }

}


class MyServerCallbacks: public BLEServerCallbacks {    // BLE 연결 Callback Class
    void onConnect(BLEServer* pServer) { // inner function
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

void sdCardInit() {

  if (!SD.begin(4)) { // cs를 4로 설정합니다.
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sdCardInit();
  tv.tv_sec = 1540885090;
  settimeofday(&tv, NULL);

  pinMode(button1.PIN, INPUT_PULLUP);             // 심박 센서 GPIO 핀처리
  //    attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);             // 자계감지센터 GPIO 처리
  attachInterrupt(button2.PIN, isr, FALLING);     // Extenal Interrupt Connection

  //-------------------------------------------------------------- BLE Profile Setting

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


  BLEService *pDateTimeService = pServer -> createService(DATETIME_SERVICE_UUID);
  pDateTimeSyncCharacteristic = pDateTimeService -> createCharacteristic(CHARACTERISTIC_DATE_TIME_SYNC,
                                BLECharacteristic::PROPERTY_WRITE |
                                BLECharacteristic::PROPERTY_READ);

  pResultCharacteristic = pDateTimeService -> createCharacteristic(CHARACTERISTIC_RESULT_CHAR,
                          BLECharacteristic::PROPERTY_WRITE |
                          BLECharacteristic::PROPERTY_READ |
                          BLECharacteristic::PROPERTY_NOTIFY );
  pResultCharacteristic->addDescriptor(new BLE2902());

  BLEService *pUserAuthService = pServer -> createService(AUTH_SERVICE_UUID);
  pAuthCharacteristic = pUserAuthService -> createCharacteristic(CHARACTERISTIC_AUTH,
                        BLECharacteristic::PROPERTY_WRITE |
                        BLECharacteristic::PROPERTY_READ);

  BLEService *pDataSyncService = pServer -> createService(DATA_SYNC_SERVICE_UUID);
  pControlCharacteristic = pDataSyncService -> createCharacteristic(CHARACTERISTIC_CONTROL,
                           BLECharacteristic::PROPERTY_WRITE |
                           BLECharacteristic::PROPERTY_READ);

  pSyncCharacteristic = pDataSyncService -> createCharacteristic(CHARACTERISTIC_SYNC,
                        BLECharacteristic::PROPERTY_WRITE |
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_NOTIFY );
  pSyncCharacteristic->addDescriptor(new BLE2902());


  pService->start();                          // Start the service
  pFitnessMachineService -> start();          // FitnessMachineService 시작
  pDateTimeService -> start();                // 날짜 시간 동기화 시작
  pUserAuthService -> start();                // 장비 인증 서비스 시작
  pDataSyncService -> start();                // 정보 동기화 서비스 시작 
  pServer->getAdvertising()->start();         // BLE 서버 동작 시작 (Start advertising)
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  // put your main code here, to run repeatedly:

  //  if (button1.pressed) {
  //    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
  //    button1.pressed = false;
  //  }
  //  if (button2.pressed) {
  //    Serial.printf("Button 2 has been pressed %u times\n", button2.numberKeyPresses);
  //    button2.pressed = false;
  //  }

  if (deviceConnected) { // 만약 블루투스 연결이 되었다면
    if (fitnessStartOrEndFlag) { // 블루투스 연결은 되어 있는 상태에서 운동 중일 때만 실시간 전송이 되도록

      long currentMillis = millis();    // 현재 시스템 시간 저장
      // if 200ms have passed, check the battery level:
      if (currentMillis - previousMillis >= 1000) {  // 1초마다 데이터 값 업데이트
        previousMillis = currentMillis;

        indoorBikeData[2] = (uint8_t)(uintSpeedNow & 0xFF);
        indoorBikeData[3] = (uint8_t)(uintSpeedNow >> 8) & 0xFF;

        treadmillData[2] = (uint8_t)(uintTotalDistance & 0xFF);
        treadmillData[3] = (uint8_t)(uintTotalDistance >> 8) & 0xFF;
        treadmillData[4] = (uint8_t)(uintTotalDistance >> 16) & 0xFF;

        pIndoorBikeCharacteristic->setValue(indoorBikeData, 4);
        pIndoorBikeCharacteristic->notify();
        pTreadmillCharacteristic->setValue(treadmillData, 5);
        pTreadmillCharacteristic->notify();

        //        indorBikeChar.setValue(indoorBikeData, 4);
        //        treadmillChar.setValue(treadmillData, 5);
        //        updateBatteryLevel();

        Serial.println("ble ok , workout ok");
      }

      if (currentMillis - t > REAL_TIME_STOP_MILLIS) {  // 운동 중이면서 만약 3초동안 인터럽트 발생이 없다면 실시간 운동 변수 초기화
        InstantTime = 0;
        speedNow = 0;
        uintSpeedNow = 0;
      }


    } else { // 블루투스 연결은 되어있고 운동중이지 않을때
      Serial.println("ble ok , workout no");
    }
  } else { // 앱과 블루투스 연결이 안되었다면
    if (fitnessStartOrEndFlag) { // 블루투스 연결되지 않고 운동 중일 때
      Serial.println("ble no , workout ok");
      long realTimeCurrentTimeMillis = millis();  // 현재 시스템 시간을 가져온다.

      if (realTimeCurrentTimeMillis - t > REAL_TIME_STOP_MILLIS) {  // 운동 중이면서 만약 3초동안 인터럽트 발생이 없다면 실시간 운동 변수 초기화
        InstantTime = 0;
        speedNow = 0;
        uintSpeedNow = 0;
      }

      Serial.print("count -> "); Serial.print(count); Serial.print("| instant Time  -> "); Serial.print(InstantTime);
      Serial.print("| workout Time  -> "); Serial.print(workoutTime);
      Serial.print("| distance -> "); Serial.print(distance);   Serial.print("| distance m to Km-> "); Serial.print(distanceUnitKm);
      Serial.print(" | Speed ->"); Serial.print(speedNow); Serial.print(" | Speed * 100 ->"); Serial.println(uintSpeedNow);

      // 운동 종료 시
      // 모든 변수 초기화 및 플레그 초기화
      // SD카드 저장 작업 처리
      if (realTimeCurrentTimeMillis  - t > WORKOUT_DONE_TIME_MILLIS) { // 운동 중 플레그가 high이고 (운동 중 이지만) 30초 동안 동작이 없으면 운동 종료 판단


        // 운동 종료시 모든 변수 초기화
        fitnessStartOrEndFlag = false;
        t = 0;
        count = 0;              // 자계감지 인터럽트 카운트
        distance = 0;           // 이동거리
        distanceUnitKm = 0;     // 이동거리
        startFitnessTime = 0;   // 운동 시작 시간
        endFitnessTime = 0;     // 운동 종료 시간
        sumSpeed = 0;           // 속도 총합
        workoutTime = 0;        // 운동 시간
        uintDistanceKm = 0;     // unsigned 이동거리
        sumDistanceKm = 0x0000; // 이동거리 총합
        sumSpeed = 0x0000;      // 운동 속도 총합
        sumHeartRate = 0;       // 심박수 총합
        Serial.println("운동 종료처리");

      }

    } else { // 블루투스 연결되지 않고 운동중이지 않을때
      Serial.println("ble no , workout no");
    }
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

