#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// ================= PIN CONFIGURATION =================
#define RELAY_DIR   18 
#define RELAY_PWR   19 

#define PIN_RED     4
#define PIN_GREEN   5 
#define PIN_BLUE    6 

#define PIN_MIC     9

// ================= CLAP DETECTION =================
#define CLAP_THRESHOLD    1200
#define CLAP_DEBOUNCE_MS  250
#define CLAP_WINDOW_MS    800

// ================= TIMING =================
#define TOTAL_TRAVEL_TIME 18500 
#define SLEEP_TIMEOUT_MS  10000 

// ================= AUDIO DSP CONFIG =================
#define MIC_GAIN        320.0 
#define LPF_ALPHA       0.2 
#define NOISE_GATE      30

#endif