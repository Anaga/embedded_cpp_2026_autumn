#pragma once

#include <stdint.h>

class Button {
public:
    Button(uint8_t pin);

    void begin(void);
    bool wasPressed(void);

private:
    const uint8_t m_button_pin;
    uint8_t m_previous;
    uint32_t m_last_change_ms;
};