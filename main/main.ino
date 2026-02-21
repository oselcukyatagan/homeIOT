#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProBlinds.h>
#include <SinricProLight.h>

#include "config.h"
#include "globals.h"

// ================= GLOBAL DEFINITIONS =================
WiFiServer webServer(80);
bool isWebStarted = false;

bool isMusicMode = false;

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

  return true;
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

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED &&
        millis() - startAttempt < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi FAILED");
    while (true);
  }

  Serial.println("\r\nWiFi Connected.");
  Serial.print("Web Server URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":80");


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

}

void loop() {

  SinricPro.handle();

  handleBlinds();
  handleMusic();
  checkWebClient();
}

// Add this at the bottom of your main .ino file

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