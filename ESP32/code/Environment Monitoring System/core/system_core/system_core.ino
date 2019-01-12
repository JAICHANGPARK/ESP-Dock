#include "GravityRtc.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <WEMOS_SHT3X.h>

SHT3X sht30(0x45);

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
GravityRtc rtc;     //RTC Initialization

void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Pressure Sensor Test"); Serial.println("");

  if (!bmp.begin())
  {
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");  /* There was a problem detecting the BMP085 ... check your connections */
    while (1);
  }
  displaySensorDetails();  /* Display some basic information on this sensor */

  rtc.setup();

}

void loop() {
  // put your main code here, to run repeatedly:

  rtc.read();
  Serial.print("   Year = ");//year
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

  sensors_event_t event;
  bmp.getEvent(&event);

  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    Serial.print("Pressure:    ");
    Serial.print(event.pressure);
    Serial.println(" hPa");

    /* Then convert the atmospheric pressure, and SLP to altitude         */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    Serial.print("Altitude:    ");
    Serial.print(bmp.pressureToAltitude(seaLevelPressure, event.pressure));
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Sensor error");
  }
  delay(1000);


}
