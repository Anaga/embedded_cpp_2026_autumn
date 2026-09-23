/*
 * RgbLed.cpp - how the LED is actually driven.
 *
 * Everything in this file is private business of the class. main.cpp never
 * sees any of it.
 */

#include <Arduino.h>

#include "RgbLed.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

// Set this to false and upload: the same class now drives the LED with plain
// digitalWrite. Eight colours instead of millions, and main.cpp does not
// change by a single character. That is the point of the class.
static const bool USE_PWM = true;

static const uint32_t PWM_FREQ_HZ = 5000U;
static const uint8_t PWM_BITS = 8U;
static const uint8_t PWM_MAX = 255U;

// In on/off mode a channel counts as on from half brightness upwards.
static const uint8_t DIGITAL_THRESHOLD = 128U;

// ---------------------------------------------------------------------------
// PUBLIC
// ---------------------------------------------------------------------------

/* Remember the pins. Do not touch the hardware yet: an object created
 * outside any function exists before the chip is ready for it. */
RgbLed::RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin)
    : m_red(red_pin), m_green(green_pin), m_blue(blue_pin) {
}

/* Touch the hardware. Called from setup(), when the chip is ready. */
void RgbLed::begin(void) {
    if (USE_PWM) {
        ledcAttach(m_red, PWM_FREQ_HZ, PWM_BITS);
        ledcAttach(m_green, PWM_FREQ_HZ, PWM_BITS);
        ledcAttach(m_blue, PWM_FREQ_HZ, PWM_BITS);
    } else {
        pinMode(m_red, OUTPUT);
        pinMode(m_green, OUTPUT);
        pinMode(m_blue, OUTPUT);
    }

    // A freshly attached channel starts at duty 0, and on a common anode LED
    // duty 0 is full brightness. Switch off at once, or the LED flashes white
    // every time the board starts.
    off();
}

void RgbLed::setColour(Colour c) {
    writeChannel(m_red, c.red);
    writeChannel(m_green, c.green);
    writeChannel(m_blue, c.blue);
}

void RgbLed::off(void) {
    const Colour black = { 0U, 0U, 0U };
    setColour(black);
}

// ---------------------------------------------------------------------------
// PRIVATE
// ---------------------------------------------------------------------------

/* The only place in the whole program that knows the LED is inverted. */
void RgbLed::writeChannel(uint8_t pin, uint8_t level) {
    if (USE_PWM) {
        ledcWrite(pin, (uint32_t)(PWM_MAX - level));
    } else {
        digitalWrite(pin, (level >= DIGITAL_THRESHOLD) ? LOW : HIGH);
    }
}
