// The message the node sends and the gateway receives.
// ONE file, included by both projects (-I ../common), so the two ends
// can never disagree about the layout.
//
// It is sent as raw bytes: the node copies the struct's memory into the
// LoRa packet, the gateway copies the packet back into a struct. That only
// works because:
//   - "packed" removes the padding bytes the compiler would otherwise add
//     to align fields (seq would start at byte 4 instead of byte 3);
//   - both chips (RISC-V C3, Xtensa S3) are little-endian, so a uint16_t
//     has the same byte order on both sides.
// 23 bytes instead of a 60-character text: shorter air time, less battery.
#pragma once

#include <stdint.h>

constexpr uint8_t MSG_MAGIC = 0x47;   // 'G': anything else on 434 MHz is ignored
constexpr uint8_t MSG_VERSION = 2;    // bump when the layout changes

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
  uint8_t magic;        // MSG_MAGIC
  uint8_t version;      // MSG_VERSION
  uint8_t flags;        // FLAG_* bits
  uint16_t seq;         // counts up per message; the gateway finds lost ones from gaps
  uint16_t batt_mV;     // battery in millivolts (12345 = 12.345 V).
  int16_t t_c10;        // temperature x 10 (215 = 21.5 C). Signed: winter!
  uint16_t rh_10;       // relative humidity x 10 (452 = 45.2 %)
  uint16_t awake_ms;    // how long the previous wake took.
  uint8_t wake_cause;   // WAKE_*
  char name[9];         // "fz8", zero-terminated
};
static_assert(sizeof(GuardianMsg) == 23, "GuardianMsg size changed");
