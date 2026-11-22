// What loop() and the net task share - and the only way they share it.
//
//   net task -> loop():  LED commands through a FreeRTOS queue
//   loop() -> net task:  a Snapshot copied under a critical section
//
// One owner per thing: loop() owns the pins, the net task owns WiFi and
// the bot. The bot never calls digitalWrite(); it sends a command.
#pragma once
#include <stdint.h>

namespace shared {

enum class LedCmd : uint8_t { ON, OFF, TOGGLE };

// Everything /status needs, copied as one block.
struct Snapshot {
  bool     sensorValid;
  float    tempC;
  float    rhPct;
  float    chipC;                    // ESP32-C3 die temperature
  bool     ledOn;
  uint32_t longestPassMs;
};

void begin();                        // from setup(), before net::begin()

// "esp-xxxx" from the last two bytes of the WiFi MAC address: tells the
// boards apart in /status, on Serial and as the WiFi host name.
const char *deviceId();

bool sendLedCmd(LedCmd cmd);         // net task: false if the queue is full
bool receiveLedCmd(LedCmd &cmd);     // loop(): false if there is nothing

void publish(const Snapshot &s);     // loop()
Snapshot read();                     // net task

}  // namespace shared
