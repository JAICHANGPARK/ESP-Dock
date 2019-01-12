#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include "GravityRtc.h"
#include <WEMOS_SHT3X.h>
#include <Ticker.h>

Ticker toggler;

SHT3X sht30(0x45);
GravityRtc rtc;     //RTC Initialization
const float togglePeriod = 10; //seconds
bool trag_flag = false;

void toggle() {
  trag_flag = true;
}

void setup() {
  // put your setup code here, to run once:
  toggler.attach(togglePeriod, toggle);
  Serial.begin(115200);

  if (!SD.begin(4)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  rtc.setup();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (trag_flag) {
    rtc.read();
    Serial.print("Year = ");//year
    Serial.print(rtc.year);
    Serial.print("   Month = ");//month
    Serial.print(rtc.month);
    Serial.print("   Day = ");//day
    Serial.print(rtc.day);
    Serial.print("   Week = ");//week
    Serial.print(rtc.week);
    Serial.print("   Hour = ");//hour
    Serial.print(rtc.hour);
    Serial.print("   Minute = ");//minute
    Serial.print(rtc.minute);
    Serial.print("   Second = ");//second
    Serial.println(rtc.second);
    if (sht30.get() == 0) {
      Serial.print("Temperature in Celsius : ");
      Serial.println(sht30.cTemp);
      Serial.print("Temperature in Fahrenheit : ");
      Serial.println(sht30.fTemp);
      Serial.print("Relative Humidity : ");
      Serial.println(sht30.humidity);
      Serial.println();
    } else {
      Serial.println("SHT30 Error!");
    }


    trag_flag = false;

  }


}
