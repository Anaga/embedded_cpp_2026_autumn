// AHT10 through Adafruit AHTX0: a reading every 2 s, "missing" and recovery.
//
// getEvent() still waits for the conversion (about 80 ms, session 15).
// That is the one short wait we accept here; the by-hand state machine
// from the session 13 lab would remove it too.
#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "board.h"

class Aht10Sensor {
 public:
  static constexpr uint32_t SAMPLE_PERIOD_MS = 2000U;
  static constexpr uint32_t RETRY_PERIOD_MS  = 5000U;
  static constexpr uint8_t  ADDRESS          = 0x38U;

  void begin() {
    Wire.begin(PIN_SDA, PIN_SCL);
    lastTry_ = millis() - RETRY_PERIOD_MS;   // try at once
  }

  void update(uint32_t now) {
    if (!present_) {
      if (now - lastTry_ < RETRY_PERIOD_MS) return;
      lastTry_ = now;
      if (responds() && aht_.begin(&Wire)) {
        present_    = true;
        lastSample_ = now - SAMPLE_PERIOD_MS;  // first reading at once
        Serial.println("AHT10: found");
      }
      return;
    }

    if (now - lastSample_ < SAMPLE_PERIOD_MS) return;
    lastSample_ = now;

    sensors_event_t h;
    sensors_event_t t;
    if (responds() && aht_.getEvent(&h, &t)) {
      valid_ = true;
      tempC_ = t.temperature;
      rhPct_ = h.relative_humidity;
    } else {
      present_ = false;
      valid_   = false;
      lastTry_ = now;
      Serial.println("AHT10: missing");
    }
  }

  bool  valid() const { return valid_; }
  float temperature() const { return tempC_; }
  float humidity() const { return rhPct_; }

 private:
  // Check the address first: the library hangs on a missing sensor.
  static bool responds() {
    Wire.beginTransmission(ADDRESS);
    return Wire.endTransmission() == 0;
  }

  Adafruit_AHTX0 aht_;
  bool     present_    = false;
  bool     valid_      = false;
  float    tempC_      = NAN;
  float    rhPct_      = NAN;
  uint32_t lastTry_    = 0U;
  uint32_t lastSample_ = 0U;
};
