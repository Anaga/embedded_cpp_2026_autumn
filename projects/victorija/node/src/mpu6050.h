// A small MPU-6050 driver, written out like aht10.h, so every register is
// visible: the accelerometer read, and the motion interrupt that wakes
// the node.
//
// Unlike the AHT10 (commands), the MPU-6050 works with REGISTERS, like the
// SX1278: numbered 8-bit boxes. Writing one = [address][register][value].
// Reading = [address][register], then a "repeated start", then read bytes.
// The chip moves to the next register by itself, so one read of 6 bytes
// from ACCEL_XOUT_H gets X, Y and Z in one go.
//
// Register numbers: MPU-6050 register map, rev 4.2.

#pragma once

#include <Arduino.h>
#include <Wire.h>

class Mpu6050 {
 public:
  static constexpr uint8_t ADDRESS = 0x68U;  // AD0 pin low (the GY-521 pulls it low)

  uint8_t whoAmI = 0U;  // 0x68 on a real MPU-6050 (some clones answer 0x70, 0x72, 0x98)

  // Wakes the chip and sets +-2 g. Returns false if nothing answers at 0x68.
  bool begin(TwoWire& wire = Wire) {
    wire_ = &wire;
    if (!present()) return false;
    whoAmI = readReg(REG_WHO_AM_I);
    writeReg(REG_PWR_MGMT_1, PWR1_AWAKE);
    delay(WAKE_MS);
    writeReg(REG_ACCEL_CONFIG, ACCEL_2G);
    return true;
  }

  // Acceleration in g (1 g = gravity), and the chip's own temperature.
  bool read(float& x, float& y, float& z, float& tempC) {
    uint8_t d[8];
    if (!readRegs(REG_ACCEL_XOUT_H, d, sizeof d)) return false;
    // Each value is two bytes, high byte first, as a signed 16-bit number.
    int16_t rx = static_cast<int16_t>((d[0] << 8) | d[1]);
    int16_t ry = static_cast<int16_t>((d[2] << 8) | d[3]);
    int16_t rz = static_cast<int16_t>((d[4] << 8) | d[5]);
    int16_t rt = static_cast<int16_t>((d[6] << 8) | d[7]);
    x = rx / COUNTS_PER_G;
    y = ry / COUNTS_PER_G;
    z = rz / COUNTS_PER_G;
    tempC = rt / 340.0f + 36.53f;  // formula from the register map
    return true;
  }

  // ---- The MPU-6050 as a motion alarm ------------------------------------
  //
  // While the C3 sleeps, the MPU-6050 keeps watching by itself: it wakes
  // 20 times a second, measures, and compares with the previous sample. If
  // the change is bigger than the threshold for long enough, it raises its
  // INT pin HIGH and KEEPS it high ("latched") until someone reads
  // INT_STATUS. INT is wired to C3 GPIO 1, a pin that can wake the C3 from
  // deep sleep.
  //
  // Current: ~70 uA in 20 Hz cycle mode (datasheet), vs ~3.9 mA fully on.
  // Call once after power-up (cold boot): the MPU keeps power and settings
  // through the C3's deep sleep.
  bool setupMotionWake(uint8_t threshold, uint8_t durationMs) {
    if (!present()) return false;
    writeReg(REG_PWR_MGMT_1, PWR1_RESET);
    delay(RESET_MS);
    writeReg(REG_PWR_MGMT_1, PWR1_AWAKE);
    delay(WAKE_MS);
    writeReg(REG_ACCEL_CONFIG, ACCEL_2G_HPF_5HZ);
    writeReg(REG_MOT_THR, threshold);  // roughly 2 mg per step (chip-dependent: tune it)
    writeReg(REG_MOT_DUR, durationMs); // samples in a row over the threshold
    writeReg(REG_MOT_DETECT_CTRL, MOT_DETECT_DEFAULT);
    writeReg(REG_INT_PIN_CFG, INT_HIGH_LATCHED);
    writeReg(REG_INT_ENABLE, INT_MOTION);
    delay(FILTER_SETTLE_MS);           // let the filter settle before going low-power
    writeReg(REG_PWR_MGMT_2, PWR2_20HZ_GYRO_OFF);
    writeReg(REG_PWR_MGMT_1, PWR1_CYCLE_NO_TEMP);
    clearInterrupt();                  // start with INT low
    return true;
  }

