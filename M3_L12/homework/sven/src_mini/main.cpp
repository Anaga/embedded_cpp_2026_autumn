/*
* # Lesson 12 homework - one Pico, two UARTs
 * Mini reads potentiometer readings from a second Pico.
 */

#include <Arduino.h>
#include <stdint.h>

#include "Led.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

// Reuse the former red channel of the common-anode LED.
static const uint8_t PIN_LED = 5U;
static const bool LED_ACTIVE_LOW = true;

static const uint16_t POT_MAX = 1023U;
static const uint32_t MIN_HALF_PERIOD_MS = 50U;
static const uint32_t MAX_HALF_PERIOD_MS = 500U;
static const uint32_t STATUS_PERIOD_MS = 1000U;
static const uint32_t SERIAL_BAUD = 115200U;

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_status_ms = 0U;

// ---------------------------------------------------------------------------
// THE LED
// ---------------------------------------------------------------------------

// Created here, before setup() runs. The constructor only remembers the
// settings; begin() in setup() is where the hardware is touched.
static LedController led(PIN_LED, LED_ACTIVE_LOW);

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(SERIAL_BAUD);

    led.begin();

    Serial.println();
    Serial.println("Lesson 12 homework - one Pico, two UARTs");

    Serial.println();
    Serial.println("--- Potentiometer controls LED blink speed ---");
}

void loop(void) {
    const uint32_t now = millis();

    // Placeholder until UART receives a potentiometer reading in 0..1023.
    const uint16_t potentiometer_reading = 1023;
    const uint32_t half_period_ms = MIN_HALF_PERIOD_MS
                                    + (uint32_t) potentiometer_reading
                                    * (MAX_HALF_PERIOD_MS - MIN_HALF_PERIOD_MS) / POT_MAX;

    // Blink timing must run on every iteration, even between status messages.
    led.blink(half_period_ms);

    if ((now - g_last_status_ms) < STATUS_PERIOD_MS) {
        return;
    }
    g_last_status_ms = now;

    Serial.printf("pot=%u half=%lu ms\n",
                  potentiometer_reading,
                  half_period_ms);
}
