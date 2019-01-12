

void setup() {
  // put your setup code here, to run once:

  epd_init();
  epd_wakeup();
  epd_set_memory();

}

void loop() {
  draw_text_demo();
  delay(1000);
  draw_text();
  delay(1000);
  draw_line(); 
  delay(1000);
  // put your main code here, to run repeatedly:

}
