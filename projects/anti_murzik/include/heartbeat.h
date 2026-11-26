// Blue LED blinks 4 times a second while loop() is alive.
// If it stutters, some part of loop() is waiting.
#pragma once
#include <Arduino.h>
#include "rgb_led.h"

class Heartbeat {
 public:
  static constexpr uint32_t HEARTBEAT_HALF_MS = 125U;

  explicit Heartbeat(RgbLed &led) : led_(led) {}

  void begin() { last_ = millis(); }

  void update(uint32_t now) {
    if (now - last_ < HEARTBEAT_HALF_MS) return;
    last_ = now;
    on_   = !on_;
    led_.set(RgbLed::Channel::BLUE, on_);
  }

 private:
  RgbLed  &led_;
  uint32_t last_ = 0U;
  bool     on_   = false;
};