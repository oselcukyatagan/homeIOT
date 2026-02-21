// The brightness control from the google home is not working.

#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProBlinds.h>
#include <SinricProLight.h>

// ================= CREDENTIALS =================
// ================= CREDENTIALS ================= //   ADD THE CREDENTIALS
#define WIFI_SSID         
#define WIFI_PASS         
#define APP_KEY           
#define APP_SECRET        
#define BLINDS_ID         
#define BLINDS_ID        
#define LIGHT_ID          

// ================= PIN CONFIGURATION =================
#define RELAY_DIR   18 
#define RELAY_PWR   19 

// RGB Pins (Inverted Logic: High = Off)
#define PIN_RED     4
#define PIN_GREEN   5 
#define PIN_BLUE    6 

// Microphone Analog Pin
#define PIN_MIC     9

// ================= CLAP DETECTION =================
#define CLAP_THRESHOLD    1200  // Minimum amplitude deviation from DC bias (2048)
#define CLAP_DEBOUNCE_MS  250   // Minimum time between claps to ignore acoustic reflections
#define CLAP_WINDOW_MS    800   // Maximum allowable time between first and second clap

unsigned long lastClapTime = 0;
int clapCount = 0;
#define PIN_RED     25
#define PIN_GREEN   27 
#define PIN_BLUE    26 

// Microphone Analog Pin
#define PIN_MIC     34

// ================= TIMING =================
#define TOTAL_TRAVEL_TIME 18500 
#define SLEEP_TIMEOUT_MS  10000 

// ================= AUDIO DSP CONFIG =================
#define MIC_GAIN        320.0 
#define LPF_ALPHA       0.2 
#define NOISE_GATE      30

// ================= GLOBALS =================
WiFiServer webServer(8080);
bool isWebStarted = false;
bool isMusicMode = false;
unsigned long lastCmdTime = 0; 
bool isSleeping = false;       

// Light State
int targetR = 255;
int targetG = 255;
int targetB = 255;
bool lightPower = false;
float smoothedVal = 2048;
float dcOffset = 2048;

// Blind State
int blindPos = 0;      // 0-100
int blindTarget = 0;   // 0-100
int blindState = 2;    // 0=Down, 1=Up, 2=Stop
unsigned long blindLastTime = 0;
bool isPaused = false;
unsigned long pauseStart = 0;

// ================= HTML CONTENT =================
const char* htmlPage = 
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>"
".btn{padding:15px;margin:5px;width:100px;cursor:pointer;border:none;border-radius:5px;color:white;font-weight:bold;}"
".u{background:#555}.d{background:#555}.s{background:#f44336}"
".c_music{background:#E91E63;width:80%;display:block;margin:10px auto;}"
".c_exit{background:#D32F2F;width:80%;display:block;margin:10px auto;border:2px solid #fff;}"
"body{font-family:sans-serif;text-align:center;margin-top:20px;background:#f0f0f0}"
"</style>"
"</head>"
"<body>"
"<h1>Living Room Control</h1>"
"<h3>Blinds</h3>"
"<a href='/UP'><button class='btn u'>UP</button></a>"
"<a href='/STOP'><button class='btn s'>STOP</button></a>"
"<a href='/DOWN'><button class='btn d'>DOWN</button></a>"
"<hr>"
"<h3>Modes</h3>"
"<a href='/MUSIC'><button class='btn c_music'>MUSIC MODE</button></a>"
"<a href='/EXIT_MUSIC'><button class='btn c_exit'>EXIT MUSIC (RED)</button></a>"
"</body></html>";

////////////////////////////////////
// HELPER: WAKE UP TRIGGER
////////////////////////////////////
void updateActivity() {
  lastCmdTime = millis();
  isSleeping = false;
}

////////////////////////////////////
// HELPER: SET LEDS
////////////////////////////////////
void setLEDs(int r, int g, int b) {
  // Inverted Logic: 255 - Val
  ledcWrite(PIN_RED, 255 - r);
  ledcWrite(PIN_GREEN, 255 - g);
  ledcWrite(PIN_BLUE, 255 - b);
}

