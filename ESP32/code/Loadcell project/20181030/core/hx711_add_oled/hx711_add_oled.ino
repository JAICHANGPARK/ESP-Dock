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


#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;


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

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
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



struct timeval tv;
struct timeval mytime;

long val, val2 = 0;
volatile float count = 0;
volatile long offset = 0;

volatile unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 150;    // the debounce time; increase if the output flickers

volatile float rice = 0.0f;

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

void setup() {

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);

  pinMode(5, OUTPUT);
  pinMode(button1.PIN, INPUT_PULLUP);
  pinMode(button2.PIN, INPUT_PULLUP);
  pinMode(button3.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
  attachInterrupt(button2.PIN, isr_2, FALLING);
  attachInterrupt(button3.PIN, isr_3, FALLING);
  Serial.begin(115200);
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

  readFile(SD, "/foo.txt");

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
  // Create the BLE Device
  BLEDevice::init("SmartTray");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE
                                          );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
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
    gettimeofday(&mytime, NULL);
    //    Serial.print("times ==> "); Serial.println(mytime.tv_sec);
    char x[18];
    //    char y[11];
    sprintf(x, "%3.1f,%ld\n", rice, mytime.tv_sec);

    appendFile(SD, "/foo.txt", x);
    //    appendFile(SD, "/foo.txt", y);
    readFile(SD, "/foo.txt");
    button2.pressed = false;
  }

  if (button3.pressed) { //식사 시작
    Serial.printf("Button 3 has been pressed %u times\n", button3.numberKeyPresses);
    button3.pressed = false;
  }

  if (deviceConnected) { // 블루투스 연결됨
    uint8_t real_time[2];
    double tmp = average(20);
    uint16_t tmp_uint = (uint16_t)(tmp * 100);
    real_time[0] = (uint8_t)(tmp_uint >> 8 & 0xff);
    real_time[1] = (uint8_t)(tmp_uint & 0xff);
    //    pTxCharacteristic->setValue(&real_time, 2);
    pTxCharacteristic->setValue(real_time, 2);
    pTxCharacteristic->notify();

    delay(1000);
  } else { // 블루투스 연결 안됌
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
