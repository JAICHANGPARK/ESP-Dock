#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
SSD1306Wire  screen(0x3c, 21, 22);

void setup() {
  // put your setup code here, to run once:
  screen.init();
  screen.clear();
  screen.drawString(0, 0, "Hello World");
  screen.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
