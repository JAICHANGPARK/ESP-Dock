#include <Arduino.h>
#include <FunctionalInterrupt.h>

class Button
{
  public:
    Button(uint8_t reqPin) : PIN(reqPin) {
      pinMode(PIN, INPUT);
      attachInterrupt(PIN, std::bind(&Button::isr, this), FALLING);
    };
    ~Button() {
      detachInterrupt(PIN);
    }

    void IRAM_ATTR isr() {
      if ((millis() - lastDebounceTime) > debounceDelay) {
        numberKeyPresses += 1;
        pressed = true;
      }
      lastDebounceTime = millis();
    }

    void checkPressed() {
      if (pressed) {
        Serial.printf("Button on pin %u has been pressed %u times\n", PIN, numberKeyPresses);
        pressed = false;
      }
    }

  private:
    const uint8_t PIN;
    volatile uint32_t numberKeyPresses;
    volatile bool pressed;
    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers
};

#define BUTTON1 34
#define BUTTON2 17

Button button1(BUTTON1);
Button button2(BUTTON2);

void setup() {
  Serial.begin(115200);
}

void loop() {
  button1.checkPressed();
  button2.checkPressed();
}
