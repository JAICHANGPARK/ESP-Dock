#include <HX711_ADC.h>

//HX711 constructor (dout pin, sck pin)
HX711_ADC LoadCell(25, 26);

void setup() {

  Serial.begin(9600);
  Serial.println("Wait...");
  LoadCell.begin();
  // put your setup code here, to run once:


}

void loop() {
  LoadCell.update();
  Serial.println(LoadCell.smoothedData());
  // put your main code here, to run repeatedly:

}
