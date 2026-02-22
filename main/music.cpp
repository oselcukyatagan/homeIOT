#include "config.h"
#include "globals.h"

void setLEDs(int r, int g, int b) {
  ledcWrite(PIN_RED, 255 - r);
  ledcWrite(PIN_GREEN, 255 - g);
  ledcWrite(PIN_BLUE, 255 - b);
}

void handleMusic() {

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