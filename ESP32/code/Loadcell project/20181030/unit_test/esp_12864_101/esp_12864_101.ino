

#include <Wire.h>
#include <SSD1306Wire.h>

SSD1306Wire display(0x78, 21, 22);
void setup() {
  // put your setup code here, to run once:
  display.init();
  display.drawString(0, 0, "Hello World");
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
