

#include <HX711_ADC.h>
#include "soc/rtc.h"

//HX711 constructor (dout pin, sck pin)
HX711_ADC LoadCell(25, 26);

long t;

void setup() {
  Serial.begin(9600);
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.println("Wait...");
  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(696.0); // user set calibration factor (float)
  Serial.println("Startup + tare is complete");
}

void loop() {
  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //longer delay in scetch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();

  //get smoothed value from data set + current calibration factor
  if (millis() > t + 250) {
    float i = LoadCell.getData();
    float v = LoadCell.getCalFactor();
    Serial.print("Load_cell output val: ");
    Serial.print(i);
    Serial.print("      Load_cell calFactor: ");
    Serial.println(v);
    t = millis();
  }

  //receive from serial terminal
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 'l') i = -1.0;
    else if (inByte == 'L') i = -10.0;
    else if (inByte == 'h') i = 1.0;
    else if (inByte == 'H') i = 10.0;
    else if (inByte == 't') LoadCell.tareNoDelay();
    if (i != 't') {
      float v = LoadCell.getCalFactor() + i;
      LoadCell.setCalFactor(v);
    }
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

}
