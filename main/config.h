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


// ================= TIMING =================
#define TOTAL_TRAVEL_TIME 18500 

// ================= AUDIO DSP CONFIG =================
#define MIC_GAIN        320.0 
#define LPF_ALPHA       0.2 
#define NOISE_GATE      30

// ================= AUTO-OFF CONFIG =================
#define CHECK_INTERVAL_MS     60000  // 1 minute total cycle
#define LISTEN_DURATION_MS    5000   // 5 seconds of active listening
#define SILENCE_TIMEOUT_MIN   15     // Turn off after 15 silent checks
#define AUTO_OFF_THRESHOLD    300    // Threshold to ignore low noise

#define WDT_TIMEOUT 60
#define WIFI_RECONNECT_INTERVAL 5000

#endif