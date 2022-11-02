#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// if you make 12hours version, change true to false
#define VERSION_24H true

#define WIFI_SMARTCONFIG false

#if !WIFI_SMARTCONFIG
// if you do not use smartConfifg, please specify SSID and password here
#define WIFI_SSID "SSID" // your WiFi's SSID
#define WIFI_PASS "password" // your WiFi's password
#endif

// LED color setting
#define LED_R 0
#define LED_G 125
#define LED_B 0

// starting LED number of each digit
#define MIN1 0
#define MIN10 7
#define COLON 14
#define HOUR1 15
#define HOUR10A 22
#define HOUR10B 24
#define HOUR10 22

#if VERSION_24H
#define TOTAL_LED 29
#else
#define TOTAL_LED 25
#endif
#define LED_CONTROL 2
Adafruit_NeoPixel pixels(TOTAL_LED, LED_CONTROL, NEO_GRB + NEO_KHZ800);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  Serial.begin(115200);
  Serial.println("start");

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  Serial.println("WiFi Connected.");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  pixels.begin();
#if VERSION_24H
  pixels.setPixelColor(HOUR10 + 3, pixels.Color(LED_R, LED_G, LED_B));
#endif
  pixels.setPixelColor(HOUR1 + 3, pixels.Color(LED_R, LED_G, LED_B));
  pixels.setPixelColor(MIN10 + 3, pixels.Color(LED_R, LED_G, LED_B));
  pixels.setPixelColor(MIN1  + 3, pixels.Color(LED_R, LED_G, LED_B));
  pixels.show();

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);

}

void loop() {
  static int prevMin = -1;
  static int prevCol = 0;
  static int prevDay = -1;
  int col;
  int hour;
  struct tm tmtime;

  timeClient.update(); 
     {  
    int currentHour = timeClient.getHours();  
    int currentMinute = timeClient.getMinutes();
    delay(400);
     }
    
  col = (millis() / 500) % 2;
  if(prevCol == col)
    return;
  prevCol = col;

  pixels.clear();
  if(col) {
    pixels.setPixelColor(COLON, pixels.Color(LED_R, LED_G, LED_B));
  
    }
  prevDay = tmtime.tm_mday;

#if VERSION_24H
  hour = timeClient.getHours();

  setDigit(HOUR10, hour / 10);
#else
  hour = currentHour % 12;
  if(hour == 0) hour = 12;

  if(hour >= 10) {
    pixels.setPixelColor(HOUR10A, pixels.Color(LED_R, LED_G, LED_B));
    pixels.setPixelColor(HOUR10B, pixels.Color(LED_R, LED_G, LED_B));
  }
#endif

  setDigit(HOUR1, hour % 10);
  setDigit(MIN10, timeClient.getMinutes() / 10);
  setDigit(MIN1 , timeClient.getMinutes() % 10);

  pixels.show();
  delay(400);
}

#define NUM 10
#define SEG 7
int seg[NUM][SEG] = {
  {1, 1, 1, 0, 1, 1, 1},
  {0, 1, 0, 0, 1, 0, 0},
  {1, 1, 0, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 0, 1},
  {0, 1, 1, 1, 1, 0, 0},
  {1, 0, 1, 1, 1, 0, 1},
  {1, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 1, 0, 0},
  {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 0, 1}
};

void setDigit(int pos, int n) {
  for(int j = 0; j < SEG; j++) {
    if(seg[n][j]) {
      pixels.setPixelColor(j + pos, pixels.Color(LED_R, LED_G, LED_B));
    }        
  }  
}

//////////////////////////////
// WiFi and NTP section
//////////////////////////////

void wifiSetup() {
  WiFi.mode(WIFI_STA);
#if WIFI_SMARTCONFIG
  WiFi.begin();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif

  for (int i = 0; ; i++) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
#if WIFI_SMARTCONFIG
  if(i > 30)
    break;
#endif    
  }

#if WIFI_SMARTCONFIG
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      Serial.print(".");
      pixels.clear();
      setDigit(MIN10, 5); //s
      pixels.setPixelColor(MIN1, pixels.Color(LED_R, LED_G, LED_B));
      pixels.setPixelColor(MIN1+2, pixels.Color(LED_R, LED_G, LED_B));
      pixels.setPixelColor(MIN1+5, pixels.Color(LED_R, LED_G, LED_B));
      pixels.setPixelColor(MIN1+6, pixels.Color(LED_R, LED_G, LED_B));
      pixels.show();
      delay(1000);
    }

    Serial.println("");
    Serial.println("SmartConfig received.");

    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(",");
    }
  }
  Serial.println("WiFi Connected.");
#endif

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}
