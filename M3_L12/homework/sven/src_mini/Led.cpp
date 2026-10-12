/*
 * Led.cpp - pin control and blink timing, hidden from main.cpp.
 */

#include <Arduino.h>

#include "Led.h"

// The constructor only remembers settings. Hardware is touched in begin().
LedController::LedController(uint8_t pin, bool active_low)
    : m_pin(pin), m_active_low(active_low), m_on(false), m_last_toggle_ms(0U) {
}

void LedController::begin(void) {
    // Set the inactive level before enabling output to avoid a startup flash.
    off();
    pinMode(m_pin, OUTPUT);
}

void LedController::blink(uint32_t half_period_ms) {
    const uint32_t now = millis();
    if ((now - m_last_toggle_ms) < half_period_ms) {
        return;
    }

    m_last_toggle_ms = now;
    write_state(!m_on);
}

void LedController::off(void) {
    write_state(false);
    m_last_toggle_ms = millis();
}

void LedController::write_state(bool on) {
    m_on = on;
    const uint8_t on_level = m_active_low ? LOW : HIGH;
    const uint8_t off_level = m_active_low ? HIGH : LOW;
    digitalWrite(m_pin, on ? on_level : off_level);
}
