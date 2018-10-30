#include <SH1106.h>
#include <SH1106Fonts.h>
#include <SH1106Ui.h>

#include <Wire.h>

// Pin definitions for I2C
#define OLED_SDA    21  // pin 14
#define OLED_SDC    22  // pin 12
#define OLED_ADDR   0x3C

SH1106   display(OLED_ADDR, OLED_SDA, OLED_SDC);    // For I2C

void setup() {
  // put your setup code here, to run once:
  display.init();
  display.drawString(0, 0, "hellow");
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}
