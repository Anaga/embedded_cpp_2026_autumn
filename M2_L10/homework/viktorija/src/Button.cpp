#include <Arduino.h>

#include "Button.h"

// How long the pin must stay quiet before a new press is believed.
static const uint32_t DEBOUNCE_MS = 30U;

Button::Button(uint8_t pin)
    : m_button_pin(pin), m_previous(HIGH), m_last_change_ms(0U) {
}

void Button::begin(void) {
    pinMode(m_button_pin, INPUT_PULLUP);
}

bool Button::wasPressed(void) {
    const uint8_t current = (uint8_t)digitalRead(m_button_pin);
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
