// Packet layout shared by node and gateway, sent as raw bytes.
// packed: no padding. Both chips are little-endian, so the bytes match.
#pragma once

#include <stdint.h>

constexpr uint8_t MSG_MAGIC = 0x47;  // 'G': filters foreign 434 MHz traffic
constexpr uint8_t MSG_VERSION = 2;

// flags bits
constexpr uint8_t FLAG_AHT_OK = 1 << 0;
constexpr uint8_t FLAG_MPU_OK = 1 << 1;
constexpr uint8_t FLAG_TAMPER = 1 << 2;
constexpr uint8_t FLAG_BUTTON = 1 << 3;

// wake_cause values
constexpr uint8_t WAKE_COLD = 0;
constexpr uint8_t WAKE_TIMER = 1;
constexpr uint8_t WAKE_MOTION = 2;
constexpr uint8_t WAKE_BUTTON = 3;

struct __attribute__((packed)) GuardianMsg {
  uint8_t magic;
  uint8_t version;
  uint8_t flags;        // FLAG_*
  uint16_t seq;         // gaps = lost packets
  uint16_t batt_mV;
  int16_t t_c10;        // temperature x 10
  uint16_t rh_10;       // humidity x 10
  uint16_t awake_ms;    // duration of the previous wake
  uint8_t wake_cause;   // WAKE_*
  char name[9];         // zero-terminated
};
static_assert(sizeof(GuardianMsg) == 23, "GuardianMsg size changed");
