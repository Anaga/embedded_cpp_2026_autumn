/*
 * PISI demonstration - ESP32-C3 Super Mini.
 * Derived from user's main(4).cpp, with classes and demonstration timings.
 * Pump: ON 30s, OFF 90s. Grow LEDs: ON 120s, OFF 60s (10s fades).
 * RGB: fault red 200ms blink; idle green 100ms / 1000ms;
 * lights only green; pump only blue; both alternate blue/green 200ms.
 * Requires Arduino-ESP32 core 2.x, Adafruit AHTX0 library.
 */
#include <Arduino.h>
#include "Config.h"
#include "PumpController.h"
#include "TemperatureMonitor.h"
#include "LightController.h"
#include "RgbLed.h"
#include "StatusIndicator.h"
#include "Potentiometer.h"
#include "LoopStats.h"

static PumpController pump;
static TemperatureMonitor climate;
static LightController lights;
static Potentiometer knob;
static RgbLed rgb;
static StatusIndicator indicator(rgb);
static LoopStats stats;
static uint32_t lastStatusMs = 0U;

static void printStatus(uint32_t now) {
  Serial.printf("[%lu s] pump=%s pumpPWM=%lu%% grow=%s growPWM=%lu%% AHT10=%s T=%.1fC RH=%.1f%% heatPause=%s fault=%s knob=%lumV\n",
    (unsigned long)(now / 1000UL), pump.running()?"ON":"OFF",
    (unsigned long)pump.dutyPercent(), lights.on()?"ON":"OFF",
    (unsigned long)lights.dutyPercent(), climate.ok()?"OK":"ERROR",
    climate.temperatureC(), climate.humidity(), climate.thermalPause()?"YES":"NO",
    pump.fault()?"YES":"NO", (unsigned long)knob.millivolts());
  if (cfg::LIGHT_SENSOR_INSTALLED) {
    Serial.printf("light sensor DO=%s\n", digitalRead(cfg::PIN_LIGHT_SENSOR_DO)?"HIGH":"LOW");
  }
}

static void readCommands(uint32_t now) {
  // Limit work each pass so a long incoming serial stream cannot stall the loop.
  uint8_t count = 0U;
  while (Serial.available() > 0 && count++ < 12U) {
    switch ((char)Serial.read()) {
      case 'w': pump.startManual(now); break;
      case 'p': pump.stopManual(now); break;
      case 's': printStatus(now); break;
      case 'h': Serial.println("Commands: w=water now, p=stop pump, s=status, h=help"); break;
      case '\r': case '\n': break;
      default: Serial.println("Unknown command; type h"); break;
    }
  }
}

void setup() {
  // Force the two load-control pins LOW before any sensor or serial work.
  digitalWrite(cfg::PIN_PUMP, LOW);
  digitalWrite(cfg::PIN_LIGHT, LOW);
  pinMode(cfg::PIN_PUMP, OUTPUT);
  pinMode(cfg::PIN_LIGHT, OUTPUT);
  rgb.begin();
  indicator.begin();

  Serial.begin(cfg::SERIAL_BAUD);
  // No delay(1500) and no while(!Serial): standalone cold start works.
  Serial.println("PISI demo: pump 30 ON / 90 OFF, lights 120 ON / 60 OFF");
  Serial.println("RGB: fault RED; idle GREEN heartbeat; light GREEN; pump BLUE; both alternate");
  Serial.println("PWM pump ramp remains enabled; verify operation with water");

  knob.begin();
  if (cfg::LIGHT_SENSOR_INSTALLED) pinMode(cfg::PIN_LIGHT_SENSOR_DO, INPUT);
  const uint32_t now = millis();
  climate.begin(now);
  lights.begin(now);
  pump.begin(now); // pump starts at t=0
  stats.begin(now);
  lastStatusMs = now;
}

void loop() {
  const uint32_t now = millis();
  readCommands(now);
  knob.update(now);
  climate.update(now);
  pump.update(now);
  lights.update(now, climate.ok(), climate.thermalPause(), pump.fault());
  const bool fault = pump.fault() || !climate.ok() || climate.thermalPause();
  indicator.update(now, fault, pump.running(), lights.on());
  if ((uint32_t)(now - lastStatusMs) >= cfg::STATUS_PERIOD_MS) {
    lastStatusMs = now;
    printStatus(now);
  }
  stats.update(now);
}
