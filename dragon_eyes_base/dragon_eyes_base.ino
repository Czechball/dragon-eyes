#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

#define SCREEN_WIDTH	128
#define SCREEN_HEIGHT	64
#define OLED_RESET		-1
#define SCREEN_ADDRESS	0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// default iris values

int eyeIrisSizex = 32;
int eyeIrisSizey = 32;
int eyeIrisPosx = display.width()/2;
int eyeIrisPosy = display.height()/2;

// default eyesocket values

int eyeSocketSizex = 64;
int eyeSocketSizey = 64;
int eyeSocketPosx = display.width()/2;
int eyeSocketPosy = display.height()/2;

// default animation speeds

int lidSpeed = 8;
int irisSpeed = 0.5;

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  eyesOpen(lidSpeed, eyeSocketSizey);

  eyes();
}

void loop() {
  delay(500);
  eyesBlink(300);
  delay(500);
  eyesLookx(irisSpeed, 8);
  delay(500);
  eyesLookx(irisSpeed, -8);
  delay(1000);
  eyesClose(lidSpeed, 1);
  delay(1000);
  eyesOpen(lidSpeed, eyeSocketSizey);
}

void eyes(int eyeSocketPosx, int eyeSocketPosy, int eyeSocketSizex, int eyeSocketSizey, int eyeIrisPosx, int eyeIrisPosy, int eyeIrisSizex, int eyeIrisSizey) {
  display.clearDisplay();
  drawSocket(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, eyeSocketSizey);
  drawIris(eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  display.display();
}

void eyes()
{
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, eyeSocketSizey, eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
}

void drawSocket(int posx, int posy, int sizex, int sizey) {
  // set eyesocket, position center
  display.drawRoundRect(posx-sizex/2, posy-sizey/2, sizex, sizey, sizex*2, SSD1306_WHITE);
}

void drawIris(int posx, int posy, int sizex, int sizey) {
  // set iris, position center + eyeDir
  display.fillRoundRect(posx-sizex/2, posy-sizey/2, sizex, sizey, sizex*2, SSD1306_WHITE);
}

void eyesOpen(int increment, int finalSize) {
  display.clearDisplay();
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, 1, eyeIrisPosx, eyeIrisPosy, 0, 0);
  display.display();
  delay(500);
  for (int i = 1; i < finalSize; i = i + increment)
    {
      if (i <= eyeIrisSizey)
    {
    eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, i, eyeIrisPosx, eyeIrisPosy, 0, 0);
  }
  else
    {
      eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, i, eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
    }
  display.display();
  }
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, finalSize, eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  display.display();
}

void eyesClose(int increment, int finalSize) {
  display.clearDisplay();
  eyes();
  display.display();
  delay(500);
  for (int i = eyeSocketSizey; i > 1; i = i - increment)
    {
      if (i >= eyeIrisSizey)
    {
    eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, i, eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  }
  else
    {
      eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, i, eyeIrisPosx, eyeIrisPosy, 0, 0);
    }
  display.display();
  }
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, finalSize, eyeIrisPosx, eyeIrisPosy, 0, 0);
  display.display();
}

void eyesBlink(int duration) {
  display.clearDisplay();
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, 4, eyeIrisPosx, eyeIrisPosy, 0, 0);
  display.display();
  delay(duration);
  eyes();
  display.display();
}

void eyesLookx(int increment, int distance) {
  display.clearDisplay();
  int finalSize = 0;
  finalSize = eyeIrisPosx + distance;
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, eyeSocketSizey, finalSize, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  display.display();
  delay(500);
  eyes();
}
