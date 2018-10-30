#include <Arduino.h>

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

Button button1 = {18, 0, false};
Button button2 = {34, 0, false};

void IRAM_ATTR isr(void* arg) {
  Button* s = static_cast<Button*>(arg);
  s->numberKeyPresses += 1;
  s->pressed = true;
}

void IRAM_ATTR isr() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    button2.numberKeyPresses += 1;
    button2.pressed = true;
  }
  lastDebounceTime = millis();
}

void setup() {
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, isr, FALLING);
}

void loop() {
  if (button1.pressed) {
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }
  if (button2.pressed) {
    Serial.printf("Button 2 has been pressed %u times\n", button2.numberKeyPresses);
    button2.pressed = false;
  }
  //    static uint32_t lastMillis = 0;
  //    if (millis() - lastMillis > 10000) {
  //      lastMillis = millis();
  //      detachInterrupt(button1.PIN);
  //    }
}
