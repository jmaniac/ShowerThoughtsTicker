/**************************************************************************
  TTGO T-Display ST7789 OLED based on Adafruit example
  https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/examples/graphicstest/graphicstest.ino
 **************************************************************************/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// pinouts from https://github.com/Xinyuan-LilyGO/TTGO-T-Display
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_BL 4

const char* ssid = "";
const char* password = "Jericho71101";
const uint8_t upButtonPin = 35;
const uint8_t downButtonPin = 0;
volatile int32_t counter = 0;
volatile bool pressed = false;

String lastAfterId = "";
String url = "https://www.reddit.com/r/Showerthoughts/new.json?limit=1&after=";
HTTPClient http;

// constructor for data object named tft 
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void ARDUINO_ISR_ATTR upButtonPinInterrupt() {
  if (counter < 9) counter++;
  pressed = true;
}
void ARDUINO_ISR_ATTR downButtonPinInterrupt() {
  if (counter > 0) counter--;
  pressed = true;
}

void setup(void) {
  Serial.begin(115200);

  // Button ISR
  pinMode(upButtonPin, INPUT_PULLUP);
  attachInterrupt(upButtonPin, upButtonPinInterrupt, RISING);
  pinMode(downButtonPin, INPUT_PULLUP);
  attachInterrupt(downButtonPin, downButtonPinInterrupt, RISING);

  pinMode(TFT_BL, OUTPUT);      // TTGO T-Display enable Backlight pin 4
  digitalWrite(TFT_BL, HIGH);   // T-Display turn on Backlight
  tft.init(135, 240);           // Initialize ST7789 240x135
  Serial.println(F("Display Initialized"));
  tft.fillScreen(ST77XX_BLACK);

  tft.println("Connecting to Wifi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  Serial.print("CONNECTED to SSID: ");
  Serial.println(ssid);

  tft.print("Connected to ");
  tft.println(ssid);
  delay(5000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && pressed) {
      Serial.print(counter);
      String showerThought = getShowerThought();
      if (showerThought.length() > 0) {
        drawText(showerThought, ST77XX_WHITE);
      }
      else{
        drawText("No wisdom for you", ST77XX_RED);
      }
    }
}

String getShowerThought() {
  Serial.println("Getting current data...");
  http.begin(url+lastAfterId);
  int httpCode = http.GET();
  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    JsonDocument filter;
    filter["data"]["children"][0]["data"]["title"] = true;
    filter["data"]["after"] = true;

    JsonDocument doc;
    String payload = http.getString();  // capture once
    http.end();

    DeserializationError error = deserializeJson(doc, payload,
                                   DeserializationOption::Filter(filter));
    if (error) {
      Serial.print(F("deserializeJson failed: "));
      Serial.println(error.f_str());
      return "Parse Error";
    }

    String showerThought = doc["data"]["children"][0]["data"]["title"];
    lastAfterId = (String) doc["data"]["after"];
    Serial.println("Got: " + showerThought);
    Serial.println("lastAfter : " + lastAfterId);
    return showerThought;
  }

  http.end();
  return "HTTP Error";
}

void drawText(String text, uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
  pressed = false;
}