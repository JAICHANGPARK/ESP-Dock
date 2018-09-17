void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(SDA);
  Serial.println(SCL);
}

void loop() {
  // put your main code here, to run repeatedly:
  int soil_moisture = analogRead(A0);
  Serial.println(soil_moisture);
  delay(1000);
}
