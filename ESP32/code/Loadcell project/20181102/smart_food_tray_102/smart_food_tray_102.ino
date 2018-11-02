#include "mbedtls/md.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <Arduino.h>
#include "soc/rtc.h"
#include <sys/time.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SH1106Wire.h"


//#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
//#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
//#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define SERVICE_UUID                        "0000ffe0-0000-1000-8000-00805f9b34fb" // UART service UUID
#define CHARACTERISTIC_UUID_RX              "0000ffe1-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_TX              "0000ffe2-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID_REALTIME        "0000ffe3-0000-1000-8000-00805f9b34fb"


BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
BLECharacteristic * pRealTimeCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

struct timeval tv;
struct timeval mytime;

boolean firstPhase  = false;
boolean secondPhase = false;
boolean checkAuth = false;

boolean realtimeFirstPhase = false;

volatile uint8_t receivedCallback[20] = {0,};
uint32_t receivedTime = 0;

char *payload = "smartfoodtray";
byte shaResult[32];

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(5, false);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(5, true);
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
        //        for (int i = 0; i < rxValue.length(); i++) {
        //          Serial.print(tmp[i]);
        //        }
        //        for (int i = 0; i < rxValue.length(); i++) {
        //          Serial.write(tmp[i]);
        //        }

        if (rxValue[0] == 0x02 && rxValue[1]  == 0x07 && rxValue[2] == 0x70 && rxValue[3] == 0x03) { // 실시간 전송 첫번째 인증과정
          realtimeFirstPhase = true;

        }

        if (rxValue[0] == 0x02 && rxValue[1]  == 0x01 && rxValue[6] == 0x03) {
          firstPhase = true;
          receivedTime = ((tmp[2] << 24) & 0xff000000)
                         | ((tmp[3] << 16) & 0x00ff0000)
                         | ((tmp[4] << 8) & 0x0000ff00)
                         | (tmp[5] & 0x000000ff);
          Serial.println(receivedTime);
          tv.tv_sec = receivedTime;
          settimeofday(&tv, NULL);
        } else if (rxValue[0] == 0x02 && rxValue[1] == 0x02 && rxValue[19] == 0x03) {
          Serial.println("동기화 콜백 들어옴");
          secondPhase = true;
          for (int i = 0; i < rxValue.length(); i++) {
            receivedCallback[i] = rxValue[i];
          }
        }
      }
    }
};

class Hx711
{
  public:
    Hx711(uint8_t pin_din, uint8_t pin_slk);
    virtual ~Hx711();
    long value();
    long value_b();
    long nomalvalue(byte times = 32);
    void setOffset(long offset);
    void setScale(float scale = 742.f);
    float gram();

  private:
    const uint8_t DOUT;
    const uint8_t SCK;
    long _offset;
    float _scale;
};

Hx711::Hx711(uint8_t pin_dout, uint8_t pin_slk) :
  DOUT(pin_dout), SCK(pin_slk)
{
  pinMode(SCK, OUTPUT);
  pinMode(DOUT, INPUT);

  digitalWrite(SCK, HIGH);
  delayMicroseconds(100);
  digitalWrite(SCK, LOW);

  nomalvalue();
  this->setOffset(nomalvalue());
  this->setScale();
}

Hx711::~Hx711()
{
}

long Hx711::nomalvalue(byte times)
{
  long sum = 0;
  for (byte i = 0; i < times; i++)
  {
    sum += value();
  }

  return sum / times;
}

long Hx711::value()
{
  byte data[3];

  while (digitalRead(DOUT))
    ;

  for (byte j = 3; j--;)
  {
    for (char i = 8; i--;)
    {
      digitalWrite(SCK, HIGH);
      bitWrite(data[j], i, digitalRead(DOUT));
      digitalWrite(SCK, LOW);
    }
  }

  digitalWrite(SCK, HIGH);
  digitalWrite(SCK, LOW);

  data[2] ^= 0x80;

  return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
}

