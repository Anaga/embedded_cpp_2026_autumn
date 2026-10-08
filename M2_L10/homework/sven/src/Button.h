/*
 * Button.h - one push button between a pin and GND, debounced.
 *
 * The same logic as isButtonPressed() in lesson 07, with one difference:
 * the memory that used to live in two static variables now lives in the
 * object. Every Button remembers its own previous state and its own timer,
 * so any number of buttons can work side by side.
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
