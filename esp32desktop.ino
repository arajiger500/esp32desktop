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
const char* spotifyStatusUrl = "http://your-local-server-or-api/spotify"; 

// ============================================================================
// OLED DISPLAY CONFIGURATION
// Configured for SH1106 4-Wire Software SPI (More reliable pin configuration)
// ============================================================================

// OPTION 1: 4-Wire Software SPI SH1106 (1.3" OLED) - ACTIVE
// This allows you to define every pin explicitly. Change the pin numbers below to match your wiring!
// Pins: clock=18 (D0/SCK), data=23 (D1/MOSI), cs=5, dc=16, reset=17
U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 16, /* reset=*/ 17);

// OPTION 2: 4-Wire Hardware SPI SH1106 (1.3" OLED)
// U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 16, /* reset=*/ 17);

// OPTION 3: I2C SSD1306 (0.96" OLED with 4 pins: VCC, GND, SCL, SDA)
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);
// ============================================================================

// Variables to store track info
String trackTitle = "Loading...";
String trackArtist = "Waiting for Wi-Fi";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting ESP32 Spotify Desktop Display...");

  // Initialize OLED display
  Serial.println("Initializing OLED display (SH1106 4-Wire Software SPI)...");
  if (u8g2.begin()) {
    Serial.println("OLED initialized successfully!");
  } else {
    Serial.println("OLED initialization failed! Check connections/constructor.");
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 15, "Connecting to Wi-Fi...");
  u8g2.sendBuffer();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "Connected!");
  u8g2.sendBuffer();
  delay(1000);
}

void fetchSpotifyData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(spotifyStatusUrl);
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
  u8g2.drawStr(0, 10, "Spotify Now Playing");
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