long Hx711::value_b()
{
  byte data[3];

  while (digitalRead(DOUT))
    ;

  for (byte j = 3; j--;)
  {
    for (char i = 8; i--;)
    {
      digitalWrite(SCK, HIGH);
      bitWrite(data[j], i, digitalRead(DOUT));
      digitalWrite(SCK, LOW);
    }
  }

  digitalWrite(SCK, HIGH);
  digitalWrite(SCK, LOW);
  digitalWrite(SCK, HIGH);
  digitalWrite(SCK, LOW);

  data[2] ^= 0x80;

  return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
}

void Hx711::setOffset(long offset)
{
  _offset = offset;
}

void Hx711::setScale(float scale)
{
  _scale = scale;
}

float Hx711::gram()
{
  long val = (nomalvalue() - _offset);
  return (float) val / _scale;
}

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Hx711 scale(26, 25);

// Initialize the OLED display using Wire library
SH1106Wire  display(0x3c, 21, 22);

Button button1 = {34, 0, false};
Button button2 = {32, 0, false};
Button button3 = {33, 0, false};

long val, val2 = 0;
volatile float count = 0;
volatile long offset = 0;

volatile unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 150;    // the debounce time; increase if the output flickers
long startIntakeTime = 0;  // 섭취 시작 시간
volatile float rice = 0.0f;
volatile float soup = 0.0f;
volatile float sideA = 0.0f;
volatile float sideB = 0.0f;
volatile float sideC = 0.0f;
volatile float sideD = 0.0f;

boolean toggle = false;


double average(int count) {
  double value = 0;
  for (int i = 0; i < count ; i++) {
    value += (scale.value() - offset) / 433.8f;
  }
  return value / (double)count;
}

void IRAM_ATTR isr() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    button1.numberKeyPresses += 1;
    button1.pressed = true;

    offset = 0;
    count = 0;
    for (int i = 0; i < 10; i++) {
      count += 1;
      offset += scale.value();
    }
    offset  = offset / count;
    count = 0;
  }
  lastDebounceTime = millis();
}

void IRAM_ATTR isr_2() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    button2.numberKeyPresses += 1;
    button2.pressed = true;

  }
  lastDebounceTime = millis();
}

void IRAM_ATTR isr_3() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    button3.numberKeyPresses += 1;
    button3.pressed = true;

  }
  lastDebounceTime = millis();
}



void createFile(fs::FS &fs, const char * path) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.close();
}


void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
    //    file.readStringUntil('\n');
  }
  file.close();
}

void readFileForBle(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    //    Serial.println("Failed to open file for reading");
    return;
  }

  int count = 0;
  uint8_t syncBuffer[20] = {};
  uint8_t tmpBuffer[50] = {0,};
  //  Serial.print("Read from file: ");

  while (file.available()) {
    int fileRead = file.read();
    //    Serial.println(fileRead);

    tmpBuffer[count++] = (uint8_t)fileRead;
    if (fileRead == 0x0A) {

      for (int k = 0; k < 8 ; k++) {
        int buffIndex = 2 * k;  // 무선 통신으로 전송할 버퍼에 담을 인덱스
        int index = 4 * k;     // sd에서 읽어온 데이터 처리 인덱스
        String tmp =  String((char)tmpBuffer[index]) + String((char)tmpBuffer[index + 1]) + String((char)tmpBuffer[index + 2]);
        int riceInt = tmp.toInt();
        //        Serial.print("String parse : "); Serial.println(tmp);
        //        Serial.print("String to int: "); Serial.println(riceInt);
        syncBuffer[buffIndex] = (uint8_t)((riceInt << 8) & 0xff);
        syncBuffer[buffIndex + 1] = (uint8_t)(riceInt & 0xff);
        if (k == 6) {  //시작시간 처리
          String tmp =  String((char)tmpBuffer[index]) + String((char)tmpBuffer[index + 1]) + String((char)tmpBuffer[index + 2])
                        + String((char)tmpBuffer[index + 3]) + String((char)tmpBuffer[index + 4]) + String((char)tmpBuffer[index + 5])
                        + String((char)tmpBuffer[index + 6]) + String((char)tmpBuffer[index + 7]) + String((char)tmpBuffer[index + 8])
                        + String((char)tmpBuffer[index + 9]);
          long startTimeValule = tmp.toInt();
          //          Serial.print("time : String parse : "); Serial.println(tmp);
          //          Serial.print("time String to int: "); Serial.println(startTimeValule);
          syncBuffer[buffIndex] = (uint8_t)((startTimeValule >> 24) & 0xff);
          syncBuffer[buffIndex + 1] = (uint8_t)((startTimeValule >> 16) & 0xff);
          syncBuffer[buffIndex + 2] = (uint8_t)((startTimeValule >> 8) & 0xff);
          syncBuffer[buffIndex + 3] = (uint8_t)((startTimeValule) & 0xff);

        }
        if (k > 6) { // 종료시간 처리
          index += 7;
          buffIndex += 2;
          String tmp =  String((char)tmpBuffer[index]) + String((char)tmpBuffer[index + 1]) + String((char)tmpBuffer[index + 2])
                        + String((char)tmpBuffer[index + 3]) + String((char)tmpBuffer[index + 4]) + String((char)tmpBuffer[index + 5])
                        + String((char)tmpBuffer[index + 6]) + String((char)tmpBuffer[index + 7]) + String((char)tmpBuffer[index + 8])
                        + String((char)tmpBuffer[index + 9]);
          long endTimeValule = tmp.toInt();
          //          Serial.print("time : String parse : "); Serial.println(tmp);
          //          Serial.print("time String to int: "); Serial.println(endTimeValule);
          syncBuffer[buffIndex] = (uint8_t)((endTimeValule >> 24) & 0xff);
          syncBuffer[buffIndex + 1] = (uint8_t)((endTimeValule >> 16) & 0xff);
          syncBuffer[buffIndex + 2] = (uint8_t)((endTimeValule >> 8) & 0xff);
          syncBuffer[buffIndex + 3] = (uint8_t)((endTimeValule) & 0xff);
        }
      }
      for (int i = 0 ; i < 20; i++) {
        Serial.print(syncBuffer[i], HEX); Serial.print("|");
      }
      pTxCharacteristic->setValue(syncBuffer, 20);
      pTxCharacteristic->notify();
      Serial.println("한줄의 끝을 읽음");
      count = 0;
    }
  }
  file.close();
}

