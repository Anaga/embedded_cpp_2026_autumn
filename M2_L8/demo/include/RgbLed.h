/*
 * RgbLed.h - an RGB LED with a common anode, one pin per colour.
 *
 * The class hides two details from everyone who uses it:
 *   - how a channel is driven: plain on/off, or PWM
 *   - that the logic is inverted: on a common anode LED, LOW or duty 0
 *     means full brightness
 *
 * Callers only ever say what colour they want.
 */

#pragma once

#include <stdint.h>

#include "Colour.h"

class RgbLed {
public:
    RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin);

    void begin(void);
    void setColour(Colour c);
    void off(void);

private:
    void writeChannel(uint8_t pin, uint8_t level);

    uint8_t m_red;
    uint8_t m_green;
    uint8_t m_blue;
};
