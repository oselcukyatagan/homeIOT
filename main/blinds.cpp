#include "config.h"
#include "globals.h"

void handleBlinds() {

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

            if (blindPos % 10 == 0 || blindPos == blindTarget) {
                syncBlindPosition(blindPos);

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

        syncBlindPosition(blindPos);

        Serial.printf("Blind position: %d (stopped)\n", blindPos);
        if (telnetClient && telnetClient.connected()) {
            telnetClient.printf("Blind position: %d (stopped)\r\n", blindPos);
        }
    }
}