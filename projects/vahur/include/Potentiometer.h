#pragma once
#include "Config.h"

// Diagnostic knob reading only in demo mode: pump time is always 30 s.
class Potentiometer {
public:
  void begin() { analogReadResolution(12); analogSetPinAttenuation(cfg::PIN_POT, ADC_11db); pinMode(cfg::PIN_POT, INPUT); }
  void update(uint32_t now) {
    if ((uint32_t)(now - lastMs_) < 100UL) return;
    lastMs_ = now;
    millivolts_ = (uint32_t)analogReadMilliVolts(cfg::PIN_POT);
  }
  uint32_t millivolts() const { return millivolts_; }
private:
  uint32_t lastMs_ = 0U;
  uint32_t millivolts_ = 0U;
};
