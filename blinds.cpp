#include "config.h"
#include "globals.h"

bool onRangeValue(const String &deviceId, int &rangeValue) {
  updateActivity();
  blindTarget = rangeValue;
  Serial.printf("Blinds target: %d\r\n", blindTarget);
  return true;
}

bool onAdjustRangeValue(const String &deviceId, int &rangeValueDelta) {
  updateActivity();
  blindTarget += rangeValueDelta;
  if (blindTarget > 100) blindTarget = 100;
  if (blindTarget < 0) blindTarget = 0;
  Serial.printf("Blinds target adjusted to: %d\r\n", blindTarget);
  return true;
}

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
        SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
        myBlinds.sendRangeValueEvent(blindPos);
      }
    }

  } else if (blindState != 2) {

    digitalWrite(RELAY_PWR, LOW);
    digitalWrite(RELAY_DIR, LOW);
    blindState = 2;

    SinricProBlinds &myBlinds = SinricPro[BLINDS_ID];
    myBlinds.sendRangeValueEvent(blindPos);
  }
}