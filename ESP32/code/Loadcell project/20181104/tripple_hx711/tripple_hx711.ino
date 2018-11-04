#include "soc/rtc.h"

#define PART_ZERO_CHANNAL_A_SCALE 433.8f
#define PART_ZERO_CHANNAL_B_SCALE 112.12f
#define PART_ONE_CHANNAL_A_SCALE 428.97f
#define PART_ONE_CHANNAL_B_SCALE 103.48f
#define PART_TWO_CHANNAL_A_SCALE 428.97f
#define PART_TWO_CHANNAL_B_SCALE 106.68f



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

Hx711 scalePartZero(26, 25);  //part 0
Hx711 scalePartOne(14, 27);  //part 1
Hx711 scalePartTwo(13, 12);  //part 2

long valuePartZeroA, valuePartZeroB = 0;
long valuePartOneA, valuePartOneB = 0;
long valuePartTwoA, valuePartTwoB = 0;

volatile float count = 0;
volatile long offset_p0a = 0;
volatile long offset_p0b = 0;

volatile long offset_p1a = 0;
volatile long offset_p1b = 0;

volatile long offset_p2a = 0;
volatile long offset_p2b = 0;
//----------------------------------------------------------------------------------파트 0 평균 함수
double average(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scalePartZero.value();
    value += (scalePartZero.value() - offset_p0a) / PART_ZERO_CHANNAL_A_SCALE;
  }
  return value / (double)count;
}
double average_2(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scalePartZero.value_b();
    value += (scalePartZero.value_b() - offset_p0b) / PART_ZERO_CHANNAL_B_SCALE;
  }
  return value / (double)count;
}

//----------------------------------------------------------------------------------파트 1 평균 함수
double average_p1a(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scalePartOne.value();
    value += (scalePartOne.value() - offset_p1a) / PART_ONE_CHANNAL_A_SCALE;
  }
  return value / (double)count;
}


double average_p1b(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scalePartOne.value_b();
    value += (scalePartOne.value_b() - offset_p1b) / PART_ONE_CHANNAL_B_SCALE;
  }
  return value / (double)count;
}

//----------------------------------------------------------------------------------파트 2 평균 함수
double average_p2a(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scalePartTwo.value();
    value += (scalePartTwo.value() - offset_p2a) / PART_TWO_CHANNAL_A_SCALE;
  }
  return value / (double)count;
}


double average_p2b(int count) {
  double value = 0;
  double tmp = 0;
  for (int i = 0; i < count ; i++) {
    tmp = scalePartTwo.value_b();
    value += (scalePartTwo.value_b() - offset_p2b) / PART_TWO_CHANNAL_B_SCALE;
  }
  return value / (double)count;
}

void setup() {

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(115200);

  //-------------------------------------------------------------------- 파트 0 오프셋 초기화

  for (int i = 0; i < 10; i++) {
    count += 1;
    Serial.print("파트 0 init offset A :  "); Serial.println(scalePartZero.value());
    offset_p0a += scalePartZero.value();
  }
  offset_p0a  = offset_p0a / count;
  count = 0;

  scalePartZero.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scalePartZero.power_up();

  for (int i = 0; i < 10; i++) {
    count += 1;
    Serial.print("파트 0 init offset B:  "); Serial.println(scalePartZero.value_b());
    offset_p0b += scalePartZero.value_b();
  }
  offset_p0b  = offset_p0b / count;
  count = 0;

  //----------------------------------------------------------------- 파트 1 오프셋 초기화

  for (int i = 0; i < 10; i++) {
    count += 1;
    Serial.print(" 파트 1 init offset A :  "); Serial.println(scalePartOne.value());
    offset_p1a += scalePartOne.value();
  }
  offset_p1a  = offset_p1a / count;
  count = 0;

  scalePartOne.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scalePartOne.power_up();

  for (int i = 0; i < 10; i++) {
    count += 1;
    Serial.print(" 파트 1 init offset B:  "); Serial.println(scalePartOne.value_b());
    offset_p1b += scalePartOne.value_b();
  }
  offset_p1b  = offset_p1b / count;
  count = 0;
  //---------------------------------------------------------------


  //----------------------------------------------------------------- 파트 2 오프셋 초기화

  for (int i = 0; i < 10; i++) {
    count += 1;
    Serial.print(" 파트 2 init offset A :  "); Serial.println(scalePartTwo.value());
    offset_p2a += scalePartTwo.value();
  }
  offset_p2a  = offset_p2a / count;
  count = 0;

  scalePartTwo.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scalePartTwo.power_up();

  for (int i = 0; i < 10; i++) {
    count += 1;
    Serial.print(" 파트 2 init offset B:  "); Serial.println(scalePartTwo.value_b());
    offset_p2b += scalePartTwo.value_b();
  }
  offset_p2b  = offset_p2b / count;
  count = 0;
  //---------------------------------------------------------------

}

