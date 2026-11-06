// Lesson 19 - Lab 3: the message a node sends to the gateway over ESP-NOW.
//
// The SAME file is in lab3-node/include and lab3-gateway/include.
// Change one, change both - byte for byte. The static_assert below stops
// the build if the layout changes on one side only.
#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr uint8_t MSG_VERSION   = 1;
constexpr uint8_t MSG_FLAG_SENSOR_OK = 0x01;
constexpr size_t  MSG_NAME_LEN  = 9;     // "esp-xxxx" + '\0'

struct __attribute__((packed)) NodeMsg {
  uint8_t  version;              // MSG_VERSION
  uint8_t  flags;                // bit 0: sensor ok
  uint16_t seq;                  // counts every send, survives deep sleep
  int16_t  t_c10;                // temperature x 10 (215 = 21.5 C)
  uint16_t rh_10;                // relative humidity x 10 (402 = 40.2 %)
  uint16_t awake_ms;             // how long the PREVIOUS wake took
  char     name[MSG_NAME_LEN];   // "esp-xxxx"
};

static_assert(sizeof(NodeMsg) == 19, "NodeMsg layout changed - update both sides");
