// Debounced button, active low with INPUT_PULLUP (the K3 Button, shortened).
// A change counts only after DEBOUNCE_MS of quiet. pressed() is true for
// exactly one loop() pass per press.
#pragma once
#include <Arduino.h>

class Button {
 public:
  static constexpr uint32_t DEBOUNCE_MS = 30U;

  explicit Button(int pin) : pin_(pin) {}

  void begin() {
    pinMode(pin_, INPUT_PULLUP);
    raw_       = (digitalRead(pin_) == LOW);
    stable_    = raw_;
    changedAt_ = millis();
  }

  void update(uint32_t now) {
    pressed_  = false;
    bool down = (digitalRead(pin_) == LOW);
    if (down != raw_) {               // something moved: restart the quiet time
      raw_       = down;
      changedAt_ = now;
    } else if ((down != stable_) && (now - changedAt_ >= DEBOUNCE_MS)) {
      stable_  = down;
      pressed_ = down;                // count the press, not the release
    }
  }

  bool pressed() const { return pressed_; }

 private:
  int      pin_;
  bool     raw_       = false;
  bool     stable_    = false;
  bool     pressed_   = false;
  uint32_t changedAt_ = 0U;
};
