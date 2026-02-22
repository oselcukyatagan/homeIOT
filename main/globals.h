#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

// Web
extern WiFiServer webServer;
extern bool isWebStarted;

// Global states
extern bool isMusicMode;
extern WiFiClient telnetClient;

// Auto-off variables
extern int minutesRemaining;
extern unsigned long lastCheckMillis;
extern bool isCurrentlyListening;
extern bool audioDetectedInWindow;
extern String lastDetectionStatus;



// Update forward declarations
void handleAutoOff();

// Light state
extern int targetR;
extern int targetG;
extern int targetB;
extern bool lightPower;

extern int currentBrightness;

extern float smoothedVal;
extern float dcOffset;

extern int lastRawMic;

// Blind state
extern int blindPos;
extern int blindTarget;
extern int blindState;
extern unsigned long blindLastTime;
extern bool isPaused;
extern unsigned long pauseStart;
extern bool dirChangePending;
extern unsigned long dirChangeTime;

// Functions shared across files
void setLEDs(int r, int g, int b);

void syncBlindPosition(int pos);
void syncLightPower(bool state);
void syncLightColor(int r, int g, int b);

void checkWebClient();


#endif

