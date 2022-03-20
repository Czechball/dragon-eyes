#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define PCA_ADDR 0x70
#define LEFT_EYE_PORT 2
#define RIGHT_EYE_PORT 1

DNSServer dnsServer;
AsyncWebServer server(80);

const char *ssid = "Dragon's Eyes";
const char *password = "12345678";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Eye Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h3>ESP OLED Eyes Control Panel</h3>
  <br><br>
  <form action="/get">
    <input type="submit" name="open" value="Open">
    <input type="submit" name="close" value="Close">
    <input type="submit" name="idle" value="Idle">
  </form>
  <form action="/img">
  <select>
    <option value="silvron">Silvron</option>
    <option value="v2">v2.0</option>
    <option value="uzlabina">SPŠE Logo</option>
    <option value="hourglass">Hourglass</option>
    <option value="danger">Danger</option>
    <option value="frog">Frog</option>
    <option value="webnings">???</option>
  </select>
  <input type="submit" value="Submit" />
  </form>
</body></html>)rawliteral";

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS  0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      //request->addInterestingHeader("ANY");
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", index_html);
    }
};

// default iris values

int eyeIrisSizex = 32;
int eyeIrisSizey = 32;
int eyeIrisPosx = display.width() / 2;
int eyeIrisPosy = display.height() / 2;

// default eyesocket values

int eyeSocketSizex = 64;
int eyeSocketSizey = 64;
int eyeSocketPosx = display.width() / 2;
int eyeSocketPosy = display.height() / 2;

// default animation speeds

int lidSpeed = 8;
int irisSpeed = 0.5;

// default web requests statuses

enum { NONE_, OPEN_, CLOSE_, IDLE_, IMAGE_ } web;
unsigned char *img;

void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
    Serial.println("Client Connected");
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    web = NONE_;
    if (request->hasParam("open")) {
      web = OPEN_;
    }
    if (request->hasParam("close")) {
      web = CLOSE_;
    }
    if (request->hasParam("idle")) {
      web = IDLE_;
    }
    request->send(200, "text/html", "The eye command has been sent <br><a href=\"/\">Return to Home Page</a>");
  });
  server.on("/img", HTTP_GET, [] (AsyncWebServerRequest * request) {
    if (request->hasParam("silvron")) {
      web = IMAGE_;
      img = silvron;
    }
    if (request->hasParam("v2"))
    {
      web = IMAGE_;
      img = v2;
    }
    if (request->hasParam("uzlabina"))
    {
      web = IMAGE_;
      img = uzlabina;
    }
    if (request->hasParam("hourglass"))
    {
      web = IMAGE_;
      img = hourglass;
    }
    if (request->hasParam("danger"))
    {
      web = IMAGE_;
      img = danger;
    }
    if (request->hasParam("frog"))
    {
      web = IMAGE_;
      img = frog;
    }
    if (request->hasParam("webnings"))
    {
      web = IMAGE_;
      img = webnings;
    }
    request->send(200, "text/html", "The eye command has been sent <br><a href=\"/\">Return to Home Page</a>");
  });
}

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  server.begin();

  Serial.println("Initializing display...");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  muxPortEnable(PCA_ADDR, true, LEFT_EYE_PORT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Left SSD1306 allocation failed"));
  }

  muxPortEnable(PCA_ADDR, true, RIGHT_EYE_PORT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Left SSD1306 allocation failed"));
  }
  Wire.setClock(400000);

  eyesOpen(lidSpeed, eyeSocketSizey);

  Serial.println("All Done, starting eyes...");
  eyes();
}

void loop() {
  dnsServer.processNextRequest();
  switch (web) {
    case NONE_:
      eyes();
      break;
    case OPEN_:
      eyesOpen(lidSpeed, eyeSocketSizey);
      web = NONE_;
      break;
    case CLOSE_:
      eyesClose(lidSpeed, 1);
      web = NONE_;
      break;
    case IDLE_:
      eyesIdle();
      break;
    case IMAGE_:
      showImage(img);
      break;
    default:
      eyesIdle();
      break;
  }
  // delay(500);
  // dnsServer.processNextRequest();
  // eyesBlink(300);
  // dnsServer.processNextRequest();
  // delay(500);
  // dnsServer.processNextRequest();
  // eyesLookx(irisSpeed, 8);
  // dnsServer.processNextRequest();
  // delay(500);
  // dnsServer.processNextRequest();
  // eyesLookx(irisSpeed, -8);
  // dnsServer.processNextRequest();
  // delay(1000);
  // dnsServer.processNextRequest();
  // eyesClose(lidSpeed, 1);
  // dnsServer.processNextRequest();
  // delay(1000);
  // dnsServer.processNextRequest();
  // eyesOpen(lidSpeed, eyeSocketSizey);
}

