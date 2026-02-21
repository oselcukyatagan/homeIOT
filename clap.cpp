#include "config.h"
#include "globals.h"

void detectDoubleClap() {

  if (isMusicMode) return;

  int raw = analogRead(PIN_MIC);

  dcOffset = (dcOffset * 0.999) + (raw * 0.001);
  int amplitude = abs(raw - dcOffset);

  if (amplitude > CLAP_THRESHOLD) {

    unsigned long now = millis();

    if (clapCount == 0) {
      clapCount = 1;
      lastClapTime = now;

    } else if (clapCount == 1) {

      unsigned long timeSinceLast = now - lastClapTime;

      if (timeSinceLast > CLAP_DEBOUNCE_MS &&
          timeSinceLast < CLAP_WINDOW_MS) {

        updateActivity();
        lightPower = !lightPower;

        if (lightPower)
          setLEDs(targetR, targetG, targetB);
        else
          setLEDs(0, 0, 0);

        SinricProLight &myLight = SinricPro[LIGHT_ID];
        myLight.sendPowerStateEvent(lightPower);

        clapCount = 0;

      } else if (timeSinceLast >= CLAP_WINDOW_MS) {
        lastClapTime = now;
      }
    }
  }

  if (clapCount == 1 &&
      (millis() - lastClapTime > CLAP_WINDOW_MS)) {
    clapCount = 0;
  }
}