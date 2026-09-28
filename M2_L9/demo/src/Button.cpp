/*
 * Button.cpp - debounced push button.
 */

#include <Arduino.h>

#include "Button.h"

// How long the pin must stay quiet before a new press is believed.
static const uint32_t DEBOUNCE_MS = 30U;

Button::Button(uint8_t pin)
    : m_pin(pin), m_previous(HIGH), m_last_change_ms(0U) {
}

void Button::begin(void) {
    pinMode(m_pin, INPUT_PULLUP);
}

/*
 * A press only counts if the pin had been quiet for DEBOUNCE_MS before it.
 * Every change restarts the quiet time, so the bounces after a press and the
 * bounces of a release are all ignored.
 *
 * Compare with lesson 07: m_previous and m_last_change_ms used to be
 * static locals. There was one set of them for the whole program.
 * Now there is one set per object.
 */
bool Button::wasPressed(void) {
    const uint8_t current = (uint8_t)digitalRead(m_pin);
    const uint32_t now = millis();
    bool pressed = false;

    if (current != m_previous) {
        if ((current == LOW) && ((now - m_last_change_ms) >= DEBOUNCE_MS)) {
            pressed = true;
        }
        m_last_change_ms = now;
        m_previous = current;
    }

    return pressed;
}
