#include <ArduinoOTA.h>
#include <WiFi.h>
#include "ota.h"
#include "globals.h"
#include "config.h"

void setupOTA() {

  ArduinoOTA.setHostname("HomeIOT-ESP32");

  ArduinoOTA.onStart([]() {

    digitalWrite(RELAY_PWR, LOW);
    digitalWrite(RELAY_DIR, LOW);
    setLEDs(0, 0, 0);

    Serial.println("OTA Start - Hardware Safely Locked");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End. Rebooting...");
    delay(100);
    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]\n", error);
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void handleOTA() {
  ArduinoOTA.handle();
}