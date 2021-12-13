#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH	128
#define SCREEN_HEIGHT	64
#define OLED_RESET		-1
#define SCREEN_ADDRESS	0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int eyeDir;
String eyeValue = "15";

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
}

void loop() {
  eyes();
}

void eyes() {
  display.clearDisplay();
  display.drawCircle(display.width()/2, display.height()/2, 30, SSD1306_WHITE);
  display.fillCircle(display.width()/2+eyeDir, display.height()/2, eyeValue.toInt(), SSD1306_WHITE);
  display.display();
  if (Serial.available() > 0) {
    // read the incoming byte:
    eyeValue = Serial.readString();
    // say what you got:
    Serial.print("Eye value set to ");
    Serial.println(eyeValue);
  }
}