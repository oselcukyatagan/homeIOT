#include <ArduinoOTA.h>
#include <WiFi.h>
#include "ota.h"
#include "globals.h"
#include "config.h"
#include <esp_task_wdt.h>

void setupOTA() {

  ArduinoOTA.setHostname("HomeIOT-ESP32");
  ArduinoOTA.setPassword(OTA_PASSWORD); 

  ArduinoOTA.onStart([]() {

    digitalWrite(RELAY_PWR, LOW);
    digitalWrite(RELAY_DIR, LOW);
    setLEDs(0, 0, 0);
    esp_task_wdt_delete(NULL);

    Serial.println("OTA Start - Hardware Safely Locked");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End. Rebooting...");
    esp_task_wdt_add(NULL);
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