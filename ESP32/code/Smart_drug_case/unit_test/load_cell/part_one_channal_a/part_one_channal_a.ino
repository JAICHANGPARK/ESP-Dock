#include "soc/rtc.h"

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

long val, val2 = 0;
volatile float count = 0;
volatile long offset = 0;
volatile long offset_b = 0;

double average(int count) {
  double value = 0;
  for (int i = 0; i < count ; i++) {
    value += (scale.value() - offset) / 433.8f;
  }
  return value / (double)count;
}

double average_2(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scale.value_b();
    value += (scale.value_b() - offset_b) / 112.12f;
  }
  return value / (double)count;
}

void setup() {

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(115200);

  for (int i = 0; i < 20; i++) {
    count += 1;
    Serial.print(" init offset_b :  "); Serial.println(scale.value());
    offset_b += scale.value();

  }
  offset_b  = offset_b / count;
  count = 0;

  scale.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scale.power_up();


}

void loop() {


  val2 = scale.value();
  Serial.print("offset ==> "); Serial.print(offset_b);
  Serial.print(" | raw value 2: "); Serial.print(val2);
  Serial.print(" | raw-offset: "); Serial.print(-(val2 - offset_b));
  Serial.print(" | cal : ");  Serial.print(-(val2 - offset_b) / 112.12f, 1);
  Serial.print(" | averages :  "); Serial.println(average(20), 1);

  scale.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scale.power_up();


}