void setup() {

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(115200);

  pinMode(5, OUTPUT);
  pinMode(button1.PIN, INPUT_PULLUP);
  pinMode(button2.PIN, INPUT_PULLUP);
  pinMode(button3.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
  attachInterrupt(button2.PIN, isr_2, FALLING);
  attachInterrupt(button3.PIN, isr_3, FALLING);


  if (!SD.begin(4)) {
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

  createFile(SD, "/log.csv");
  //  readFile(SD, "/foo.txt");

  //  appendFile(SD, "/foo.txt", "My World!\n");
  //  appendFile(SD, "/foo.txt", "dream World!\n");
  //  appendFile(SD, "/foo.txt", "walk World!\n");
  // put your setup code here, to run once:

  for (int i = 0; i < 10; i++) {
    count += 1;
    offset += scale.value();
  }
  offset  = offset / count;
  count = 0;

  tv.tv_sec = 1540885090;
  settimeofday(&tv, NULL);
  //--------------------------------------------------------------

  BLEDevice::init("SmartTray");  // Create the BLE Device


  pServer = BLEDevice::createServer();  // Create the BLE Server
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID); // Create the BLE Service
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);  // Create a BLE Characteristic
  pTxCharacteristic->addDescriptor(new BLE2902());

  pRealTimeCharacteristic = pService -> createCharacteristic(CHARACTERISTIC_UUID_REALTIME, BLECharacteristic::PROPERTY_NOTIFY);
  pRealTimeCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();// Start the service
  pServer->getAdvertising()->start();// Start advertising
  Serial.println("Waiting a client connection to notify...");

  display.init();
  display.flipScreenVertically();
  display.drawString(0, 0, "Hello World");
  display.display();

}


