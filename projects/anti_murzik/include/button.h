// Button with debouncing, active LOW (INPUT_PULLUP).
// Usage: begin() once, update(now) every loop pass, pressed() returns true once per press.
#pragma once
#include <Arduino.h>

class Button {
 public:
  explicit Button(int pin, uint32_t debounceMs = 50) : pin_(pin), debounceMs_(debounceMs) {}

  void begin() {
    pinMode(pin_, INPUT_PULLUP);
    lastState_ = digitalRead(pin_);
    stableState_ = lastState_;
    lastChange_ = millis();
  }

  void update(uint32_t now) {
    bool raw = digitalRead(pin_);
    if (raw != lastState_) {
      lastState_ = raw;
      lastChange_ = now;
    }
    if (now - lastChange_ >= debounceMs_ && raw != stableState_) {
      stableState_ = raw;
      justPressed_ = (raw == LOW);  // active low
    } else {
      justPressed_ = false;
    }
  }

  bool pressed() const { return justPressed_; }
  bool isPressed() const { return stableState_ == LOW; }

 private:
  int pin_;
  uint32_t debounceMs_;
  bool lastState_ = HIGH;
  bool stableState_ = HIGH;
  bool justPressed_ = false;
  uint32_t lastChange_ = 0U;
};