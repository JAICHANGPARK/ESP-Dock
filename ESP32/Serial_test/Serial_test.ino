uint32_t while_loop_count = 0;
uint32_t loop_count = 0;

void setup() {
  Serial.begin(115200);
  delay (1000);
  Serial.println("Serial Port ok!");
}

void loop() {

  Serial.print("loop_count: "); Serial.println(loop_count);

  while (Serial.available())
  {
    Serial.print("inside while_loop_count, got: ");

    Serial.write(Serial.read());

    Serial.println();

    while_loop_count++;
  }

  Serial.print("while_loop_count: "); Serial.println(while_loop_count);

  loop_count++;

  delay (500);
}
