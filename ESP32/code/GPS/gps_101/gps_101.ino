HardwareSerial gps(2);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Start GPS... ");
  gps.begin(9600, SERIAL_8N1);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (gps.available())
  {
    Serial.write(gps.read());
  }

}
