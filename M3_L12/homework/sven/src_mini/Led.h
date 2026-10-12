/*
 * Led.h - a single LED with non-blocking blinking.
 * The class hides the pin, polarity and blink timing from its caller.
 */

#pragma once

#include <stdint.h>

class LedController {
public:
    explicit LedController(uint8_t pin, bool active_low = false);

    void begin(void);
    // Call on every loop iteration. A half-period is one on or off interval.
    void blink(uint32_t half_period_ms);
    // Stop calling blink() while the LED should stay off.
    void off(void);

private:
    void write_state(bool on);

    uint8_t m_pin;
    bool m_active_low;
    bool m_on;
    uint32_t m_last_toggle_ms;
};
