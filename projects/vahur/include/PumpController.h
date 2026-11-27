#pragma once
#include "Config.h"

class PumpController {
public:
  void begin(uint32_t now) {
    digitalWrite(cfg::PIN_PUMP, LOW);
    pinMode(cfg::PIN_PUMP, OUTPUT);
    ledcSetup(cfg::CH_PUMP, cfg::PWM_HZ, cfg::PWM_BITS);
    ledcAttachPin(cfg::PIN_PUMP, cfg::CH_PUMP);
    ledcWrite(cfg::CH_PUMP, 0U);
    duty_ = 0U;
    bootMs_ = now;
    cycleStartMs_ = now;
    fault_ = false;
    running_ = false;
    // On immediately, at the start of the 30-s ON phase.
    start(now);
  }

  void update(uint32_t now) {
    if (fault_) { write(0U); return; }
    if (running_) {
      const uint32_t elapsed = now - runStartMs_;
      if (elapsed >= cfg::PUMP_HARD_LIMIT_MS) {
        stop(now);
        fault_ = true;
        Serial.println("FAULT: pump hard time limit exceeded; reset required");
        return;
      }
      if (elapsed >= cfg::PUMP_ON_MS) { stop(now); return; }
      uint32_t requested = cfg::PWM_MAX;
      if (cfg::PUMP_PWM_RAMP && elapsed < cfg::PUMP_RAMP_MS) {
        const uint32_t initial = cfg::PUMP_START_DUTY_PERCENT * cfg::PWM_MAX / 100UL;
        requested = initial + (uint32_t)(((uint64_t)(cfg::PWM_MAX - initial) * elapsed) / cfg::PUMP_RAMP_MS);
      }
      write(requested);
    } else {
      // Fixed start-to-start cycle; no drift if the main loop is a little late.
      if ((uint32_t)(now - cycleStartMs_) >= cfg::PUMP_CYCLE_MS) {
        cycleStartMs_ += cfg::PUMP_CYCLE_MS;
        start(now);
      }
    }
  }

  void startManual(uint32_t now) {
    if (fault_) { Serial.println("Pump FAULT; reset first"); return; }
    if (running_) return;
    cycleStartMs_ = now; // manual start begins a fresh 120-s cycle
    start(now);
  }
  void stopManual(uint32_t now) {
    if (running_) stop(now);
  }
  bool running() const { return running_; }
  bool fault() const { return fault_; }
  uint32_t dutyPercent() const { return duty_ * 100UL / cfg::PWM_MAX; }
private:
  void write(uint32_t requested) {
    if (requested > cfg::PWM_MAX) requested = cfg::PWM_MAX;
    if (requested != duty_) { ledcWrite(cfg::CH_PUMP, requested); duty_ = requested; }
  }
  void start(uint32_t now) {
    running_ = true;
    runStartMs_ = now;
    write(0U); // PWM duty begins when update() is called
    Serial.printf("[%lu s] pump ON for 30 s (PWM ramp ON)\n", (unsigned long)(now / 1000UL));
  }
  void stop(uint32_t now) {
    write(0U);
    running_ = false;
    Serial.printf("[%lu s] pump OFF\n", (unsigned long)(now / 1000UL));
  }
  uint32_t bootMs_ = 0U;
  uint32_t cycleStartMs_ = 0U;
  uint32_t runStartMs_ = 0U;
  uint32_t duty_ = 0U;
  bool running_ = false;
  bool fault_ = false;
};
