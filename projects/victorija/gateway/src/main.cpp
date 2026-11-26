// Winter Guardian - the GATEWAY (ESP32-S3, by the window).
//
// Receives the node's LoRa packets, raises alarms, keeps a history, shows
// the most important problem on the on-board LED, and reports to a
// Telegram bot (status card, buttons, alerts that say what to do).
//
// Built the lesson 24 way:
//   - one class per part, each with begin() once and update(now) every pass
//   - a part never waits: it checks the clock, does a little, returns
//   - WiFi and Telegram must wait, so they run in their own task (net.cpp)
//   - loop() owns the radio, the alarms and the LED; the net task owns WiFi
//     and the bot. They share only what shared.h says.
//
// The parts, how often they run, and who runs them:
//
//   part         file            how often                         who
//   receiver     receiver.cpp    every pass: "a packet there?"     loop()
//   alarms       alarms.cpp      every packet + time check 1 s     loop()
//   history      history.cpp     every packet                      loop() adds, net task reads
//   status LED   status_led.cpp  every pass, writes on a change    loop()
//   console      console.cpp     every pass, one line at a time    loop()
//   loop stats   loop_stats.h    every pass, prints once a minute  loop()
//   WiFi         net.cpp         when disconnected: every 5 s      net task (core 0)
//   Telegram     net.cpp         every 2 s, 0.3-2 s each           net task (core 0)
//
// All tunable numbers (and the DEMO values) are in config.h.

#include <Arduino.h>

#include "alarms.h"
#include "config.h"
#include "console.h"  // lesson 24: serial commands (wifi add/list/del, reboot)
#include "history.h"
#include "loop_stats.h"
#include "net.h"
#include "receiver.h"
#include "shared.h"
#include "status_led.h"

constexpr uint32_t SERIAL_SETTLE_MS = 500U;  // the CP2102 USB-serial chip needs a moment after reset
constexpr uint32_t HALT_BLINK_MS = 1000U;

static void printStatus();

static Alarms alarms;
static History history;
static StatusLed led(alarms);
static Receiver receiver(alarms, history, led);
static Console console(printStatus);
static LoopStats stats;

// The console's "status" command.
static void printStatus() {
  net::printStatus();
  receiver.printStatus();
  Serial.printf("longest loop pass %lu ms (last minute)\n", static_cast<unsigned long>(stats.longestMs()));
}

void setup() {
  Serial.begin(115200);
  delay(SERIAL_SETTLE_MS);
  Serial.println("\n=== Winter Guardian - gateway (S3) ===");

  shared::begin();
  led.begin();
  alarms.begin();
  history.begin();
  if (!receiver.begin()) {
    Serial.println("LoRa NOT FOUND - check the wiring (run lora-check)");
    led.showError();  // steady red = error
    for (;;) delay(HALT_BLINK_MS);
  }
  console.begin();
  net::begin(alarms, history);  // WiFi + bot: own task
  stats.begin();
}

void loop() {
  uint32_t now = millis();
  receiver.update(now);
  alarms.update(now);
  led.update(now);
  console.update();
  stats.update(now);
}
