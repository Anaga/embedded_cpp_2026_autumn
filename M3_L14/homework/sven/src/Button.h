/**
 * Button.h - one push button between a pin and GND, debounced.
 *
 * Each Button remembers its own previous state and debounce timer.
 */

#pragma once

#include <stdint.h>

class Button {
public:
    Button(uint8_t pin);

    void begin(void);

    /* True exactly once per press: not while held, not on bounce. */
    bool wasPressed(void);

private:
    uint8_t m_pin;
    uint8_t m_previous;
    uint32_t m_last_change_ms;
};
