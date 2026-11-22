// The ESP32-C3's own temperature sensor, inside the chip.
//
// temperatureRead() is part of Arduino-ESP32 core 2.x: it starts the sensor,
// reads it and stops it again. No wiring, works on a bare board.
//
// It measures the DIE, not the room: expect 10-20 C above the AHT10, more
// while WiFi is busy. Useful as "is my board running hot", not as a
// thermometer.
#pragma once
#include <Arduino.h>

class ChipTemp {
 public:
  static constexpr uint32_t PERIOD_MS = 2000U;

  void begin() { last_ = millis() - PERIOD_MS; }   // first reading at once

  void update(uint32_t now) {
    if (now - last_ < PERIOD_MS) return;
    last_  = now;
    tempC_ = temperatureRead();
  }

  float celsius() const { return tempC_; }

 private:
  uint32_t last_  = 0U;
  float    tempC_ = NAN;
};