void eyes(int eyeSocketPosx, int eyeSocketPosy, int eyeSocketSizex, int eyeSocketSizey, int eyeIrisPosx, int eyeIrisPosy, int eyeIrisSizex, int eyeIrisSizey) {
  newClearDisplay();
  drawSocket(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, eyeSocketSizey);
  drawIris(eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  newDisplayDisplay();

}

void eyes_dummy(int eyeSocketPosx, int eyeSocketPosy, int eyeSocketSizex, int eyeSocketSizey, int eyeIrisPosx, int eyeIrisPosy, int eyeIrisSizex, int eyeIrisSizey) {
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
  display.drawRoundRect(posx - sizex / 2, posy - sizey / 2, sizex, sizey, sizex * 2, SSD1306_WHITE);
}

void drawIris(int posx, int posy, int sizex, int sizey) {
  // set iris, position center + eyeDir
  display.fillRoundRect(posx - sizex / 2, posy - sizey / 2, sizex, sizey, sizex * 2, SSD1306_WHITE);
}

void eyesOpen(int increment, int finalSize) {
  newClearDisplay();
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, 1, eyeIrisPosx, eyeIrisPosy, 0, 0);
  newDisplayDisplay();
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
    newDisplayDisplay();
  }
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, finalSize, eyeIrisPosx, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  newDisplayDisplay();
}

void eyesClose(int increment, int finalSize) {
  newClearDisplay();
  eyes();
  newDisplayDisplay();
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
    newDisplayDisplay();
  }
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, finalSize, eyeIrisPosx, eyeIrisPosy, 0, 0);
  newDisplayDisplay();

}

void eyesBlink(int duration) {
  newClearDisplay();
  eyes(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, 4, eyeIrisPosx, eyeIrisPosy, 0, 0);
  newDisplayDisplay();
  delay(duration);
  eyes();
  newDisplayDisplay();

}

void eyesLookx(int increment, int distance) {
  muxPortEnable(PCA_ADDR, true, LEFT_EYE_PORT);
  display.clearDisplay();
  int finalSize = 0;
  finalSize = eyeIrisPosx + distance;
  eyes_dummy(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, eyeSocketSizey, finalSize, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  display.display();
  //delay(500);
  //eyes();
  
  muxPortEnable(PCA_ADDR, true, RIGHT_EYE_PORT);
  display.clearDisplay();
  finalSize = 0;
  finalSize = eyeIrisPosx - distance;
  eyes_dummy(eyeSocketPosx, eyeSocketPosy, eyeSocketSizex, eyeSocketSizey, finalSize, eyeIrisPosy, eyeIrisSizex, eyeIrisSizey);
  display.display();
  delay(500);
  //eyes();
}

void eyesIdle() {
  eyesBlink(300);
  delay(500);
  eyesLookx(irisSpeed, 8);
  delay(500);
  eyesLookx(irisSpeed, -8);
  delay(1000);
  eyesClose(lidSpeed, 1);
  delay(1000);
  eyesOpen(lidSpeed, eyeSocketSizey);
  delay(500);
}

void idleDelay(int delaynum) {
  if (web == IDLE_)
  {
    delay(delaynum);
  }
}

void newClearDisplay(){
  muxPortEnable(PCA_ADDR, true, LEFT_EYE_PORT);
  display.clearDisplay();
  muxPortEnable(PCA_ADDR, true, RIGHT_EYE_PORT);
  display.clearDisplay();
}

void newDisplayDisplay(){
  muxPortEnable(PCA_ADDR, true, LEFT_EYE_PORT);
  display.display();
  muxPortEnable(PCA_ADDR, true, RIGHT_EYE_PORT);
  display.display();
}

void showImage(unsigned char *name) {
  display.clearDisplay();
  display.drawBitmap(0, 0, name, 128, 64, 1);
  display.display();
}

bool muxPortEnable(int muxAddres, bool enable, uint8_t port)
{
  //Serial.println("muxPortEnable: entering function (address " + (String)muxAddres + ", enable " + (String)enable + ", port " + (String)port + ")");
  uint8_t muxRes = 0;
  uint32_t retbytes;

  delay (5);
  Wire.beginTransmission(muxAddres);
  if (enable == true)port += 8;
  if (port > 15)return true;

  Wire.write(port);
  Wire.endTransmission(true);

  //  if (Wire.lastError()) {
  //    Serial.println("muxPortEnable: I2C write error #" + (String)Wire.lastError());
  //    //resetI2C();
  //    return false;
  //  }

  retbytes = Wire.requestFrom(muxAddres,  1, 1);

  //  if (Wire.lastError()) {
  //    Serial.println("muxPortEnable: I2C read error #" + (String)Wire.lastError());
  //    //resetI2C();
  //    return false;
  //  }

  if (retbytes == 1) {
    muxRes = Wire.read();
    if (muxRes == port) {
      delay (5);
      return true;
    } else {
      //Serial.println("muxPortEnable: I2C slave returned (" + (String)muxRes + ") instead of (" + (String)port + ")");
      return false;
    }
  } else {
    //Serial.println("muxPortEnable: I2C slave did not return 1 byte");
    return false;
  }
}
