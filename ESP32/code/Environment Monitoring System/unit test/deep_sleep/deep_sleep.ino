#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include "GravityRtc.h"
#include <WEMOS_SHT3X.h>
//#include <Ticker.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */
#define DEBUG

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

//Ticker toggler;
//const float togglePeriod = 10; //seconds
//bool trag_flag = false;

SHT3X sht30(0x45);

volatile uint32_t fileIndex = 0;
String filePath = "/sht30_mornitoring/logger.csv";
GravityRtc rtc;     //RTC Initialization


//void toggle() {
//  trag_flag = true;
//  fileIndex++;
//}

void createDir(fs::FS &fs, const char * path) {
#ifdef DEBUG
  Serial.printf("Creating Dir: %s\n", path);
#endif
  if (fs.mkdir(path)) {
#ifdef DEBUG
    Serial.println("Dir created");
#endif
  } else {
#ifdef DEBUG
    Serial.println("mkdir failed");
#endif
  }
}
void createFile(fs::FS &fs, const char * path) {
#ifdef DEBUG
  Serial.printf("Writing file: %s\n", path);
#endif
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
#ifdef DEBUG
    Serial.println("Failed to open file for writing");
#endif
    return;
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
#ifdef DEBUG
    Serial.println("Failed to open file for writing");
#endif
    return;
  }
  if (file.println(message)) {
#ifdef DEBUG
    Serial.println("File written");
#endif
  } else {
#ifdef DEBUG
    Serial.println("Write failed");
#endif
  }
  file.close();
}
void setup() {
  // put your setup code here, to run once:


  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(12, OUTPUT);
  //  toggler.attach(togglePeriod, toggle);
#ifdef DEBUG
  Serial.begin(115200);
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  print_wakeup_reason(); //Print the wakeup reason for ESP32
#endif

  if (!SD.begin(4)) {
#ifdef DEBUG
    Serial.println("Card Mount Failed");
#endif
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
#ifdef DEBUG
    Serial.println("No SD card attached");
#endif
    return;
  }

#ifdef DEBUG
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
#endif
  createDir(SD, "/sht30_mornitoring");
  createFile(SD, "/sht30_mornitoring/logger.csv");

  rtc.setup();

}

void loop() {

  rtc.read();
  String dateMonth = rtc.month < 10 ? ("0" + String(rtc.month)) : String(rtc.month);
  String dateTimeString = String(rtc.year) + "-" + dateMonth + "-" + String(rtc.day) + " "
                          + String(rtc.hour) + ":" + String(rtc.minute)  + ":" + String(rtc.second);
#ifdef DEBUG
  Serial.println(dateTimeString);
#endif
  if (sht30.get() == 0) {
    String saveData = String(sht30.cTemp) + "," + String(sht30.humidity);
#ifdef DEBUG
    Serial.println(saveData);
#endif
    String logging = String(fileIndex) + "," + saveData + "," + dateTimeString;
    //      Serial.print("Temperature in Celsius : ");
    //      Serial.println(sht30.cTemp);
    //      Serial.print("Temperature in Fahrenheit : ");
    //      Serial.println(sht30.fTemp);
    //      Serial.print("Relative Humidity : ");
    //      Serial.println(sht30.humidity);
    //      Serial.println();

    const char * logging_path = filePath.c_str(); // c_str() 함수는 string 을 char* 형으로 변환
    const char * logging_c = logging.c_str();

    writeFile(SD, logging_path, logging_c);
  } else {
#ifdef DEBUG
    Serial.println("SHT30 Error!");
#endif
  }

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(12, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

#ifdef DEBUG
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush();
#endif
  esp_deep_sleep_start();



}
