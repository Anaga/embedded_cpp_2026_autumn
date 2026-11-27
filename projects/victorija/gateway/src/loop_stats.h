// How long does one loop() pass take? Call update() LAST in loop().
//
// From the lesson 24 demo (src/common/loop_stats.h); only the period changed.
//
// Every STATS_PERIOD_MS it prints, for example:
//   loop: 48213.4 passes/s, longest pass 3 ms
//
// The longest pass is the number that matters: a button press shorter than
// that can be lost, and every other part waits at least that long.
// Same idea as the timing statistics in session 11.
#pragma once
#include <Arduino.h>

class LoopStats {
 public:
  static constexpr uint32_t STATS_PERIOD_MS = 60000U;  // once a minute: the packet lines stay readable

  void begin() {
    lastUs_     = micros();
    lastReport_ = millis();
  }

  void update(uint32_t now) {
    uint32_t us   = micros();
    uint32_t pass = us - lastUs_;     // one loop() pass
    lastUs_ = us;
    if (pass > maxUs_) {
      maxUs_ = pass;
    }
    passes_++;
    if (now - lastReport_ >= STATS_PERIOD_MS) {
      report(now);                    // print, then reset
    }
  }

  // Longest pass of the last finished period, in ms (for /status).
  uint32_t longestMs() const { return lastLongestMs_; }

 private:
  void report(uint32_t now) {
    uint32_t elapsed = now - lastReport_;
    // passes per second with one decimal: the "before" demo does less than 1
    uint32_t perSec10 = static_cast<uint32_t>(
        (static_cast<uint64_t>(passes_) * 10000ULL) / elapsed);
    lastLongestMs_ = maxUs_ / 1000U;
    Serial.printf("loop: %lu.%lu passes/s, longest pass %lu ms\n",
                  static_cast<unsigned long>(perSec10 / 10U),
                  static_cast<unsigned long>(perSec10 % 10U),
                  static_cast<unsigned long>(lastLongestMs_));
    passes_     = 0U;
    maxUs_      = 0U;
    lastReport_ = now;
  }

  uint32_t lastUs_        = 0U;
  uint32_t maxUs_         = 0U;
  uint32_t passes_        = 0U;
  uint32_t lastReport_    = 0U;
  uint32_t lastLongestMs_ = 0U;
};
