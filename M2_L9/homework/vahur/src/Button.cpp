#include <Arduino.h>
#include "Button.h"

Button::Button(uint8_t pin)
:   m_pin(pin),
    m_raw_down(false),
    m_stable_down(false),
    m_last_change_ms(0U) {
}

void Button::begin(void) {
    pinMode(m_pin, INPUT_PULLUP);
}

bool Button::wasPressed(void) {
    const bool is_down = (digitalRead(m_pin) == LOW);
    const uint32_t now = millis();
    bool pressed = false;

    if (is_down != m_raw_down) {
        m_raw_down = is_down;
        m_last_change_ms = now;
    }

    if ((now - m_last_change_ms) >= DEBOUNCE_MS &&
        m_raw_down != m_stable_down) {
        m_stable_down = m_raw_down;
        pressed = m_stable_down;
    }

    return pressed;
}