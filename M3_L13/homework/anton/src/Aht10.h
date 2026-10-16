// Aht10 - the AHT10 temperature/humidity sensor by hand, on Wire.
//
// The AHT10 is a command-based device: no register pointer, no repeated
// start. One transaction triggers a measurement, another one reads the
// result 80 ms later. This class keeps that as a state machine so poll()
// never blocks: it does at most one small I2C transaction per call.
//
// Protocol (7-bit address 0x38):
//   init:     write 0xE1 0x08 0x00, then read 1 status byte
//   reset:    write 0xBA
//   trigger:  write 0xAC 0x33 0x00, wait >= 80 ms
//   read:     read 6 bytes: status, b1..b5
//
// Results are kept in integer hundredths (the RP2040 has no FPU):
//   RH % = raw / 2^20 * 100,  T degC = raw / 2^20 * 200 - 50

#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>

enum class Aht10Status {
  BUSY,       // initializing or measuring: try again later
  READY,      // a new reading is available, use the getters
  NO_SENSOR   // the AHT10 does not answer (NACK): retry in 2 s
};

class Aht10 {
public:
  // Start Wire and kick off the initialization (non-blocking: the real
  // transactions happen in poll()).
  void begin();

  // Trigger a measurement. Ignored while a measurement or an
  // initialization is still in progress.
  void start();

  // Advance the internal state machine. Call from loop(), never block
  // around it.
  Aht10Status poll();

  // Valid after poll() returned READY.
  int16_t temperatureHundredths() const;  // degrees C * 100
  uint16_t humidityHundredths() const;    // percent * 100

private:
  enum class State {
    INIT_WRITE,   // send 0xE1 0x08 0x00
    INIT_WAIT,    // wait a bit, then read the status byte
    IDLE,         // sensor ready, waiting for start()
    MEASURING,    // trigger sent, waiting for >= 80 ms
    RETRY_LATER   // sensor missing: wait, then init again
  };

  static const uint8_t ADDR = 0x38U;

  static const uint32_t INIT_SETTLE_MS = 20U;   // after the init command
  static const uint32_t MEASURE_WAIT_MS = 80U;  // AHT10 needs >= 80 ms
  static const uint32_t RETRY_PERIOD_MS = 2000U;  // missing sensor: 2 s

  bool writeCommand(uint8_t b1, uint8_t b2, uint8_t b3);
  bool readResult(uint8_t *status, uint8_t *bytes);
  void unpack(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5);

  State m_state;
  uint32_t m_state_ms;  // when the current state started

  int16_t m_temp_c100;
  uint16_t m_rh_pct100;
};
