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
    uint8_t m_red;
    uint8_t m_green;
    uint8_t m_blue;
};
