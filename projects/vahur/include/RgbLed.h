#pragma once
#include "Config.h"

class RgbLed {
public:
  void begin() {
    // Set HIGH first to keep common-anode LED off while pins are configured.
    digitalWrite(cfg::PIN_RGB_RED, HIGH);
    digitalWrite(cfg::PIN_RGB_GREEN, HIGH);
    digitalWrite(cfg::PIN_RGB_BLUE, HIGH);
    pinMode(cfg::PIN_RGB_RED, OUTPUT);
    pinMode(cfg::PIN_RGB_GREEN, OUTPUT);
    pinMode(cfg::PIN_RGB_BLUE, OUTPUT);
    set(false, false, false);
  }
  void set(bool red, bool green, bool blue) {
    digitalWrite(cfg::PIN_RGB_RED, red ? LOW : HIGH);
    digitalWrite(cfg::PIN_RGB_GREEN, green ? LOW : HIGH);
    digitalWrite(cfg::PIN_RGB_BLUE, blue ? LOW : HIGH);
  }
};
