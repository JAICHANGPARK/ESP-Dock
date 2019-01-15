/*
  DigitalReadSerial
  Reads a digital input on pin 2, prints the result to the serial monitor

  This example code is in the public domain.
*/

// digital pin 2 has a pushbutton attached to it. Give it a name:
int pushButton = 12;

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

//Button button1 = {23, 0, false};
Button button2 = {12, 0, false};

//void IRAM_ATTR isr(void* arg) {
//  Button* s = static_cast<Button*>(arg);
//  s->numberKeyPresses += 1;
//  s->pressed = true;
//}

void IRAM_ATTR isr() {
  button2.numberKeyPresses += 1;
  button2.pressed = true;
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton's pin an input:
//  pinMode(pushButton, INPUT);

  //  pinMode(button1.PIN, INPUT_PULLUP);
  //  attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, isr, RISING);

}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
//  int buttonState = digitalRead(pushButton);
//  // print out the state of the button:
//  Serial.println(buttonState);
  delay(10);        // delay in between reads for stability
  if (button2.pressed) {
    Serial.printf("Button 2 has been pressed %u times\n", button2.numberKeyPresses);
    button2.pressed = false;
  }

}
