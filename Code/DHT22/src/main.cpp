#include <Arduino.h>
#include "DHT.h"
#include <WiFi.h>
#include "time.h"

// WiFi credentials
const char* ssid     = "iPhone";
const char* password = "jwelcomee";

// NTP server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // EDT (UTC-4)
const int   daylightOffset_sec = 3600; // Daylight savings

// DHT settings
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Sensor ID
#define SENSOR_ID "DHT22_01"

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi.");

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Starting DHT sensor...");
}

String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "N/A";
  }
  char buffer[10];
  strftime(buffer, sizeof(buffer), "%I:%M %p", &timeinfo); // 02:08 PM
  return String(buffer);
}

void loop() {
  // Read temperature and humidity
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String timestamp = getCurrentTime();

  // Check if readings failed
  if (isnan(h) || isnan(t)) {
    Serial.print("{\"sensor_id\":\"");
    Serial.print(SENSOR_ID);
    Serial.print("\", \"timestamp\":\"");
    Serial.print(timestamp);
    Serial.println("\", \"error\":\"Failed to read from DHT sensor\"}");
  } else {
    // Print JSON
    Serial.print("{\"sensor_id\":\"");
    Serial.print(SENSOR_ID);
    Serial.print("\", \"timestamp\":\"");
    Serial.print(timestamp);
    Serial.print("\", \"temperature\":");
    Serial.print(t, 2);
    Serial.print(", \"humidity\":");
    Serial.print(h, 2);
    Serial.println("}");
  }

  delay(5000);
}
