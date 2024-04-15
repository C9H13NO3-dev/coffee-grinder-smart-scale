#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "display.hpp"
#include "scale.hpp"

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid     = "ThisIsNotTheWifiYoureLookingFor"; // Change this to your WiFi SSID
const char* password = "PASSWORD"; // Change this to your WiFi password
long lastReconnectAttempt = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial){ delay(100); }

  setupDisplay();
  setupScale();

  Serial.println();
  Serial.println("******************************************************");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize OTA Handlers
  ArduinoOTA.setHostname("coffee-scale-ota"); // Set the hostname for the OTA device
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready for OTA updates");

  lastReconnectAttempt = 0;
  client.setServer("192.168.1.201", 1883);
}

boolean reconnect() {
  if (client.connect("coffee-scale")) {
    // Once connected, publish an announcement...
    Serial.println("Connected to MQTT");
  }
  return client.connected();
}

void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  
  ArduinoOTA.handle();  // Handle OTA updates
  delay(1000);
}
