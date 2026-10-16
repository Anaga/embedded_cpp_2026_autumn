// Lesson 13 - the AHT10 by hand: readings every 2 s, loop() never blocks.
//
// Wiring (AHT10 -> Pico):
//   VIN -> 3V3 OUT (pin 36)   never VBUS
//   GND -> GND
//   SDA -> GP4 (pin 6)
//   SCL -> GP5 (pin 7)

#include <Arduino.h>
#include <stdint.h>
#include "Aht10.h"

static const uint32_t SERIAL_BAUD = 115200U;
static const uint32_t READ_PERIOD_MS = 2000U;  // one reading every 2 s
static const uint32_t BLINK_HALF_MS = 100U;    // heartbeat LED half-period

static Aht10 aht;

static uint32_t last_reading_ms = 0U;
static uint32_t last_blink_ms = 0U;
static bool led_on = false;
static bool missing_reported = false;

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  aht.begin();
}

void loop() {
  const uint32_t now_ms = millis();

  // Heartbeat: loop() is turning even between readings
  if (now_ms - last_blink_ms >= BLINK_HALF_MS) {
    last_blink_ms = now_ms;
    led_on = !led_on;
    digitalWrite(LED_BUILTIN, led_on ? HIGH : LOW);
  }

  const Aht10Status status = aht.poll();

  if (status == Aht10Status::NO_SENSOR) {
    // The class retries by itself every 2 s; we report the miss once
    if (!missing_reported) {
      missing_reported = true;
      Serial.println("sensor missing");
    }
    return;
  }

  // The sensor answers again: the missing state is over
  missing_reported = false;

  if (status == Aht10Status::BUSY) {
    // Trigger the next measurement, but no more often than every 2 s
    if (now_ms - last_reading_ms >= READ_PERIOD_MS) {
      aht.start();
    }
    return;
  }

  // READY: print the new reading
  last_reading_ms = now_ms;

  const int16_t temp100 = aht.temperatureHundredths();
  const uint16_t rh100 = aht.humidityHundredths();

  int16_t t_whole = temp100 / 100;
  int16_t t_frac = temp100 - t_whole * 100;
  if (t_frac < 0) {
    t_frac = -t_frac;
  }

  Serial.printf("T = %d.%02d C   RH = %u.%02u %%\n",
                (int)t_whole, (int)t_frac,
                (unsigned)(rh100 / 100U), (unsigned)(rh100 % 100U));
}
