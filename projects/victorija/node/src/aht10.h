// AHT10 driver without a library. Command-based, not register-based:
//   0xE1 0x08 0x00 init, 0xAC 0x33 0x00 measure (~75 ms), then 6 bytes.
#pragma once

#include <Arduino.h>
#include <Wire.h>

class Aht10 {
 public:
  static constexpr uint8_t ADDRESS = 0x38;

  bool begin(TwoWire& wire = Wire) {
    wire_ = &wire;
    delay(40);  // power-up time

    wire_->beginTransmission(ADDRESS);
    if (wire_->endTransmission() != 0) return false;

    if (calibrated()) return true;
    command(0xE1, 0x08, 0x00);  // AHT10 init
    delay(10);
    if (calibrated()) return true;
    command(0xBE, 0x08, 0x00);  // AHT20 init: some "AHT10" modules carry an AHT20
    delay(10);
    return calibrated();
  }

  bool read(float& tempC, float& humidity) {
    if (!command(0xAC, 0x33, 0x00)) return false;
    delay(80);

    uint8_t d[6];
    for (int tries = 0; tries < 5; tries++) {
      if (wire_->requestFrom(ADDRESS, (uint8_t)6) != 6) return false;
      for (uint8_t& b : d) b = wire_->read();
      if ((d[0] & 0x80) == 0) break;  // status bit 7 = busy
      delay(20);
    }
    if (d[0] & 0x80) return false;

    // 20-bit values: humidity = d1 d2 d3[7:4], temperature = d3[3:0] d4 d5
    uint32_t rawH = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
    uint32_t rawT = (((uint32_t)d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];

    humidity = rawH * 100.0f / 1048576.0f;
    tempC = rawT * 200.0f / 1048576.0f - 50.0f;
    return true;
  }

 private:
  TwoWire* wire_ = nullptr;

  bool command(uint8_t a, uint8_t b, uint8_t c) {
    wire_->beginTransmission(ADDRESS);
    wire_->write(a);
    wire_->write(b);
    wire_->write(c);
    return wire_->endTransmission() == 0;
  }

  bool calibrated() {  // status bit 3
    if (wire_->requestFrom(ADDRESS, (uint8_t)1) != 1) return false;
    return wire_->read() & 0x08;
  }
};
