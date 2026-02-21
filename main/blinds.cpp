#include "config.h"
#include "globals.h"



void handleBlinds() {

  if (isPaused) {
    if (millis() - pauseStart >= 1000) isPaused = false;
    else return;
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
        }
        blindPos++;
      } else {
        if (blindState != 0) {
          blindState = 0;
          digitalWrite(RELAY_DIR, LOW);
        }
        blindPos--;
      }

      digitalWrite(RELAY_PWR, HIGH);

      if (blindPos % 10 == 0 || blindPos == blindTarget) {
        syncBlindPosition(blindPos);
      }
    }

  } else if (blindState != 2) {

    digitalWrite(RELAY_PWR, LOW);
    digitalWrite(RELAY_DIR, LOW);
    blindState = 2;

    syncBlindPosition(blindPos);
  }
}