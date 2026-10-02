#pragma once

#include <stdint.h>

class Button {
public:
    Button(uint8_t pin);
    void begin(void);
    bool wasPressed(void);

private:
    uint8_t m_pin;
    bool m_raw_down;
    bool m_stable_down;
    uint32_t m_last_change_ms;
    static const uint32_t DEBOUNCE_MS = 20U;
};