#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SPI.h>

// Wi-Fi Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Spotify API / Web-hook configuration
// This points to the computer running your Python script on port 5000
const char* spotifyStatusUrl = "http://192.168.1.114:5000/spotify"; 

// ============================================================================
// OLED DISPLAY CONFIGURATION (Working Pins & Constructor)
// ============================================================================
#define OLED_CS   5
#define OLED_DC   2
#define OLED_RST  4
#define OLED_MOSI 23
#define OLED_CLK  18

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RST);
// ============================================================================

// Variables to store track info
String trackTitle = "Fetching tunes...";
String trackArtist = "Looking for signal...";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting ESP32 Spotify Desktop Display...");

  // Initialize SPI with the correct pins
  SPI.begin(OLED_CLK, -1, OLED_MOSI, OLED_CS);

  // Initialize OLED display
  Serial.println("Initializing OLED display...");
  u8g2.begin();
  u8g2.setBusClock(400000);
  delay(150); // Let power/reset settle

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 15, "Getting online...");
  u8g2.sendBuffer();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "We're in!");
  u8g2.sendBuffer();
  delay(1000);
}

void fetchSpotifyData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(spotifyStatusUrl);
    http.setTimeout(2000); // Added timeout to prevent hanging
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Received payload: " + payload);
      
      // Simple parsing placeholder (assuming plain text "Title - Artist" or simple JSON)
      int separator = payload.indexOf('-');
      if (separator != -1) {
        trackTitle = payload.substring(0, separator);
        trackTitle.trim();
        trackArtist = payload.substring(separator + 1);
        trackArtist.trim();
      } else {
        trackTitle = payload;
        trackArtist = "";
      }
    } else {
      Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
      trackTitle = "Offline / Error";
      trackArtist = "";
    }
    http.end();
  }
}

void updateDisplay() {
  u8g2.clearBuffer();

  // Draw Header
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "Jamming to...");
  u8g2.drawHLine(0, 14, 128);

  // Draw Track Title
  u8g2.setFont(u8g2_font_7x14B_tf);
  u8g2.drawStr(0, 34, trackTitle.substring(0, 18).c_str()); // Truncate to fit screen

  // Draw Artist Name
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 52, trackArtist.substring(0, 21).c_str());

  u8g2.sendBuffer();
}

void loop() {
  fetchSpotifyData();
  updateDisplay();
  delay(5000); // Refresh every 5 seconds
}
