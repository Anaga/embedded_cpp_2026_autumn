#pragma once

#include <stdint.h>
#include "Colour.h"

class RgbLed {
public:
    RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin);
    void begin(void);
    void setColour(const Colour &colour);
    void setColour(uint8_t red, uint8_t green, uint8_t blue);

private:
    uint8_t m_red_pin;
    uint8_t m_green_pin;
    uint8_t m_blue_pin;
};
