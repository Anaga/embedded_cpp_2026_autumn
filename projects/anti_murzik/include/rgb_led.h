// RGB LED, common anode: LOW turns a channel ON (as in K3).
// Plain on / off per channel - no PWM needed in this demo.
#pragma once
#include <Arduino.h>
#include "board.h"

class RgbLed {
 public:
  enum class Channel : uint8_t { RED, GREEN, BLUE };

  void begin() {
    for (int pin : {PIN_LED_R, PIN_LED_G, PIN_LED_B}) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);        // all off
    }
  }

  void set(Channel c, bool on) {
    digitalWrite(pinOf(c), on ? LOW : HIGH);
  }

  bool get(Channel c) const {
    return digitalRead(pinOf(c)) == LOW;
  }

 private:
  static int pinOf(Channel c) {
    switch (c) {
      case Channel::RED:   return PIN_LED_R;
      case Channel::GREEN: return PIN_LED_G;
      case Channel::BLUE:  return PIN_LED_B;
    }
    return PIN_LED_B;
  }
};