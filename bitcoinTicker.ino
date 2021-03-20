#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "INSERT WIFI NAME(SSID)";
const char* password = "<INSERT WIFI PASSWORD>";
const String API_KEY = "<INSERT API KEY FROM COINMARKETCAP HERE>";

const char* host = "https://pro-api.coinmarketcap.com";
const int httpsPort = 443;

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

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  display.println("Connected to: ");
  display.print(ssid);
  display.display();
  delay(1500);
  display.clearDisplay();
  display.display();

}

void loop() {
  Serial.print("Connecting to ");
  Serial.println(host);

  String url = "https://pro-api.coinmarketcap.com/v2/cryptocurrency/quotes/latest?CMC_PRO_API_KEY=" + API_KEY + "&symbol=BTC&convert=USD";
  Serial.print("requesting URL: ");
  Serial.println(url);
  http.begin(url);
  int httpCode = http.GET();
  StaticJsonDocument<2500> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error) {
    Serial.print(F("deserializeJson Failed"));
    Serial.println(error.f_str());
    delay(25000);
    return;
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  String BTCUSDPrice = doc["data"]["BTC"][0]["quote"]["USD"]["price"].as<String>();
  String lastUpdated = doc["status"]["timestamp"].as<String>();
  double last24HourPercentChange = doc["data"]["BTC"][0]["quote"]["USD"]["percent_change_24h"].as<double>();
  bool isUp = last24HourPercentChange > 0;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("BTC/USD: $" + BTCUSDPrice);
  display.setCursor(0, 10);
  if (isUp) {
    display.println("BTC is UP today!");
  } else {
    display.println("BTC is down today.");
  }

  display.setCursor(0, 20);
  display.print("(");
  display.print(last24HourPercentChange);
  display.print("%)");
  display.setCursor(0, 40);
  display.println("Last Updated: ");
  display.print(formatDatetime(doc["status"]["timestamp"].as<String>())); //Date input looks like this   "2018-06-02T22:51:28.209Z"
  display.display();

  Serial.println(doc["data"]["BTC"][0]["quote"]["USD"]["price"].as<String>());

  http.end();
  delay(300000);
}

String formatDatetime(String date) {
  //Date input looks like this   "2018-06-02T22:51:28.209Z"
  int tIndex = date.indexOf('T');
  String unformattedDate = date.substring(0, tIndex); // 2018-06-02
  String unformattedTime = date.substring(tIndex + 1, date.indexOf('.')); //22:51:28
  Serial.print("unformatted date");
  Serial.print(unformattedDate);

  String day = unformattedDate.substring(unformattedDate.lastIndexOf('-') + 1);
  String month = unformattedDate.substring(unformattedDate.indexOf('-') + 1, unformattedDate.lastIndexOf('-'));
  String year = unformattedDate.substring(0, unformattedDate.indexOf('-'));
  String formattedDate = month + "/" + day + "/" + year;

  return formattedDate + " @ " + unformattedTime;
}
