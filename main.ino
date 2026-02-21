#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProBlinds.h>
#include <SinricProLight.h>

#include "config.h"
#include "globals.h"

// ================= GLOBAL DEFINITIONS =================
WiFiServer webServer(8080);
bool isWebStarted = false;

bool isMusicMode = false;
unsigned long lastCmdTime = 0;
bool isSleeping = false;

int targetR = 255;
int targetG = 255;
int targetB = 255;
bool lightPower = false;

float smoothedVal = 2048;
float dcOffset = 2048;

int blindPos = 0;
int blindTarget = 0;
int blindState = 2;
unsigned long blindLastTime = 0;
bool isPaused = false;
unsigned long pauseStart = 0;

unsigned long lastClapTime = 0;
int clapCount = 0;

// ================= FORWARD DECLARATIONS =================
void handleBlinds();
void handleMusic();
void detectDoubleClap();
void checkWebClient();

bool onRangeValue(const String &, int &);
bool onAdjustRangeValue(const String &, int &);
bool onPowerState(const String &, bool &);
bool onColor(const String &, byte &, byte &, byte &);
bool onBrightness(const String &, int &);

// ================= HELPER =================
void updateActivity() {
  lastCmdTime = millis();
  isSleeping = false;
}

void setup() {
  Serial.begin(115200);

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
  WiFi.setSleep(true);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
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

  SinricPro.onConnected([](){ Serial.println("Connected to SinricPro"); });
  SinricPro.onDisconnected([](){ Serial.println("Disconnected from SinricPro"); });

  SinricPro.begin(APP_KEY, APP_SECRET);

  lastCmdTime = millis();
}

void loop() {

  SinricPro.handle();

  if (!isSleeping && !isMusicMode && (millis() - lastCmdTime > SLEEP_TIMEOUT_MS)) {
    isSleeping = true;
  }

  handleBlinds();
  handleMusic();
  detectDoubleClap();
  checkWebClient();
}