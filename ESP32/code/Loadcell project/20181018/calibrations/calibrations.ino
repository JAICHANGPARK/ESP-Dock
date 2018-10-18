#include "HX711.h"
#include "soc/rtc.h"
//
//#define STEP_ONE
//#define STEP_TWO
//#define STEP_THREE
#define STEP_FOUR

//#define CHANNEL_A
//#define CHANNEL_B

HX711 scale;

void setup() {
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.begin(115200);
#ifdef CHANNEL_B
  scale.begin(26, 25, 32);
#elif defined(CHANNEL_A)
  scale.begin(26, 25, 128);
#else
  scale.begin(26, 25, 128);
#endif

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));    // print the average of 20 readings from the ADC
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));    // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
#ifdef STEP_ONE
  scale.set_scale(434.15f);          // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                // reset the scale to 0
#else
  scale.set_scale();          // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                // reset the scale to 0
#endif
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(10), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

#ifdef STEP_ONE
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(10), 1);
#endif

#ifdef STEP_TWO
  scale.set_gain(128);
  Serial.print("Loadcell A > ");
  Serial.print((scale.read())); // print a raw reading from the ADC channel B

  scale.set_gain(32);
  Serial.print(" Loadcell B > ");
  Serial.println((scale.read())); // print a raw reading from the ADC channel A
#endif

#ifdef STEP_THREE
  scale.set_scale(434.15f);
  scale.set_gain(128);
  Serial.print("Loadcell A > ");

  Serial.print(scale.get_units(10), 1); // print a raw reading from the ADC channel B

  scale.set_scale(107.87f);
  scale.set_gain(32);
  Serial.print(" Loadcell B > ");
  Serial.println(scale.get_units(10), 1); // print a raw reading from the ADC channel A
#endif

#ifdef STEP_FOUR
  scale.set_gain(128);
  Serial.print("Loadcell A > ");
  Serial.print(scale.get_units(10)/ 1057.19f, 1); // print a raw reading from the ADC channel B
  
  scale.set_gain(32);
  Serial.print(" Loadcell B > ");
  Serial.println(scale.get_units(10)/ 2057.19f, 1); // print a raw reading from the ADC channel A
#endif





}
