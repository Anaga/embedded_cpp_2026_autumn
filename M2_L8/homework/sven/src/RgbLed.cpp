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

static const uint32_t PWM_FREQ_HZ = 5000U;
static const uint8_t PWM_BITS = 8U;
static const uint8_t PWM_MAX = 255U;

// PWM is produced by channels, not by pins. The ESP32-C3 has six; the class
// takes the first three and connects one pin to each. Nothing outside this
// file ever hears about channels.
// One RgbLed object per program: a second one would claim the same channels.
static const uint8_t CHANNEL_RED = 0U;
static const uint8_t CHANNEL_GREEN = 1U;
static const uint8_t CHANNEL_BLUE = 2U;

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
    ledcSetup(CHANNEL_RED, PWM_FREQ_HZ, PWM_BITS);
    ledcSetup(CHANNEL_GREEN, PWM_FREQ_HZ, PWM_BITS);
    ledcSetup(CHANNEL_BLUE, PWM_FREQ_HZ, PWM_BITS);

    ledcAttachPin(m_red, CHANNEL_RED);
    ledcAttachPin(m_green, CHANNEL_GREEN);
    ledcAttachPin(m_blue, CHANNEL_BLUE);

    // A freshly set up channel starts at duty 0, and on a common anode LED
    // duty 0 is full brightness. Switch off at once, or the LED flashes white
    // every time the board starts.
    off();
}

void RgbLed::setColour(Colour c) {
    writeChannel(m_red, CHANNEL_RED, c.red);
    writeChannel(m_green, CHANNEL_GREEN, c.green);
    writeChannel(m_blue, CHANNEL_BLUE, c.blue);
}

void RgbLed::off(void) {
    const Colour black = {0U, 0U, 0U};
    setColour(black);
}

Colour RgbLed::hueToColour(uint16_t hue) {
    // Secure input to max 360.
    hue = constrain(hue, 0, 360U);
    const uint8_t sixSlices = 60U;

    const uint8_t slice = hue / sixSlices;
    const uint8_t position = hue % sixSlices;
    const uint8_t rising = (uint16_t) position * PWM_MAX / (sixSlices - 1);
    const uint8_t falling = PWM_MAX - rising;

    switch (slice) {
        case 0U:
            return {PWM_MAX, rising, 0U};
        case 1U:
            return {falling, PWM_MAX, 0U};
        case 2U:
            return {0U, PWM_MAX, rising};
        case 3U:
            return {0U, falling, PWM_MAX};
        case 4U:
            return {rising, 0U, PWM_MAX};
        default:
            return {PWM_MAX, 0U, falling};
    }
}

// ---------------------------------------------------------------------------
// PRIVATE
// ---------------------------------------------------------------------------

/* The only place in the whole program that knows the LED is inverted. */
void RgbLed::writeChannel(uint8_t pin, uint8_t channel, uint8_t level) {
    ledcWrite(channel, (uint32_t) (PWM_MAX - level));
}
