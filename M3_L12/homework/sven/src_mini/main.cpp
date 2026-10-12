/*
* # Lesson 12 homework - one Pico, two UARTs
 * Mini reads potentiometer readings from a second Pico.
 */

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "Led.h"
#include "LineReader.h"

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
static const uint8_t PIN_UART_RX = 20U; // GPIO20 <- Pico GP0 (TX), pin 1
static const uint8_t PIN_UART_TX = 21U; // GPIO21 -> Pico GP1 (RX), pin 2

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_status_ms = 0U;
static uint16_t g_potentiometer_reading = POT_MAX;
static LineReader g_uart_line;

// ---------------------------------------------------------------------------
// THE LED
// ---------------------------------------------------------------------------

// Created here, before setup() runs. The constructor only remembers the
// settings; begin() in setup() is where the hardware is touched.
static LedController led(PIN_LED, LED_ACTIVE_LOW);

// Read only bytes that have arrived, so waiting for a line never stops blinking.
static void receive_potentiometer(void) {
    while (g_uart_line.readLine(Serial1)) {
        unsigned int value = 0U;
        char extra;
        if (sscanf(g_uart_line.line(), "POT %u%c", &value, &extra) == 1 && value <= POT_MAX) {
            g_potentiometer_reading = value;
        }
    }
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(SERIAL_BAUD);
    Serial1.begin(SERIAL_BAUD, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);

    led.begin();

    Serial.println();
    Serial.println("Lesson 12 homework - one Pico, two UARTs");

    Serial.println();
    Serial.println("--- Potentiometer controls LED blink speed ---");
}

void loop(void) {
    const uint32_t now = millis();
    receive_potentiometer();

    const uint32_t half_period_ms = MIN_HALF_PERIOD_MS
                                    + g_potentiometer_reading
                                    * (MAX_HALF_PERIOD_MS - MIN_HALF_PERIOD_MS) / POT_MAX;

    // Blink timing must run on every iteration, even between status messages.
    led.blink(half_period_ms);

    if ((now - g_last_status_ms) < STATUS_PERIOD_MS) {
        return;
    }
    g_last_status_ms = now;

    Serial.printf("pot=%hu half=%lu ms\n",
                  g_potentiometer_reading,
                  half_period_ms);
}
