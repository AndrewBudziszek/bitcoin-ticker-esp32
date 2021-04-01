#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "secrets.h" // WiFi Configuration (WiFi name and Password)
#include <ArduinoJson.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = SECRET_SSID;
const char* password = SECRET_WIFI_PASSWORD;

const int httpsPort = 443;
// Powered by CoinDesk - https://www.coindesk.com/price/bitcoin
const String url = "https://api.coindesk.com/v1/bpi/currentprice/BTC.json";
const String historyURL = "https://api.coindesk.com/v1/bpi/historical/close.json";
const String cryptoCode = "BTC";

WiFiClient client;
HTTPClient http;

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  display.println("Connected to: ");
  display.print(ssid);
  display.display();
  delay(1500);
  display.clearDisplay();
  display.display();

}

void loop() {
  Serial.print("Connecting to ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error) {
    Serial.print(F("deserializeJson Failed"));
    Serial.println(error.f_str());
    delay(2500);
    return;
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  String BTCUSDPrice = doc["bpi"]["USD"]["rate_float"].as<String>();
  String lastUpdated = doc["time"]["updated"].as<String>();
  http.end();

  Serial.print("Getting history...");
  StaticJsonDocument<2000> historyDoc;
  http.begin(historyURL);
  int historyHttpCode = http.GET();
  DeserializationError historyError = deserializeJson(historyDoc, http.getString());

  if (historyError) {
    Serial.print(F("deserializeJson(History) failed"));
    Serial.println(historyError.f_str());
    delay(2500);
    return;
  }

  Serial.print("History HTTP Status Code: ");
  Serial.println(historyHttpCode);
  JsonObject bpi = historyDoc["bpi"].as<JsonObject>();
  double yesterdayPrice;
  for (JsonPair kv : bpi) {
    yesterdayPrice = kv.value().as<double>();
  }

  Serial.print("BTCUSD Price: ");
  Serial.println(BTCUSDPrice.toDouble());

  Serial.print("Yesterday's Price: ");
  Serial.println(yesterdayPrice);

  bool isUp = BTCUSDPrice.toDouble() > yesterdayPrice;
  double percentChange;
  if (isUp) {
    percentChange = ((BTCUSDPrice.toDouble() - yesterdayPrice) / yesterdayPrice) * 100;
  } else {
    percentChange = ((yesterdayPrice - BTCUSDPrice.toDouble()) / yesterdayPrice) * 100;
  }

  Serial.print("Percent Change: ");
  Serial.println(percentChange);

  //Display Header
  display.clearDisplay();
  display.setTextSize(1);
  printCenter("BTC/USD", 0, 0);

  //Display BTC Price
  display.setTextSize(2);
  printCenter("$" + BTCUSDPrice, 0, 25);

  //Display 24hr. Percent Change
  String dayChangeString = "24hr. Change: ";
  if (isUp) {
    percentChange = ((BTCUSDPrice.toDouble() - yesterdayPrice) / yesterdayPrice) * 100;
  } else {
    percentChange = ((yesterdayPrice - BTCUSDPrice.toDouble()) / yesterdayPrice) * 100;
    dayChangeString = dayChangeString + "-";
  }
  display.setTextSize(1);
  dayChangeString = dayChangeString + percentChange + "%";
  printCenter(dayChangeString, 0, 55);
  display.display();

  http.end();
  delay(10000);
}

void printCenter(const String buf, int x, int y)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
  display.setCursor((x - w / 2) + (128 / 2), y);
  display.print(buf);
}
