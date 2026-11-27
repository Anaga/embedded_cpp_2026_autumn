#pragma once
#include "Config.h"

class LightController {
public:
  void begin(uint32_t now) {
    digitalWrite(cfg::PIN_LIGHT, LOW);
    pinMode(cfg::PIN_LIGHT, OUTPUT);
    ledcSetup(cfg::CH_LIGHT, cfg::PWM_HZ, cfg::PWM_BITS);
    ledcAttachPin(cfg::PIN_LIGHT, cfg::CH_LIGHT);
    ledcWrite(cfg::CH_LIGHT, 0U);
    duty_ = 0U;
    cycleStartMs_ = now;
  }
  void update(uint32_t now, bool sensorOK, bool thermalPause, bool pumpFault) {
    // Millis subtraction remains correct across overflow for these short periods.
    // One division replaces an unbounded while loop after long uptime.
    const uint32_t elapsed = (uint32_t)(now - cycleStartMs_);
    if (elapsed >= cfg::LIGHT_CYCLE_MS) {
      cycleStartMs_ += (elapsed / cfg::LIGHT_CYCLE_MS) * cfg::LIGHT_CYCLE_MS;
    }
    const uint32_t phase = (uint32_t)(now - cycleStartMs_);
    uint32_t requested = 0U;
    if (phase < cfg::LIGHT_ON_MS) {
      if (phase < cfg::LIGHT_FADE_MS) {
        requested = (uint32_t)(((uint64_t)cfg::PWM_MAX * phase) / cfg::LIGHT_FADE_MS);
      } else if (phase >= cfg::LIGHT_ON_MS - cfg::LIGHT_FADE_MS) {
        requested = (uint32_t)(((uint64_t)cfg::PWM_MAX * (cfg::LIGHT_ON_MS - phase)) / cfg::LIGHT_FADE_MS);
      } else {
        requested = cfg::PWM_MAX;
      }
    }
    // Keep the safety conditions from main(4).cpp.
    if (!sensorOK || thermalPause || pumpFault) requested = 0U;
    if (requested != duty_) { ledcWrite(cfg::CH_LIGHT, requested); duty_ = requested; }
  }
  bool on() const { return duty_ > 0U; }
  uint32_t dutyPercent() const { return duty_ * 100UL / cfg::PWM_MAX; }
  uint32_t phaseMs(uint32_t now) const { return (uint32_t)(now - cycleStartMs_); }
private:
  uint32_t duty_ = 0U;
  uint32_t cycleStartMs_ = 0U;
};
