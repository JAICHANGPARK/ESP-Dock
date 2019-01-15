#include <Ticker.h>

Ticker toggler;

int pushButton = 12;
volatile uint8_t hrCount = 0;
boolean hrTickerFlag = false;

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button2 = {12, 0, false};

//void IRAM_ATTR isr(void* arg) {
//  Button* s = static_cast<Button*>(arg);
//  s->numberKeyPresses += 1;
//  s->pressed = true;
//}

void tick(){
  hrCount = hrCount * 6;
  hrTickerFlag = true;
}
void IRAM_ATTR isr() {
  hrCount++;
  button2.numberKeyPresses += 1;
  button2.pressed = true;
}

// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(9600);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, isr, RISING);
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

  if(hrTickerFlag){
    Serial.println(hrCount);
    hrTickerFlag = false;
  }

}
