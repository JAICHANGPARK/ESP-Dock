#include <Ticker.h>
Ticker toggler;


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {14, 0, false};
Button button2 = {15, 0, false};

int hr_cnt = 0; // 심박수 변수
int read_value, old_read;



void IRAM_ATTR flip()
{
  Serial.println("Beat - int\n");
  //    led = !led;
  hr_cnt++;
}

void calculate() {
  hr_cnt = hr_cnt * 6;
  Serial.print("Heartrate : ");
  Serial.println(hr_cnt);
  hr_cnt = 0;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  toggler.attach(10, calculate);
  pinMode(button1.PIN, INPUT_PULLUP);
  //    attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, flip, FALLING);

}

void loop() {
  // put your main code here, to run repeatedly:

}
