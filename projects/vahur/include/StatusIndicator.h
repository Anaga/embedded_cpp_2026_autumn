#pragma once
#include "Config.h"
#include "RgbLed.h"

class StatusIndicator {
public:
  explicit StatusIndicator(RgbLed &led) : led_(led) {}
  void begin() { led_.set(false, false, false); }
  void update(uint32_t now, bool fault, bool pumpOn, bool growLightOn) {
    if (fault) {
      led_.set(((now / cfg::FAULT_BLINK_MS) % 2UL) == 0UL, false, false);
    } else if (pumpOn && growLightOn) {
      // Exactly 800 ms: blue 200 on, 200 off, green 200 on, 200 off.
      const uint32_t phase = now % (4UL * cfg::RGB_BOTH_SEGMENT_MS);
      const bool blue = phase < cfg::RGB_BOTH_SEGMENT_MS;
      const bool green = phase >= 2UL * cfg::RGB_BOTH_SEGMENT_MS &&
                         phase < 3UL * cfg::RGB_BOTH_SEGMENT_MS;
      led_.set(false, green, blue);
    } else if (pumpOn) {
      led_.set(false, false, true);
    } else if (growLightOn) {
      led_.set(false, true, false);
    } else {
      const bool pulse = (now % cfg::HEARTBEAT_PERIOD_MS) < cfg::HEARTBEAT_ON_MS;
      led_.set(false, pulse, false);
    }
  }
private:
  RgbLed &led_;
};
