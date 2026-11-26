// What loop() and the net task share - and the only way they share it.
//
//   loop() -> net task:  the latest reading, copied under a lock (Latest)
//   loop() -> net task:  alerts, through a FreeRTOS queue
//   net task -> loop():  two flags: online, mute
//
// One owner per thing: loop() owns the radio, the LED and the alarms;
// the net task owns WiFi and the bot.
#pragma once
#include <Arduino.h>
#include <time.h>

#include "msg.h"  // from ../common

namespace shared {

// The latest packet and its reception, as one block.
struct Latest {
  bool have = false;  // false until the first packet arrives
  GuardianMsg msg = {};
  int16_t rssi = 0;
  float snr = 0.0f;
  uint32_t atMs = 0U;     // millis() when it arrived
  time_t atTime = 0;      // clock time when it arrived (0 before NTP)
  uint32_t received = 0U;
  uint32_t lost = 0U;
};

void begin();  // from setup(), before anything else uses it

// loop() writes it, the net task copies it. The lock is held only for the copy.
void publish(const Latest& l);
Latest read();

// An alert's text (Telegram HTML). UTF-8: an emoji is 4 bytes.
struct Alert {
  char text[320];
};
void queueAlert(const char* text);  // loop(): drops it if the queue is full
bool peekAlert(Alert& a);           // net task: look at the oldest, leave it in
void dropAlert();                   // net task: it was sent, take it out
uint8_t alertsWaiting();

// Written only by the net task, read by loop() for the LED: true while
// messages actually reach Telegram. A bool is written in one step: no lock.
extern volatile bool online;

// Movement alerts are muted until this millis() (0 = not muted). Written by
// the net task (the Mute button), read by loop(). One 32-bit write: no lock.
extern volatile uint32_t muteUntilMs;
bool muted();

}  // namespace shared
