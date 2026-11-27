// ESP32-C3 internal temperature sensor.
// Reads the die temperature in Celsius.
#pragma once
#include <Arduino.h>

class ChipTemp {
 public:
  void begin() {
    // On ESP32-C3, the temperature sensor is always available
    // No initialization needed for built-in sensor
    lastRead_ = 0;
    tempC_ = NAN;
  }

  void update(uint32_t now) {
    if (now - lastRead_ < 2000U) return;  // Read every 2 seconds
    lastRead_ = now;
    tempC_ = temperatureRead();  // ESP32 built-in function
  }

  float celsius() const { return tempC_; }
  bool valid() const { return !isnan(tempC_); }

 private:
  uint32_t lastRead_ = 0U;
  float tempC_ = NAN;
};