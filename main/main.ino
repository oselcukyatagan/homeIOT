#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProBlinds.h>
#include <SinricProLight.h>

#include "config.h"
#include "globals.h"

#include <esp_task_wdt.h>
#include <esp_system.h>

#include "ota.h"

// ================= GLOBAL DEFINITIONS =================
WiFiServer webServer(80);
bool isWebStarted = false;

bool isMusicMode = false;

String lastDetectionStatus = "No noise";

bool dirChangePending = false;
unsigned long dirChangeTime = 0;

int targetR = 255;
int targetG = 255;
int targetB = 255;
bool lightPower = false;
int currentBrightness = 100;   // 0–100 %

float smoothedVal = 2048;
float dcOffset = 2048;
int lastRawMic = 2048;

int blindPos = 0;
int blindTarget = 0;
int blindState = 2;
unsigned long blindLastTime = 0;
bool isPaused = false;
unsigned long pauseStart = 0;

int minutesRemaining = SILENCE_TIMEOUT_MIN;
unsigned long lastCheckMillis = 0;
bool isCurrentlyListening = false;
bool audioDetectedInWindow = false;

unsigned long lastWifiCheck = 0;
bool sinricConnected = false;

//telnet
WiFiServer telnetServer(23);
WiFiClient telnetClient;

// ================= FORWARD DECLARATIONS =================
void handleBlinds();
void handleMusic();

bool onRangeValue(const String &, int &);
bool onAdjustRangeValue(const String &, int &);
bool onPowerState(const String &, bool &);
bool onColor(const String &, byte &, byte &, byte &);
bool onBrightness(const String &, int &);

// ================= HELPER =================
bool onRangeValue(const String &deviceId, int &rangeValue) {
  blindTarget = constrain(rangeValue, 0, 100);
  Serial.printf("Blinds target: %d\r\n", blindTarget);
  return true; 
}

bool onAdjustRangeValue(const String &deviceId, int &rangeValueDelta) {
  blindTarget += rangeValueDelta;
  if(blindTarget > 100) blindTarget = 100;
  if(blindTarget < 0) blindTarget = 0;
  Serial.printf("Blinds target adjusted to: %d\r\n", blindTarget);
  return true;
}

bool onPowerState(const String &deviceId, bool &state) {
  isMusicMode = false;
  lightPower = state;

  if (lightPower) {
    int r = (targetR * currentBrightness) / 100;
    int g = (targetG * currentBrightness) / 100;
    int b = (targetB * currentBrightness) / 100;
    setLEDs(r, g, b);
  } else {
    setLEDs(0, 0, 0);
  }

  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  isMusicMode = false;
  lightPower = true;

  targetR = r;
  targetG = g;
  targetB = b;

  int scaledR = (targetR * currentBrightness) / 100;
  int scaledG = (targetG * currentBrightness) / 100;
  int scaledB = (targetB * currentBrightness) / 100;

  setLEDs(scaledR, scaledG, scaledB);

  return true;
}

bool onBrightness(const String &deviceId, int &brightness) {
  isMusicMode = false;

  currentBrightness = brightness;   // 0–100
  lightPower = true;

  int r = (targetR * currentBrightness) / 100;
  int g = (targetG * currentBrightness) / 100;
  int b = (targetB * currentBrightness) / 100;

  setLEDs(r, g, b);

  Serial.printf("Brightness: %d%%\r\n", currentBrightness);
  if (telnetClient && telnetClient.connected()) {
    telnetClient.printf("Brightness: %d%%\r\n", currentBrightness);
  }

  return true;
}

void handleAutoOff() {
  if (isMusicMode || !lightPower) {
    minutesRemaining = SILENCE_TIMEOUT_MIN;
    isCurrentlyListening = false;
    lastDetectionStatus = "Waiting...";
    return;
  }

  unsigned long currentMillis = millis();

  if (!isCurrentlyListening && (currentMillis - lastCheckMillis >= CHECK_INTERVAL_MS)) {
    isCurrentlyListening = true;
    audioDetectedInWindow = false;
    lastDetectionStatus = "No noise";
    lastCheckMillis = currentMillis;
  }

  if (isCurrentlyListening) {
    int raw = analogRead(PIN_MIC);

    // 1. Dynamic DC Offset Tracking
    dcOffset = (dcOffset * 0.99) + (raw * 0.01);
    
    // 2. Absolute Amplitude
    float currentAmplitude = abs(raw - dcOffset);

    // 3. Envelope Follower (Fast Attack, Slow Decay)
    static float envelope = 0;
    if (currentAmplitude > envelope) {
      envelope = (envelope * 0.2) + (currentAmplitude * 0.8); 
    } else {
      envelope = (envelope * 0.99) + (currentAmplitude * 0.01); 
    }

    // 4. Threshold Evaluation
    if (envelope > AUTO_OFF_THRESHOLD) {
      audioDetectedInWindow = true;
      lastDetectionStatus = "Sound detected"; 
    }

    if (currentMillis - lastCheckMillis >= LISTEN_DURATION_MS) {
      isCurrentlyListening = false;
      envelope = 0; // Reset envelope for next minute
      
      if (audioDetectedInWindow) {
        minutesRemaining = SILENCE_TIMEOUT_MIN;
      } else {
        if (minutesRemaining > 0) minutesRemaining--;
      }

      if (minutesRemaining <= 0) {
        lightPower = false;
        setLEDs(0, 0, 0);
        syncLightPower(false);
      }
      
      lastCheckMillis = currentMillis; 
    }
  }
}

