#include <Arduino.h>
#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

// WiFi credentials
const char* ssid     = "iPhone";
const char* password = "jwelcomee";

// MQTT Broker settings
const char* mqtt_server = "172.20.10.3"; // e.g., 192.168.1.100
const int mqtt_port = 1883;
const char* mqtt_topic = "DHT22";

// DHT settings
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Sensor ID
#define SENSOR_ID "DHT22_01"

// NTP settings
const long gmtOffset_sec = -18000; // EST base
const int daylightOffset_sec = 3600; // EDT adjustment

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "N/A";
  char buffer[9];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  // NTP
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");

  Serial.print("Waiting for NTP time sync");
  while (getCurrentTime() == "N/A") {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime synced!");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  String timestamp = getCurrentTime();

  String payload;

  if (isnan(h) || isnan(t)) {
    payload = "{\"sensor_id\":\"" + String(SENSOR_ID) + "\",\"timestamp\":\"" + timestamp + "\",\"error\":\"Failed to read from DHT sensor\"}";
  } else {
    payload = "{\"sensor_id\":\"" + String(SENSOR_ID) + "\",\"timestamp\":\"" + timestamp + "\",\"temperature\":" + String(t, 2) + ",\"humidity\":" + String(h, 2) + "}";
  }

  client.publish(mqtt_topic, payload.c_str());
  Serial.println("Published: " + payload);

  delay(10000);
}