void loop() {
  // put your main code here, to run repeatedly:

  if (button1.pressed) { //제로 세트
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }

  if (button2.pressed) { //식사 종료
    Serial.printf("Button 2 has been pressed %u times\n", button2.numberKeyPresses);
    rice = average(20);
    soup = rice;
    sideA = rice;
    sideB = rice;
    sideC = rice;
    sideD = rice;
    gettimeofday(&mytime, NULL);
    //    Serial.print("times ==> "); Serial.println(mytime.tv_sec);
    char x[46] = {};
    //    char y[11];
    sprintf(x, "%3.0f,%3.0f,%3.0f,%3.0f,%3.0f,%3.0f,%ld,%ld\n",
            rice, soup, sideA, sideB, sideC, sideD, startIntakeTime, mytime.tv_sec);

    appendFile(SD, "/log.csv", x);
    //    appendFile(SD, "/foo.txt", y);
    //    readFile(SD, "/foo.txt");

    readFileForBle(SD, "/log.csv");
    button2.pressed = false;
  }

  if (button3.pressed) { //식사 시작
    Serial.printf("Button 3 has been pressed %u times\n", button3.numberKeyPresses);
    //시간 정보 저장
    gettimeofday(&mytime, NULL);
    startIntakeTime = mytime.tv_sec;
    Serial.print("식사 시작 : 시작시간 ==> ");
    Serial.println(startIntakeTime);
    // 중량 정보 저장

    button3.pressed = false;
  }

  if (deviceConnected) { // 블루투스 연결됨
    if (firstPhase && !secondPhase) { // 첫번째 동기화 처리

      Serial.println("시간 동기화 완료 ");
      firstPhase = false;

    } else if (!firstPhase && secondPhase) { // 두번째 동기화 처리

      mbedtls_md_context_t ctx;
      mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

      const size_t payloadLength = strlen(payload);

      mbedtls_md_init(&ctx);
      mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
      mbedtls_md_starts(&ctx);
      mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
      mbedtls_md_finish(&ctx, shaResult);
      mbedtls_md_free(&ctx);

      for (int i = 0; i < 17; i++) { // Hash 값 비교 과정
        if (receivedCallback[i + 2] == shaResult[i]) {
          checkAuth = true;
        } else {
          checkAuth = false;
        }
      }
      if (checkAuth) {
        Serial.println("장비 인증 성공");
        secondPhase = false;
      } else {
        Serial.println("장비 인증 실패");
      }

    } else if (!firstPhase && !secondPhase && checkAuth ) { //모든 처리과정이 끝나고 장비 인증
      Serial.println("SD 로부터 데이터 가져오기 ");
      readFileForBle(SD, "/log.csv");
      checkAuth = false;
      Serial.println("SD 로부터 데이터 가져오기 완료했음.");
    }

    else { //실시간 데이터 전송

      if (realtimeFirstPhase) { // 첫번째 인증 과정

        uint8_t firstBuffer[3] = {0x02, 0x10, 0x03};
        pRealTimeCharacteristic->setValue(firstBuffer, 3);
        pRealTimeCharacteristic->notify();

        realtimeFirstPhase = false;
        uint8_t real_time[2];
        double tmp = average(20);
        uint16_t tmp_uint = (uint16_t)(tmp * 100);
        real_time[0] = (uint8_t)(tmp_uint >> 8 & 0xff);
        real_time[1] = (uint8_t)(tmp_uint & 0xff);
        //    pTxCharacteristic->setValue(&real_time, 2);
        pRealTimeCharacteristic->setValue(real_time, 2);
        pRealTimeCharacteristic->notify();
        delay(1000);
        
      }
    }
  } else { // 블루투스 연결 안됌 디스플레이 표기
    val = scale.value();
    Serial.print("offset ==> "); Serial.print(offset); Serial.print(" | ");
    //Serial.print(scale.gram() * 2, 1);
    //채널 a의 오프셋을 맞추는 과정
    Serial.print("raw valeu: "); Serial.print(val); Serial.print(" | raw-offset: "); Serial.print(val - offset);
    Serial.print(" | ");  Serial.println((val - offset) / 433.8f, 1);
    Serial.print(" averages :  "); Serial.println(average(20), 1);
    gettimeofday(&mytime, NULL);
    Serial.print("times ==> "); Serial.println(mytime.tv_sec);
    Serial.println(digitalRead(34));
    String dis = String(average(20));

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Rice");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, dis + "g");

    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, "soup");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 10, dis + "g");
    display.drawHorizontalLine(0, 30, 128);

    //    display.setFont(ArialMT_Plain_10);
    //    display.drawString(0, 31, "side a");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 31, dis + "g");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 47, dis + "g");

    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 31, dis + "g");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 47, dis + "g");
    display.display();
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
