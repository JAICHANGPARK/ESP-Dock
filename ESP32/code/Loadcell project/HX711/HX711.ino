

// Hx711.DOUT - pin #A1
// Hx711.SCK - pin #A0
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

Hx711 scale(26, 25);

void setup() {

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(9600);

}
long val, val2 = 0;
float count = 0;
void loop() {

  //  count += 1;
  //  val = ((count - 1) / count) * val;
  //  val = val + (1 / count) * scale.value();
  //    val = scale.value();


  //  Serial.print(scale.gram() * 2, 1);
  //  Serial.print(scale.value(), HEX);
  //  Serial.println(" g");

  //  val = (0.7 * val) + (0.3 * scale.value());
  //  Serial.println((val - 8315648) / 434.42f);

  //  count += 1;
  //  val2 = ((count - 1) / count) * val2;
  //  val2 = val2 + (1 / count) * scale.value_b();
  //  val2 = scale.value_b();
  val2 = (0.7 * val2) + (0.3 * scale.value_b());
  Serial.println((val2 - 8368870) / (10925.0f / 100.0f)); 
  //  val2 = scale.value_b();

  //  Serial.print(scale.value_b());
  //  Serial.print("-->");
  //  Serial.println(scale.value_b(), HEX);

}

