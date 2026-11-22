// Lesson 24 demo - AFTER: one class per part, loop() is a list
//
// The same device as src/before/:
//   - AHT10 reading every 2 s, chip temperature every 2 s
//   - button A toggles the green LED
//   - blue heartbeat, 4 blinks a second while loop() is alive
//   - Telegram: /status, /led on | off | toggle
//   - WiFi networks from NVS, the strongest one wins (type 'help')
//
// Rules:
//   - a part never waits: it checks the clock, does a little, returns
//   - each part: begin() once from setup(), update(now) every pass
//   - WiFi and the bot must wait, so they run in their own task (net.cpp)
//   - loop() owns the pins; the net task sends commands (shared.h)
//   - LoopStats prints the longest loop pass every 5 s

#include <Arduino.h>
#include <WiFi.h>

#include "board.h"
#include "rgb_led.h"
#include "heartbeat.h"
#include "button.h"
#include "aht10_sensor.h"
#include "chip_temp.h"
#include "console.h"
#include "shared.h"
#include "net.h"
#include "../common/loop_stats.h"

constexpr uint32_t SNAPSHOT_PERIOD_MS = 100U;

static void printStatus();

static RgbLed      rgb;
static Heartbeat   heartbeat(rgb);
static Button      button(PIN_BUTTON_A);
static Aht10Sensor sensor;
static ChipTemp    chipTemp;
static Console     console(printStatus);
static LoopStats   stats;

static bool     ledOn        = false;
static uint32_t lastSnapshot = 0U;

// ---------------------------------------------------------------- glue
static void ledSet(bool on) {
  ledOn = on;
  rgb.set(RgbLed::Channel::GREEN, on);
}

static void ledToggle() {
  ledSet(!ledOn);
  Serial.printf("LED %s (button)\n", ledOn ? "on" : "off");
}

// Commands the bot task has queued for us. loop() owns the LED pins.
static void ledCommands() {
  shared::LedCmd cmd;
  while (shared::receiveLedCmd(cmd)) {
    switch (cmd) {
      case shared::LedCmd::ON:     ledSet(true);   break;
      case shared::LedCmd::OFF:    ledSet(false);  break;
      case shared::LedCmd::TOGGLE: ledSet(!ledOn); break;
    }
    Serial.printf("LED %s (bot)\n", ledOn ? "on" : "off");
  }
}

// A copy of what /status needs, for the bot task.
static void publishSnapshot() {
  uint32_t now = millis();
  if (now - lastSnapshot < SNAPSHOT_PERIOD_MS) return;
  lastSnapshot = now;
  shared::publish({sensor.valid(), sensor.temperature(), sensor.humidity(),
                   chipTemp.celsius(), ledOn, stats.longestMs()});
}

static void printStatus() {
  Serial.printf("%s, up %lu min\n", shared::deviceId(),
                static_cast<unsigned long>(millis() / 60000UL));
  if (sensor.valid()) {
    Serial.printf("AHT10 %.1f C, %.1f %% RH\n", sensor.temperature(), sensor.humidity());
  } else {
    Serial.println("AHT10 missing");
  }
  Serial.printf("chip %.1f C\n", chipTemp.celsius());
  Serial.printf("LED %s\n", ledOn ? "on" : "off");
  if (WiFi.isConnected()) {
    Serial.printf("WiFi \"%s\", %d dBm\n", WiFi.SSID().c_str(), WiFi.RSSI());
  } else {
    Serial.println("WiFi not connected");
  }
  Serial.printf("longest loop pass %lu ms (last 5 s period)\n",
                static_cast<unsigned long>(stats.longestMs()));
}

// ---------------------------------------------------------------- main
void setup() {
  Serial.begin(115200);
  delay(2000);                       // USB CDC needs a moment after reset
  Serial.println();
  Serial.println("Lesson 24 - AFTER: one class per part");

  rgb.begin();
  heartbeat.begin();
  button.begin();
  sensor.begin();
  chipTemp.begin();
  console.begin();
  shared::begin();
  Serial.printf("Device %s\n", shared::deviceId());
  net::begin();                      // WiFi + bot: own task
  stats.begin();
}

void loop() {
  uint32_t now = millis();
  button.update(now);
  if (button.pressed()) ledToggle();
  ledCommands();                     // from the bot task
  sensor.update(now);
  chipTemp.update(now);
  heartbeat.update(now);
  console.update();
  publishSnapshot();                 // for the bot task
  stats.update(now);
}
