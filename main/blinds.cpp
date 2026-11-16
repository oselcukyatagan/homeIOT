#include <Preferences.h>
#include "config.h"
#include "globals.h"

static Preferences prefs;
static int lastSavedPos = -1;

void loadBlindPosition() {
    prefs.begin("blinds", false);
    blindPos = constrain(prefs.getInt("pos", 0), 0, 100);
    blindTarget = blindPos;   // without this the blinds drive to 0 on boot
    lastSavedPos = blindPos;
}

void saveBlindPosition() {
    if (blindPos == lastSavedPos) return;   // skips writes that would only wear the flash
    prefs.putInt("pos", blindPos);
    lastSavedPos = blindPos;
}

// SinricPro drops events sent less than 1 s apart (5% of travel is ~0.9 s),
// so hold the update back and send the latest position once the limit allows
static const unsigned long SYNC_INTERVAL_MS = 1000;
static unsigned long lastSyncTime = 0;
static bool syncPending = false;

static void reportBlindPosition() {
    if (millis() - lastSyncTime >= SYNC_INTERVAL_MS) {
        syncBlindPosition(blindPos);
        lastSyncTime = millis();
        syncPending = false;
    } else {
        syncPending = true;
    }
}

void handleBlinds() {

    if (syncPending && millis() - lastSyncTime >= SYNC_INTERVAL_MS) {
        reportBlindPosition();
    }

    if (isPaused) {
        if (millis() - pauseStart >= 1000) isPaused = false;
        else return;
    }

    if (dirChangePending) {
        if (millis() - dirChangeTime >= 50) {
            dirChangePending = false;
        } else {
            return;
        }
    }

    if (blindPos != blindTarget) {
        bool up = (blindTarget > blindPos);

        if (blindState != 2 && ((blindState == 1 && !up) || (blindState == 0 && up))) {
            digitalWrite(RELAY_PWR, LOW);
            blindState = 2;
            isPaused = true;
            pauseStart = millis();
            return;
        }

        if (millis() - blindLastTime >= (TOTAL_TRAVEL_TIME / 100)) {
            blindLastTime = millis();

            if (up) {
                if (blindState != 1) {
                    blindState = 1;
                    digitalWrite(RELAY_DIR, HIGH);
                    dirChangePending = true;
                    dirChangeTime = millis();
                    return;
                }
                if (blindPos < 100) blindPos++;
            } else {
                if (blindState != 0) {
                    blindState = 0;  // correct for down
                    digitalWrite(RELAY_DIR, LOW);
                    dirChangePending = true;
                    dirChangeTime = millis();
                    return;
                }
                if (blindPos > 0) blindPos--;
            }

            if (blindPos != blindTarget) {
                digitalWrite(RELAY_PWR, HIGH);
            }

            if (blindPos % 5 == 0 || blindPos == blindTarget) {
                reportBlindPosition();
                saveBlindPosition();

                Serial.printf("Blind position: %d\n", blindPos);
                if (telnetClient && telnetClient.connected()) {
                    telnetClient.printf("Blind position: %d\r\n", blindPos);
                }
            }
        }

    } else if (blindState != 2) {
        digitalWrite(RELAY_PWR, LOW);
        digitalWrite(RELAY_DIR, LOW);
        blindState = 2;

        reportBlindPosition();
        saveBlindPosition();

        Serial.printf("Blind position: %d (stopped)\n", blindPos);
        if (telnetClient && telnetClient.connected()) {
            telnetClient.printf("Blind position: %d (stopped)\r\n", blindPos);
        }
    }
}