// The S3's on-board RGB LED (a WS2812 on GPIO 38).
//
// A short flash for every packet, and between flashes the ONE most
// important problem:
//   1. movement            red / blue, like a police car
//   2. battery critical    blinking red
//   3. battery low         steady red
//   4. condensation risk   steady light blue (cyan)
//   5. system fault        blinking yellow (internet, node silent, a sensor)
//   nothing                steady dim green
// Telegram has the details of everything else that's on.
//
// Lesson learned: two WS2812 writes right after each other collide and the
// second one is lost. So it writes only when the colour changes, plus once
// a second in case a write went missing.
#pragma once
#include <Arduino.h>

#include "alarms.h"

class StatusLed {
 public:
  enum class Flash : uint8_t { GOOD, WARN, MOVEMENT };  // green, yellow, blue

  explicit StatusLed(const Alarms& alarms) : alarms_(alarms) {}
  void begin();
  void showError();      // steady red, for "the radio isn't there" at start-up
  void flash(Flash f);   // one packet arrived
  void update(uint32_t now);

 private:
  void write(uint32_t rgb);
  uint32_t stateColour(uint32_t now) const;

  const Alarms& alarms_;
  uint32_t flashUntil_ = 0U;  // 0 = no flash showing
  uint32_t shown_ = UINT32_MAX;
  uint32_t lastWrite_ = 0U;
};
