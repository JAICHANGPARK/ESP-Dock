#include <Ticker.h>

#define DF_SON3130
//#define SEED_HR_EAR
struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button2 = {14, 0, false};
Ticker toggler;

int pushButton = 12;
volatile int hrCount = 0;
volatile boolean hrTickerFlag = false;
const float togglePeriod = 10; //seconds

volatile uint32_t nowTim = 0, lastTim = 0;
//void IRAM_ATTR isr(void* arg) {
//  Button* s = static_cast<Button*>(arg);
//  s->numberKeyPresses += 1;
//  s->pressed = true;
//}

void tick() {
  
#ifdef DF_SON3130
  hrCount = (hrCount * 6) / 2 ;
#elif SEED_HR_EAR
  hrCount = (hrCount * 6)  ;
#endif

  Serial.print("Heartrate : ");
  Serial.println(hrCount);
  hrTickerFlag = true;
}

void IRAM_ATTR isr() {
  nowTim = millis();
  uint32_t difTime =  nowTim - lastTim;
  lastTim = nowTim;
  if (difTime > 300 || difTime < 2000) {
    Serial.println("Beat \n");
    hrCount++;
  }

  //  button2.numberKeyPresses += 1;
  //  button2.pressed = true;
}

// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(9600);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, isr, FALLING);
  toggler.attach(togglePeriod, tick);
  hrCount = 0;
  //  pinMode(pushButton, INPUT);
  //  pinMode(button1.PIN, INPUT_PULLUP);
  //  attachInterruptArg(button1.PIN, isr, &button1, FALLING);

}

void loop() {
  // read the input pin:
  //  int buttonState = digitalRead(pushButton);
  //  // print out the state of the button:
  //  Serial.println(buttonState);
  //  delay(10);        // delay in between reads for stability
  //  if (button2.pressed) {
  //    Serial.printf("Button 2 has been pressed %u times\n", button2.numberKeyPresses);
  //    button2.pressed = false;
  //  }

  if (hrTickerFlag) {
    Serial.println(hrCount);
    hrTickerFlag = false;
    hrCount = 0;
  }

}
