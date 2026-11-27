// A small AHT10 driver, written out instead of using a library, so every
// byte on the I2C bus is visible.
//
// The AHT10 has no registers to read like the SX1278. It works with
// commands instead:
//   0xE1 0x08 0x00   "initialise / load calibration" (once after power-up)
//   0xAC 0x33 0x00   "measure now" -> the result is ready after ~75 ms
//   read 6 bytes     status, then 20 bits humidity, then 20 bits temperature
//
// Some modules sold as AHT10 actually carry the newer AHT20, whose
// initialise command is 0xBE instead of 0xE1. begin() tries both.

#pragma once

#include <Arduino.h>
#include <Wire.h>

class Aht10 {
 public:
  static constexpr uint8_t ADDRESS = 0x38U;  // printed "0x38" on the module (the default pad)

  // Returns true if the sensor answered and reports "calibrated".
  bool begin(TwoWire& wire = Wire) {
    wire_ = &wire;
    delay(POWER_UP_MS);

    // Is anything at 0x38 at all? (an empty write = "are you there?")
    wire_->beginTransmission(ADDRESS);
    if (wire_->endTransmission() != 0U) return false;

    if (calibrated()) return true;
    command(CMD_INIT_AHT10, INIT_ARG1, INIT_ARG2);
    delay(INIT_MS);
    if (calibrated()) return true;
    command(CMD_INIT_AHT20, INIT_ARG1, INIT_ARG2);
    delay(INIT_MS);
    return calibrated();
  }

  // Takes one measurement (~80 ms). Returns false if the sensor didn't
  // answer, or stayed busy.
  bool read(float& tempC, float& humidity) {
    if (!command(CMD_MEASURE, MEASURE_ARG1, MEASURE_ARG2)) return false;
    delay(MEASURE_MS);

    uint8_t d[RESULT_BYTES];
    for (uint8_t tries = 0U; tries < BUSY_TRIES; tries++) {
      if (wire_->requestFrom(ADDRESS, RESULT_BYTES) != RESULT_BYTES) return false;
      for (uint8_t& b : d) b = static_cast<uint8_t>(wire_->read());
      if ((d[0] & STATUS_BUSY) == 0U) break;
      delay(BUSY_RETRY_MS);
    }
    if ((d[0] & STATUS_BUSY) != 0U) return false;

    // 20-bit raw values, packed across the bytes:
    //   humidity    = d1 d2 and the top half of d3
    //   temperature = the bottom half of d3, d4 d5
    uint32_t rawH = (static_cast<uint32_t>(d[1]) << 12) | (static_cast<uint32_t>(d[2]) << 4) | (d[3] >> 4);
    uint32_t rawT = ((static_cast<uint32_t>(d[3]) & 0x0FU) << 16) | (static_cast<uint32_t>(d[4]) << 8) | d[5];

    // From the datasheet: full scale is 2^20 = 1048576.
    humidity = rawH * 100.0f / FULL_SCALE;       // 0..100 %
    tempC = rawT * 200.0f / FULL_SCALE - 50.0f;  // -50..150 C
    return true;
  }

 private:
  // Commands and their two argument bytes (datasheet).
  static constexpr uint8_t CMD_INIT_AHT10 = 0xE1U;
  static constexpr uint8_t CMD_INIT_AHT20 = 0xBEU;
  static constexpr uint8_t INIT_ARG1 = 0x08U, INIT_ARG2 = 0x00U;
  static constexpr uint8_t CMD_MEASURE = 0xACU;
  static constexpr uint8_t MEASURE_ARG1 = 0x33U, MEASURE_ARG2 = 0x00U;
  // Status byte bits.
  static constexpr uint8_t STATUS_BUSY = 0x80U;        // bit 7: still measuring
  static constexpr uint8_t STATUS_CALIBRATED = 0x08U;  // bit 3: calibration loaded
  static constexpr uint8_t RESULT_BYTES = 6U;
  static constexpr uint8_t BUSY_TRIES = 5U;
  static constexpr float FULL_SCALE = 1048576.0f;  // 2^20
  // Timings.
  static constexpr uint32_t POWER_UP_MS = 40U;  // the sensor needs ~20-40 ms after power-up
  static constexpr uint32_t INIT_MS = 10U;
  static constexpr uint32_t MEASURE_MS = 80U;   // ~75 ms conversion
  static constexpr uint32_t BUSY_RETRY_MS = 20U;

  TwoWire* wire_ = nullptr;

  bool command(uint8_t a, uint8_t b, uint8_t c) {
    wire_->beginTransmission(ADDRESS);
    wire_->write(a);
    wire_->write(b);
    wire_->write(c);
    return wire_->endTransmission() == 0U;  // 0 = the sensor acknowledged every byte
  }

  bool calibrated() {
    if (wire_->requestFrom(ADDRESS, static_cast<uint8_t>(1U)) != 1U) return false;
    return (wire_->read() & STATUS_CALIBRATED) != 0;
  }
};
