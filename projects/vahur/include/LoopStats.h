#pragma once
#include "Config.h"

class LoopStats {
public:
  void begin(uint32_t now) { lastReportMs_ = now; lastUs_ = micros(); }
  void update(uint32_t now) {
    const uint32_t us = micros();
    const uint32_t duration = us - lastUs_;
    lastUs_ = us;
    if (duration > longestUs_) longestUs_ = duration;
    ++passes_;
    if ((uint32_t)(now - lastReportMs_) >= cfg::STATS_PERIOD_MS) {
      Serial.printf("LOOP: passes=%lu in 5s, longest interval=%lu us\n",
                    (unsigned long)passes_, (unsigned long)longestUs_);
      lastReportMs_ = now;
      passes_ = 0U;
      longestUs_ = 0U;
    }
  }
private:
  uint32_t lastUs_ = 0U;
  uint32_t lastReportMs_ = 0U;
  uint32_t longestUs_ = 0U;
  uint32_t passes_ = 0U;
};
