#ifndef GLOBALS_H
#define GLOBALS_H

#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProBlinds.h>
#include <SinricProLight.h>

// Web
extern WiFiServer webServer;
extern bool isWebStarted;

// Global states
extern bool isMusicMode;
extern unsigned long lastCmdTime;
extern bool isSleeping;

// Light state
extern int targetR;
extern int targetG;
extern int targetB;
extern bool lightPower;

extern float smoothedVal;
extern float dcOffset;

// Blind state
extern int blindPos;
extern int blindTarget;
extern int blindState;
extern unsigned long blindLastTime;
extern bool isPaused;
extern unsigned long pauseStart;

// Clap state
extern unsigned long lastClapTime;
extern int clapCount;

// Functions shared across files
void updateActivity();
void setLEDs(int r, int g, int b);

#endif