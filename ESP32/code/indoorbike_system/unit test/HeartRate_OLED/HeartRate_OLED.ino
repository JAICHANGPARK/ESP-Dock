#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example


Ticker toggler;


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {14, 0, false};
Button button2 = {15, 0, false};

int hr_cnt = 0; // 심박수 변수
int read_value, old_read;

boolean display_flag = false;


void IRAM_ATTR flip()
{
  Serial.println("Beat - int\n");
  //    led = !led;
  hr_cnt++;
}

void calculate() {
  hr_cnt = hr_cnt * 6;
  Serial.print("Heartrate : ");
  Serial.println(hr_cnt);
  display_flag = true;
  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  toggler.attach(10, calculate);
  pinMode(button1.PIN, INPUT_PULLUP);
  //    attachInterruptArg(button1.PIN, isr, &button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, flip, FALLING);


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (display_flag) {
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.println(hr_cnt);
    display.display();
    hr_cnt = 0;
    display_flag = false;
  }

}
