// MPU-6050 driver without a library: accelerometer read and motion-wake
// interrupt. Registers per the MPU-6050 register map rev 4.2.
#pragma once

#include <Arduino.h>
#include <Wire.h>

class Mpu6050 {
 public:
  static constexpr uint8_t ADDRESS = 0x68;  // AD0 low

  uint8_t whoAmI = 0;

  bool begin(TwoWire& wire = Wire) {
    wire_ = &wire;
    wire_->beginTransmission(ADDRESS);
    if (wire_->endTransmission() != 0) return false;

    whoAmI = readReg(0x75);
    writeReg(0x6B, 0x00);  // PWR_MGMT_1: wake (starts asleep after power-up)
    delay(10);
    writeReg(0x1C, 0x00);  // ACCEL_CONFIG: +-2 g
    return true;
  }

  bool read(float& x, float& y, float& z, float& tempC) {
    uint8_t d[8];
    if (!readRegs(0x3B, d, sizeof d)) return false;
    int16_t rx = (int16_t)((d[0] << 8) | d[1]);  // big-endian
    int16_t ry = (int16_t)((d[2] << 8) | d[3]);
    int16_t rz = (int16_t)((d[4] << 8) | d[5]);
    int16_t rt = (int16_t)((d[6] << 8) | d[7]);
    x = rx / 16384.0f;
    y = ry / 16384.0f;
    z = rz / 16384.0f;
    tempC = rt / 340.0f + 36.53f;
    return true;
  }

  // Motion detection in low-power cycle mode (~70 uA). INT goes high and
  // stays latched until INT_STATUS is read. Settings survive the C3's deep
  // sleep, so call this once after power-up.
  bool setupMotionWake(uint8_t threshold, uint8_t durationMs) {
    wire_->beginTransmission(ADDRESS);
    if (wire_->endTransmission() != 0) return false;

    writeReg(0x6B, 0x80);  // PWR_MGMT_1: reset
    delay(100);
    writeReg(0x6B, 0x00);
    delay(10);
    writeReg(0x1C, 0x01);  // ACCEL_CONFIG: +-2 g, high-pass 5 Hz (ignore gravity)
    writeReg(0x1F, threshold);   // MOT_THR
    writeReg(0x20, durationMs);  // MOT_DUR
    writeReg(0x69, 0x15);  // MOT_DETECT_CTRL
    writeReg(0x37, 0x20);  // INT_PIN_CFG: active high, push-pull, latched
    writeReg(0x38, 0x40);  // INT_ENABLE: motion only
    delay(50);             // let the filter settle
    writeReg(0x6C, 0x87);  // PWR_MGMT_2: 20 Hz wake-ups, gyro off
    writeReg(0x6B, 0x28);  // PWR_MGMT_1: cycle mode, temp sensor off
    clearInterrupt();
    return true;
  }

  // Reading INT_STATUS releases the latched INT pin. -1 = no answer.
  int clearInterrupt() {
    uint8_t v;
    if (!readRegs(0x3A, &v, 1)) return -1;
    return v;
  }

 private:
  TwoWire* wire_ = &Wire;  // non-null: setupMotionWake() may run without begin()

  bool writeReg(uint8_t reg, uint8_t value) {
    wire_->beginTransmission(ADDRESS);
    wire_->write(reg);
    wire_->write(value);
    return wire_->endTransmission() == 0;
  }

  uint8_t readReg(uint8_t reg) {
    uint8_t v = 0;
    readRegs(reg, &v, 1);
    return v;
  }

  bool readRegs(uint8_t reg, uint8_t* buf, uint8_t n) {
    wire_->beginTransmission(ADDRESS);
    wire_->write(reg);
    if (wire_->endTransmission(false) != 0) return false;  // repeated start
    if (wire_->requestFrom(ADDRESS, n) != n) return false;
    for (uint8_t i = 0; i < n; i++) buf[i] = wire_->read();
    return true;
  }
};
