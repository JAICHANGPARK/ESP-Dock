char inByte;
boolean sendFlag = false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  while (Serial.available()) {
    // get incoming byte:
    inByte = Serial.read();
    // read first analog input, divide by 4 to make the range 0-255:
    Serial.write(inByte);
    if (inByte == 'r') {
      //      printHex(rfid.uid.uidByte, rfid.uid.size); // 1차 시행 : 초기에 태그가 찍히지 않으면 리턴 값 없음
      if (!sendFlag) {
        sendFlag = true;
      }
    }
  }
 
  if (sendFlag) {
    for (byte i = 0; i < 4; i++) {
      Serial.print("wow");
    }
    sendFlag = false;
  }

}
