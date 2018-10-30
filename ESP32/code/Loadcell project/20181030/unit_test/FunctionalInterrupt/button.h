#ifndef BUTTON_H
#define BUTTON_H

class Button{
public:

void IRAM_ATTR isr();
void checkPressed();


 private:
    const uint8_t PIN;
    volatile uint32_t numberKeyPresses;
    volatile bool pressed;
    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 100;   

};

#endif