#include <Arduino.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "soc/rtc.h"
#include <sys/time.h>


#define SD_CARD_CS_PIN  4
#define LOADCELL_SCALE 1967.96f


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {15, 0, false};

struct timeval tv;
struct timeval mytime;

void IRAM_ATTR isr() {
  button1.numberKeyPresses += 1;
  button1.pressed = true;
}

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
    void power_down();
    void power_up();

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

void Hx711::power_down() {
  digitalWrite(SCK, LOW);
  digitalWrite(SCK, HIGH);
}

void Hx711::power_up() {
  digitalWrite(SCK, LOW);
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

  // 24z클럭
  for (unsigned int i = 0; i < 2; i++) {
    digitalWrite(SCK, HIGH);
    digitalWrite(SCK, LOW);
  }

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

Hx711 scale(13, 12);

long val = 0;
volatile float count = 0;
volatile long offset = 0;
volatile long offset_b = 0;

double average(int count) {
  double value = 0;
  for (int i = 0; i < count ; i++) {
    value += (scale.value() - offset) / LOADCELL_SCALE;
  }
  return value / (double)count;
}

boolean sdTestFlag = false;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
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

void createFile(fs::FS &fs, const char * path) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  } else {
    Serial.println("Create Successed to open file for writing");
    return;
  }
  file.close();
}

boolean initSDCard() {
  if (!SD.begin(SD_CARD_CS_PIN)) {
    Serial.println("Card Mount Failed");
    return false;
  } else {
    return true;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  } else {
    return true;
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


}
void setup() {
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);

  sdTestFlag = initSDCard();

  if (sdTestFlag) {
    createFile(SD, "/drug_log.csv");
  }

  for (int i = 0; i < 20; i++) {
    count += 1;
    Serial.print(" init offset_b :  "); Serial.println(scale.value());
    offset += scale.value();
  }
  offset  = offset / count;
  count = 0;

  scale.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scale.power_up();

  tv.tv_sec = 1541574461;
  settimeofday(&tv, NULL);

}


void loop() {
  if (button1.pressed) {
    ESP.restart();
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }

  val = scale.value();
  Serial.print("offset ==> "); Serial.print(offset);
  Serial.print(" | raw value 2: "); Serial.print(val);
  Serial.print(" | raw-offset: "); Serial.print((val - offset));
  Serial.print(" | cal : ");  Serial.print((val - offset) / LOADCELL_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average(20), 1);

  scale.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scale.power_up();

  gettimeofday(&mytime, NULL);
  Serial.println(mytime.tv_sec);


}