void handleTelnet() {

  if (telnetServer.hasClient()) {

    if (!telnetClient || !telnetClient.connected()) {
      telnetClient = telnetServer.available();
      telnetClient.printf("Telnet debug connected.\r\n");
      telnetClient.printf("System ready.\r\n");
    } else {
      WiFiClient newClient = telnetServer.available();
      newClient.stop();
    }
  }

  if (telnetClient && telnetClient.connected() && telnetClient.available()) {

    char buffer[128];

    int len = telnetClient.readBytesUntil('\n', buffer, sizeof(buffer)-1);
    buffer[len] = '\0';

    Serial.print("Received via Telnet: ");
    Serial.println(buffer);
  }
}

void handleWiFiReconnect() {

  if (WiFi.status() == WL_CONNECTED) {

    if (!sinricConnected) {
      Serial.println("WiFi restored. Restarting SinricPro");

      SinricPro.stop();
      delay(100);
      SinricPro.begin(APP_KEY, APP_SECRET);

      sinricConnected = true;
    }

    return;
  }

  sinricConnected = false;

  unsigned long now = millis();

  if (now - lastWifiCheck < WIFI_RECONNECT_INTERVAL) return;

  lastWifiCheck = now;

  Serial.println("WiFi lost. Attempting reconnect...");

  if (telnetClient && telnetClient.connected()) {
    telnetClient.println("WiFi lost. Attempting reconnect...");
  }

  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void printResetReason() {

  esp_reset_reason_t reason = esp_reset_reason();

  Serial.print("Reset reason: ");

  switch(reason) {

    case ESP_RST_POWERON:
      Serial.println("Power On");
      break;

    case ESP_RST_SW:
      Serial.println("Software Reset");
      break;

    case ESP_RST_PANIC:
      Serial.println("Kernel Panic");
      break;

    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
      Serial.println("Watchdog Reset");
      break;

    case ESP_RST_BROWNOUT:
      Serial.println("Brownout");
      break;

    default:
      Serial.println("Other");
  }
}

void setup() {
  
  Serial.begin(115200);

  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT * 1000,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };

  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);

  printResetReason();

  pinMode(PIN_MIC, INPUT);

  ledcAttach(PIN_RED, 5000, 8);
  ledcAttach(PIN_GREEN, 5000, 8);
  ledcAttach(PIN_BLUE, 5000, 8);

  pinMode(RELAY_DIR, OUTPUT);
  pinMode(RELAY_PWR, OUTPUT);

  digitalWrite(RELAY_PWR, LOW);
  digitalWrite(RELAY_DIR, LOW);

  setLEDs(0, 0, 0);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setSleep(false); 

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED &&
      millis() - startAttempt < 15000) {
  delay(500);
  yield();
}

  if (WiFi.status() != WL_CONNECTED) {

  Serial.println("\nWiFi initial connection failed. Continuing...");

} else {

  Serial.println("\r\nWiFi Connected.");
  Serial.print("Web Server URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":80");

}

  webServer.begin();
  isWebStarted = true;

  SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
  myBlinds.onRangeValue(onRangeValue);
  myBlinds.onAdjustRangeValue(onAdjustRangeValue);

  SinricProLight &myLight = SinricPro[LIGHT_ID];
  myLight.onPowerState(onPowerState);
  myLight.onColor(onColor);
  myLight.onBrightness(onBrightness);

  SinricPro.onConnected([](){ 
    Serial.println("Connected to SinricPro"); 
    if (telnetClient && telnetClient.connected()) {
    telnetClient.printf("Connected to SinricPro\r\n"); 
  }
  });

  SinricPro.onDisconnected([](){
     Serial.println("Disconnected from SinricPro"); 
    if (telnetClient && telnetClient.connected()) {
    telnetClient.printf("Disconnected from SinricPro\r\n");
  }
    });

  SinricPro.begin(APP_KEY, APP_SECRET);

  setupOTA();

  telnetServer.begin();
  telnetServer.setNoDelay(true);

}

void loop() {

  handleOTA();

  SinricPro.handle();

  handleBlinds();
  handleMusic();
  handleAutoOff();
  checkWebClient();

  handleTelnet();

  handleWiFiReconnect();

  esp_task_wdt_reset();

  static unsigned long lastHeapPrint = 0;

if (millis() - lastHeapPrint > 60000) {
    lastHeapPrint = millis();

    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

    if (telnetClient && telnetClient.connected()) {
        telnetClient.printf("Free heap: %d\n", ESP.getFreeHeap());
    }
}

}


void syncBlindPosition(int pos) {
  SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
  myBlinds.sendRangeValueEvent(pos);
}

void syncLightPower(bool state) {
  SinricProLight &myLight = SinricPro[LIGHT_ID];
  myLight.sendPowerStateEvent(state);
}

void syncLightColor(int r, int g, int b) {
  SinricProLight &myLight = SinricPro[LIGHT_ID];
  myLight.sendColorEvent(r, g, b);
}