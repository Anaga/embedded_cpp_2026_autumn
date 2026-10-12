/*
 * # Lesson 12 homework - one Pico, two UARTs
 * Pico sends potentiometer reading to a second Mini.
 */

#include <Arduino.h>
#include <stdint.h>

#include "Potentiometer.h"

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_update_ms = 0U;
static uint16_t g_potentiometer_reading = 0U;

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint32_t UPDATE_PERIOD_MS = 250U;
static const uint32_t UART_BAUD = 115200U;
static const uint8_t PIN_UART_TX = 0U; // GP0, pin 1 -> Mini GPIO20 (RX)
static const uint8_t PIN_UART_RX = 1U; // GP1, pin 2 <- Mini GPIO21 (TX)

// ---------------------------------------------------------------------------
// THE POTENTIOMETER
// ---------------------------------------------------------------------------

// Created here, before setup() runs.
static Potentiometer potentiometer{};

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    Serial1.setTX(PIN_UART_TX);
    Serial1.setRX(PIN_UART_RX);
    Serial1.begin(UART_BAUD);
    delay(1500U);

    potentiometer.begin();

    Serial.println();
    Serial.println("Lesson 12 homework - one Pico, two UARTs");

    Serial.println();
    Serial.println("--- Turn the knob for sending potentiometer reading ---");
}

void loop(void) {
    const uint32_t now = millis();
    if ((now - g_last_update_ms) < UPDATE_PERIOD_MS) {
        return;
    }
    g_last_update_ms = now;

    g_potentiometer_reading = potentiometer.read_potentiometer_10bit();
    Serial1.printf("POT %hu\n", g_potentiometer_reading);
}