////////////////////////////////////
// SINRIC PRO: BLINDS CALLBACKS
////////////////////////////////////
bool onRangeValue(const String &deviceId, int &rangeValue) {
  updateActivity();
  blindTarget = rangeValue;
  Serial.printf("Blinds target: %d\r\n", blindTarget);
  return true; 
}

bool onAdjustRangeValue(const String &deviceId, int &rangeValueDelta) {
  updateActivity();
  blindTarget += rangeValueDelta;
  if(blindTarget > 100) blindTarget = 100;
  if(blindTarget < 0) blindTarget = 0;
  Serial.printf("Blinds target adjusted to: %d\r\n", blindTarget);
  return true;
}

////////////////////////////////////
// SINRIC PRO: LIGHT CALLBACKS
////////////////////////////////////
bool onPowerState(const String &deviceId, bool &state) {
  updateActivity();
  isMusicMode = false;
  lightPower = state;
  if(lightPower) setLEDs(targetR, targetG, targetB);
  else setLEDs(0, 0, 0);
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  updateActivity();
  isMusicMode = false;
  lightPower = true;
  targetR = r; targetG = g; targetB = b;
  setLEDs(targetR, targetG, targetB);
  return true;
}

bool onBrightness(const String &deviceId, int &brightness) {
  updateActivity();
  // We keep the color ratio but scale it? 
  // For simplicity with RGB strips, we often just acknowledge brightness
  // effectively scaling the last RGB would be better but let's stick to simple logic
  // or just re-apply current RGB.
  return true;
}

////////////////////////////////////
// LOGIC: BLINDS MOVEMENT
////////////////////////////////////
void handleBlinds() {
  if (isPaused) { 
    if (millis() - pauseStart >= 1000) isPaused = false; 
    else return; 
  }

  if (blindPos != blindTarget) {
    bool up = (blindTarget > blindPos);
    
    // Check direction change safety
    if (blindState != 2 && ((blindState == 1 && !up) || (blindState == 0 && up))) {
      digitalWrite(RELAY_PWR, LOW); 
      blindState = 2; 
      isPaused = true; 
      pauseStart = millis(); 
      return;
    }

    // Move
    if (millis() - blindLastTime >= (TOTAL_TRAVEL_TIME / 100)) {
      blindLastTime = millis();
      
      if (up) { 
        if(blindState != 1) { blindState = 1; digitalWrite(RELAY_DIR, HIGH); }
        blindPos++; 
      } else { 
        if(blindState != 0) { blindState = 0; digitalWrite(RELAY_DIR, LOW); }
        blindPos--; 
      }
      
      digitalWrite(RELAY_PWR, HIGH);
      
      // Update SinricPro periodically (every 10 steps or at end)
      if (blindPos % 10 == 0 || blindPos == blindTarget) {
         SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
         myBlinds.sendRangeValueEvent(blindPos);
      }
    }
  } else if (blindState != 2) {
    // Stop
    digitalWrite(RELAY_PWR, LOW); 
    digitalWrite(RELAY_DIR, LOW); 
    blindState = 2;
    SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
    myBlinds.sendRangeValueEvent(blindPos);
  }
}

////////////////////////////////////
// LOGIC: MUSIC DSP
////////////////////////////////////
void handleMusic() {
  if (isSleeping) return;
  
  if (isMusicMode) {
    int raw = analogRead(PIN_MIC);
    dcOffset = (dcOffset * 0.99) + (raw * 0.01); 
    smoothedVal = (smoothedVal * (1.0 - LPF_ALPHA)) + (raw * LPF_ALPHA);
    float amplitude = abs(smoothedVal - dcOffset);
    if (amplitude < NOISE_GATE) amplitude = 0;
    
    float volumeFactor = (amplitude * MIC_GAIN) / 4095.0;
    if (volumeFactor > 1.0) volumeFactor = 1.0;
    
    // Scale stored target RGB
    setLEDs((int)(targetR * volumeFactor), (int)(targetG * volumeFactor), (int)(targetB * volumeFactor));
  }
}

