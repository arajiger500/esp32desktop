#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>
#include <time.h>

// Wi-Fi Credentials
const char* ssid = "TP-Link_C895_plus";
const char* password = "93962841";

// Spotify API / Web-hook configuration
const char* spotifyStatusUrl = "http://192.168.1.114:8888/spotify"; 

// ============================================================================
// OLED DISPLAY CONFIGURATION
// ============================================================================
#define OLED_CS   5
#define OLED_DC   2
#define OLED_RST  4
#define OLED_MOSI 23
#define OLED_CLK  18

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RST);

// Variables to store track info
String trackTitle = "Loading...";
String trackArtist = "";
String trackAlbum = "";
long progressMs = 0;
long durationMs = 0;

void setup() {
  Serial.begin(115200);
  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);
  u8g2.begin();
  u8g2.setBusClock(400000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  // Sync time
  configTime(0, 0, "pool.ntp.org");
}

void fetchSpotifyData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(spotifyStatusUrl);
    http.setTimeout(2000);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      // Parse pipe-delimited string: Title|Artist|Album|Progress|Duration
      int p1 = payload.indexOf('|');
      int p2 = payload.indexOf('|', p1 + 1);
      int p3 = payload.indexOf('|', p2 + 1);
      int p4 = payload.indexOf('|', p3 + 1);
      
      if (p1 != -1) {
        trackTitle = payload.substring(0, p1);
        trackArtist = payload.substring(p1 + 1, p2);
        trackAlbum = payload.substring(p2 + 1, p3);
        progressMs = payload.substring(p3 + 1, p4).toInt();
        durationMs = payload.substring(p4 + 1).toInt();
      }
    }
    http.end();
  }
}

void updateDisplay() {
  u8g2.clearBuffer();

  // 1. Time
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[10];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(100, 10, timeStr);
  }

  // 2. Track Info
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Now Playing");
  u8g2.setFont(u8g2_font_7x14B_tf);
  u8g2.drawStr(0, 28, trackTitle.substring(0, 18).c_str());
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 40, trackArtist.substring(0, 25).c_str());
  u8g2.drawStr(0, 50, trackAlbum.substring(0, 25).c_str());

  // 3. Progress Bar
  if (durationMs > 0) {
    int barWidth = (int)((float)progressMs / durationMs * 128);
    u8g2.drawFrame(0, 55, 128, 3);
    u8g2.drawBox(0, 55, barWidth, 3);
  }

  // 4. Synthesizer Visualizer
  for (int i = 0; i < 16; i++) {
    int h = random(2, 8);
    u8g2.drawBox(i * 8, 64 - h, 6, h);
  }

  u8g2.sendBuffer();
}

void loop() {
  fetchSpotifyData();
  updateDisplay();
  delay(1000); // Refresh every second for clock/progress
}