void loop() {
  long dummy = 0;
  //----------------------------------------------------------------- 파트 0 값 읽기 처리하기
  scalePartZero.power_up();
  dummy = scalePartZero.value();
  valuePartZeroA = scalePartZero.value();
  Serial.print("p0 offset a ==> "); Serial.print(offset_p0a);
  Serial.print(" | raw value 1: "); Serial.print(valuePartZeroA);
  Serial.print(" | raw-offset: "); Serial.print(valuePartZeroA - offset_p0a);
  Serial.print(" | cal : ");  Serial.print((valuePartZeroA - offset_p0a) / PART_ZERO_CHANNAL_A_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average(20), 1);

  scalePartZero.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scalePartZero.power_up();

  dummy = scalePartZero.value_b();
  valuePartZeroB = scalePartZero.value_b();
  Serial.print("p0 offset b ==> "); Serial.print(offset_p0b);
  Serial.print(" | raw value 1: "); Serial.print(valuePartZeroB);
  Serial.print(" | raw-offset: "); Serial.print(valuePartZeroB - offset_p0b);
  Serial.print(" | cal : ");  Serial.print((valuePartZeroB - offset_p0b) / PART_ZERO_CHANNAL_B_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average_2(20), 1);

  scalePartZero.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);

  //----------------------------------------------------------------------
  //------------------------------------------------------------------------ 파트 1 값 읽기 처리하기
  scalePartOne.power_up();
  dummy = scalePartOne.value();
  valuePartOneA = scalePartOne.value();
  Serial.print("p1 offset a ==> "); Serial.print(offset_p1a);
  Serial.print(" | raw value 1: "); Serial.print(valuePartOneA);
  Serial.print(" | raw-offset: "); Serial.print(valuePartOneA - offset_p1a);
  Serial.print(" | cal : ");  Serial.print((valuePartOneA - offset_p1a) / PART_ONE_CHANNAL_A_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average_p1a(20), 1);

  scalePartOne.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);
  scalePartOne.power_up();
  dummy = scalePartOne.value_b();
  valuePartOneB = scalePartOne.value_b();
  Serial.print("p1 offset b ==> "); Serial.print(offset_p1b);
  Serial.print(" | raw value 1: "); Serial.print(valuePartOneB);
  Serial.print(" | raw-offset: "); Serial.print(valuePartOneB - offset_p1b);
  Serial.print(" | cal : ");  Serial.print((valuePartOneB - offset_p1b) / PART_ONE_CHANNAL_B_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average_p1b(20), 1);
  scalePartOne.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);

  //------------------------------------------------------------------------ 파트 2 값 읽기 처리하기
  scalePartTwo.power_up();
  dummy = scalePartTwo.value();
  valuePartTwoA = scalePartTwo.value();
  Serial.print("p2 offset a ==> "); Serial.print(offset_p2a);
  Serial.print(" | raw value 1: "); Serial.print(valuePartTwoA);
  Serial.print(" | raw-offset: "); Serial.print(valuePartTwoA - offset_p2a);
  Serial.print(" | cal : ");  Serial.print((valuePartTwoA - offset_p2a) / PART_TWO_CHANNAL_A_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average_p2a(20), 1);

  scalePartTwo.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);  //1000us --> 1ms
  scalePartTwo.power_up();
  
  dummy = scalePartTwo.value_b();
  valuePartTwoB = scalePartTwo.value_b();
  Serial.print("p2 offset b ==> "); Serial.print(offset_p2b);
  Serial.print(" | raw value 1: "); Serial.print(valuePartTwoB);
  Serial.print(" | raw-offset: "); Serial.print(valuePartTwoB - offset_p2b);
  Serial.print(" | cal : ");  Serial.print((valuePartTwoB - offset_p2b) / PART_TWO_CHANNAL_B_SCALE, 1);
  Serial.print(" | averages :  "); Serial.println(average_p2b(20), 1);
  scalePartTwo.power_down();              // put the ADC in sleep mode
  delayMicroseconds(1000);

}
