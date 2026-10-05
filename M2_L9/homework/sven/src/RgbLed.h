/*
 * RgbLed.h - an RGB LED with a common anode, one pin per colour.
 *
 * New in lesson 09:
 *   - setColour() takes a const reference: no copy, read only
 *   - a second setColour() takes the three channels directly (overloading)
 */

#pragma once

#include <stdint.h>

#include "Colour.h"

class RgbLed {
public:
    RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin);

    void begin(void);
    void setColour(const Colour &c);
    void setColour(uint8_t red, uint8_t green, uint8_t blue);
    void off(void);

private:
    void writeChannel(uint8_t channel, uint8_t level);

    uint8_t m_red;
    uint8_t m_green;
    uint8_t m_blue;
};
