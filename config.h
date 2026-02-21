#ifndef CONFIG_H
#define CONFIG_H

// ================= CREDENTIALS =================
#define WIFI_SSID         "Hizli ve Sifreli"
#define WIFI_PASS         "k4h4yu7ya4"
#define APP_KEY           "2b17e6f7-536e-4eef-b5fb-a76726192313"
#define APP_SECRET        "9c10708f-7e37-4bb6-92bf-d75ee89e6d44-c36b5709-c179-4d57-81b8-a24e6194a5c8"
#define BLINDS_ID         "698b940fda2ae47a6c938543"
#define LIGHT_ID          "698b948a2c0599192af14a86"

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