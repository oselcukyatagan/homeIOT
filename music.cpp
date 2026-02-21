#include "config.h"
#include "globals.h"

void setLEDs(int r, int g, int b) {
  ledcWrite(PIN_RED, 255 - r);
  ledcWrite(PIN_GREEN, 255 - g);
  ledcWrite(PIN_BLUE, 255 - b);
}

bool onPowerState(const String &deviceId, bool &state) {
  updateActivity();
  isMusicMode = false;
  lightPower = state;

  if (lightPower) setLEDs(targetR, targetG, targetB);
  else setLEDs(0, 0, 0);

  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  updateActivity();
  isMusicMode = false;
  lightPower = true;

  targetR = r;
  targetG = g;
  targetB = b;

  setLEDs(targetR, targetG, targetB);
  return true;
}

bool onBrightness(const String &deviceId, int &brightness) {
  updateActivity();
  return true;
}

void handleMusic() {

  if (isSleeping) return;

  if (isMusicMode) {

    int raw = analogRead(PIN_MIC);

    dcOffset = (dcOffset * 0.99) + (raw * 0.01);
    smoothedVal = (smoothedVal * (1.0 - LPF_ALPHA)) + (raw * LPF_ALPHA);

    float amplitude = abs(smoothedVal - dcOffset);

    if (amplitude < NOISE_GATE) amplitude = 0;

    float volumeFactor = (amplitude * MIC_GAIN) / 4095.0;
    if (volumeFactor > 1.0) volumeFactor = 1.0;

    setLEDs(
      (int)(targetR * volumeFactor),
      (int)(targetG * volumeFactor),
      (int)(targetB * volumeFactor)
    );
  }
}