// The history: every packet is also stored in a RING BUFFER (the newest
// overwrites the oldest once it's full). /history sums it up in time steps:
// average temperature, humidity, battery, how many packets arrived, and any
// movement. It lives in RAM, so a gateway restart starts it from empty.
//
// loop() adds, the net task reads: a mutex guards the buffer.
#pragma once
#include <Arduino.h>
#include <time.h>

#include "config.h"
#include "msg.h"  // from ../common

class History {
 public:
  void begin() { lock_ = xSemaphoreCreateMutex(); }
  void add(time_t now, const GuardianMsg& m, bool tempOk);  // loop()
  String text(uint8_t hours);                                // net task: the /history table (Markdown)

 private:
  struct Entry {
    uint32_t t;        // clock time (seconds since 1970, UTC)
    int16_t t_c10;     // temperature x10, or NO_TEMP if the sensor was faulty
    uint16_t rh_10;
    uint16_t batt_mV;  // 0 = not measured
    uint8_t flags;
  };
  static constexpr int16_t NO_TEMP = INT16_MIN;

  Entry buf_[HIST_SIZE];
  uint16_t next_ = 0U;   // where the next entry goes
  uint16_t count_ = 0U;  // how many are filled (up to HIST_SIZE)
  SemaphoreHandle_t lock_ = nullptr;
};
