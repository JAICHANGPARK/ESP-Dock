#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <TinyGPS++.h>

#define SYSTEM_SERIAL_BAUD      115200
#define GPS_BAUD                9600

static const int RXPin = 16, TXPin = 17;
HardwareSerial myGps(2);
TinyGPSPlus gps; // The TinyGPS++ object

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void createFile(fs::FS &fs, const char * path) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}


void setup()
{
  Serial.begin(SYSTEM_SERIAL_BAUD);
  Serial.println("Start GPS... ");
  myGps.begin(GPS_BAUD, SERIAL_8N1);

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
  createFile(SD, "/gps_log.csv");


}
long delayTime = 0;

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  if (millis() - delayTime >= 1000) {
    while (myGps.available() > 0) {
      if (gps.encode(myGps.read())) {
        displayInfo();
      }
    }
    delayTime = millis();
  }



  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
  
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()){
    Serial.print(gps.location.lat());
    Serial.print(F(","));
    Serial.print(gps.location.lng());
    Serial.print("ALT=");  Serial.print(gps.altitude.meters());
    char x[20] = {};
    sprintf(x, "%7.4f,%7.4f\n", gps.location.lat(), gps.location.lng());
    appendFile(SD, "/gps_log.csv", x);
    //    Serial.print(gps.location.lat(), 6);
    //    Serial.print(F(","));
    //    Serial.print(gps.location.lng(), 6);
    //    Serial.print("ALT=");  Serial.print(gps.altitude.meters());

  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
