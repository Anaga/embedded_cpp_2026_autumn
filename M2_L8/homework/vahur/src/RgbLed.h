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
    void writeChannel(uint8_t channel, uint8_t level);

    uint8_t m_red;
    uint8_t m_green;
    uint8_t m_blue;
};