////////////////////////////////////
// WEB SERVER
////////////////////////////////////
void checkWebClient() {
  WiFiClient client = webServer.available();
  if (!client) return;
  
  updateActivity(); 
  
  String req = ""; unsigned long start = millis();
  while (client.connected() && millis() - start <= 200) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n') {
        if (req.startsWith("GET")) {
          if (req.indexOf("/UP") >= 0) blindTarget = 100;
          else if (req.indexOf("/DOWN") >= 0) blindTarget = 0;
          else if (req.indexOf("/STOP") >= 0) blindTarget = blindPos;
          else if (req.indexOf("/MUSIC") >= 0) { 
            isMusicMode = true; 
            lightPower = true;
          }
          else if (req.indexOf("/EXIT_MUSIC") >= 0) { 
             isMusicMode = false;
             targetR=255; targetG=0; targetB=0;
             setLEDs(255, 0, 0);
             lightPower = true;
             SinricProLight &myLight = SinricPro[LIGHT_ID];
             myLight.sendPowerStateEvent(true);
             myLight.sendColorEvent(255, 0, 0);
          }
          client.print(htmlPage); break;
        }
        req = "";
      } else if (c != '\r') req += c;
    }
  }
  client.stop();
}

////////////////////////////////////
// LOGIC: CLAP DETECTION
////////////////////////////////////
void detectDoubleClap() {
  if (isMusicMode) return; 

  int raw = analogRead(PIN_MIC);
  
  // Dynamically track true DC bias to prevent static offset triggering
  dcOffset = (dcOffset * 0.999) + (raw * 0.001); 
  
  // Measure transient against the calibrated baseline
  int amplitude = abs(raw - dcOffset); 

  if (amplitude > CLAP_THRESHOLD) {
    unsigned long now = millis();
    
    if (clapCount == 0) {
      clapCount = 1;
      lastClapTime = now;
    } else if (clapCount == 1) {
      unsigned long timeSinceLast = now - lastClapTime;
      
      // Reject continuous noise spikes that happen faster than physical claps
      if (timeSinceLast > CLAP_DEBOUNCE_MS && timeSinceLast < CLAP_WINDOW_MS) {
        updateActivity();
        lightPower = !lightPower;
        
        if (lightPower) setLEDs(targetR, targetG, targetB);
        else setLEDs(0, 0, 0);
        
        SinricProLight &myLight = SinricPro[LIGHT_ID];
        myLight.sendPowerStateEvent(lightPower);
        
        clapCount = 0; 
      } else if (timeSinceLast >= CLAP_WINDOW_MS) {
        // Window expired, treat as new first clap
        lastClapTime = now;
      }
    }
  }

  // Reset state machine if window expires
  if (clapCount == 1 && (millis() - lastClapTime > CLAP_WINDOW_MS)) {
    clapCount = 0;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Pins
  pinMode(PIN_MIC, INPUT);
  ledcAttach(PIN_RED, 5000, 8); 
  ledcAttach(PIN_GREEN, 5000, 8); 
  ledcAttach(PIN_BLUE, 5000, 8);
  pinMode(RELAY_DIR, OUTPUT); 
  pinMode(RELAY_PWR, OUTPUT);
  digitalWrite(RELAY_PWR, LOW); 
  digitalWrite(RELAY_DIR, LOW);
  
  // Init LEDs (Off)
  setLEDs(0, 0, 0);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setSleep(true); // Modem Sleep
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(500);
  }
  webServer.begin();
  isWebStarted = true;

  // SinricPro Blinds
  SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
  myBlinds.onRangeValue(onRangeValue);
  myBlinds.onAdjustRangeValue(onAdjustRangeValue);

  // SinricPro Light
  SinricProLight &myLight = SinricPro[LIGHT_ID];
  myLight.onPowerState(onPowerState);
  myLight.onColor(onColor);
  myLight.onBrightness(onBrightness);

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  
  SinricPro.begin(APP_KEY, APP_SECRET);
  
  lastCmdTime = millis();
}

void loop() {

  
  SinricPro.handle();
  
  // Sleep Logic
  // If no activity for 10s and NOT in music mode -> Logic Sleep
  if (!isSleeping && !isMusicMode && (millis() - lastCmdTime > SLEEP_TIMEOUT_MS)) {
    isSleeping = true;
    // LEDs remain in last state
  }

  // Active Loops
  handleBlinds();
  handleMusic();
  detectDoubleClap(); // Added function call
  checkWebClient();
  
  checkWebClient();
}