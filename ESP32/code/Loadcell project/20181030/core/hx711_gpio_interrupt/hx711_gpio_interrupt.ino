#include <Arduino.h>
#include "soc/rtc.h"
#include <sys/time.h>


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


Button button1 = {34, 0, false};
Hx711 scale(26, 25);

long val, val2 = 0;
volatile float count = 0;
volatile long offset = 0;

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

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

void setup() {
  // put your setup code here, to run once:
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);

  for (int i = 0; i < 10; i++) {
    count += 1;
    offset += scale.value();
  }
  offset  = offset / count;
  count = 0;

  struct timeval tv;
  tv.tv_sec = 1540885090;
  settimeofday(&tv, NULL);
}

struct timeval mytime;
void loop() {

  gettimeofday(&mytime, NULL);
  Serial.print("times ==> "); Serial.println(mytime.tv_sec);
  // put your main code here, to run repeatedly:
  if (button1.pressed) {
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }

  val = scale.value();
  Serial.print("offset ==> "); Serial.print(offset); Serial.print(" | ");
  //Serial.print(scale.gram() * 2, 1);
  //채널 a의 오프셋을 맞추는 과정
  Serial.print("raw valeu: "); Serial.print(val); Serial.print(" | raw-offset: "); Serial.print(val - offset);
  Serial.print(" | ");  Serial.println((val - offset) / 433.8f, 1);
  Serial.print(" averages :  "); Serial.println(average(20), 1);

}