  // Reads INT_STATUS (this lowers the latched INT pin). Returns the status
  // byte, or -1 if the chip didn't answer. Bit 6 (0x40) = motion.
  int16_t clearInterrupt() {
    uint8_t v;
    if (!readRegs(REG_INT_STATUS, &v, 1U)) return -1;
    return v;
  }

 private:
  // Registers.
  static constexpr uint8_t REG_ACCEL_CONFIG = 0x1CU;     // bits 4:3 range, bits 2:0 high-pass filter
  static constexpr uint8_t REG_MOT_THR = 0x1FU;
  static constexpr uint8_t REG_MOT_DUR = 0x20U;
  static constexpr uint8_t REG_INT_PIN_CFG = 0x37U;
  static constexpr uint8_t REG_INT_ENABLE = 0x38U;
  static constexpr uint8_t REG_INT_STATUS = 0x3AU;       // bit 6 = "motion happened"; reading it lowers INT
  static constexpr uint8_t REG_ACCEL_XOUT_H = 0x3BU;     // then X_L, Y_H, Y_L, Z_H, Z_L, TEMP_H, TEMP_L
  static constexpr uint8_t REG_MOT_DETECT_CTRL = 0x69U;
  static constexpr uint8_t REG_PWR_MGMT_1 = 0x6BU;       // bit 6 SLEEP: the chip starts ASLEEP after power-up
  static constexpr uint8_t REG_PWR_MGMT_2 = 0x6CU;
  static constexpr uint8_t REG_WHO_AM_I = 0x75U;
  // Values.
  static constexpr uint8_t PWR1_AWAKE = 0x00U;          // SLEEP off, internal 8 MHz clock
  static constexpr uint8_t PWR1_RESET = 0x80U;          // full reset, all registers to defaults
  static constexpr uint8_t PWR1_CYCLE_NO_TEMP = 0x28U;  // CYCLE on (sleep/wake/measure), temperature off
  static constexpr uint8_t PWR2_20HZ_GYRO_OFF = 0x87U;  // bits 7:6 = 2 (wake 20x/s), bits 2:0 gyro X/Y/Z off
  static constexpr uint8_t ACCEL_2G = 0x00U;            // +-2 g: 16384 counts per g
  static constexpr uint8_t ACCEL_2G_HPF_5HZ = 0x01U;    // +-2 g, high-pass 5 Hz: motion = CHANGE, not gravity
  static constexpr uint8_t MOT_DETECT_DEFAULT = 0x15U;  // start-up delay + counter settings
  static constexpr uint8_t INT_HIGH_LATCHED = 0x20U;    // active HIGH, push-pull, latched until INT_STATUS is read
  static constexpr uint8_t INT_MOTION = 0x40U;          // bit 6: motion interrupt only
  static constexpr float COUNTS_PER_G = 16384.0f;
  // Timings.
  static constexpr uint32_t RESET_MS = 100U;
  static constexpr uint32_t WAKE_MS = 10U;
  static constexpr uint32_t FILTER_SETTLE_MS = 50U;

  TwoWire* wire_ = &Wire;  // the default bus, so setupMotionWake() works without begin()

  bool present() {  // an empty write = "are you there?"
    wire_->beginTransmission(ADDRESS);
    return wire_->endTransmission() == 0U;
  }

  bool writeReg(uint8_t reg, uint8_t value) {
    wire_->beginTransmission(ADDRESS);
    wire_->write(reg);
    wire_->write(value);
    return wire_->endTransmission() == 0U;
  }

  uint8_t readReg(uint8_t reg) {
    uint8_t v = 0U;
    readRegs(reg, &v, 1U);
    return v;
  }

  bool readRegs(uint8_t reg, uint8_t* buf, uint8_t n) {
    wire_->beginTransmission(ADDRESS);
    wire_->write(reg);
    // false = "repeated start": keep the bus, turn straight around and read.
    if (wire_->endTransmission(false) != 0U) return false;
    if (wire_->requestFrom(ADDRESS, n) != n) return false;
    for (uint8_t i = 0U; i < n; i++) buf[i] = static_cast<uint8_t>(wire_->read());
    return true;
  }
};
