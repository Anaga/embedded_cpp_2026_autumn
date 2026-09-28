/*
 * RgbLed.cpp - PWM on three LEDC channels, common anode.
 *
 * Arduino-ESP32 core 2.x, as installed by PlatformIO: PWM is produced by
 * channels. ledcSetup() configures a channel, ledcAttachPin() connects a pin
 * to it, ledcWrite() sets the channel's duty.
 */

#include <Arduino.h>

#include "RgbLed.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint32_t PWM_FREQ_HZ = 5000U;
static const uint8_t PWM_BITS = 8U;
static const uint8_t PWM_MAX = 255U;

// The class takes the first three of the six channels.
// One RgbLed object per program: a second one would claim the same channels.
static const uint8_t CHANNEL_RED = 0U;
static const uint8_t CHANNEL_GREEN = 1U;
static const uint8_t CHANNEL_BLUE = 2U;

// ---------------------------------------------------------------------------
// PUBLIC
// ---------------------------------------------------------------------------

RgbLed::RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin)
    : m_red(red_pin), m_green(green_pin), m_blue(blue_pin) {
}

void RgbLed::begin(void) {
    ledcSetup(CHANNEL_RED, PWM_FREQ_HZ, PWM_BITS);
    ledcSetup(CHANNEL_GREEN, PWM_FREQ_HZ, PWM_BITS);
    ledcSetup(CHANNEL_BLUE, PWM_FREQ_HZ, PWM_BITS);

    ledcAttachPin(m_red, CHANNEL_RED);
    ledcAttachPin(m_green, CHANNEL_GREEN);
    ledcAttachPin(m_blue, CHANNEL_BLUE);

    // A fresh channel sits at duty 0: full brightness on a common anode LED.
    off();
}

/* The version that does the real work. const &: no copy, read only. */
void RgbLed::setColour(const Colour &c) {
    writeChannel(CHANNEL_RED, c.red);
    writeChannel(CHANNEL_GREEN, c.green);
    writeChannel(CHANNEL_BLUE, c.blue);
}

/* The overload only translates, then hands over to the version above. */
void RgbLed::setColour(uint8_t red, uint8_t green, uint8_t blue) {
    const Colour c = { red, green, blue };
    setColour(c);
}

void RgbLed::off(void) {
    setColour(0U, 0U, 0U);
}

// ---------------------------------------------------------------------------
// PRIVATE
// ---------------------------------------------------------------------------

/* The only place in the program that knows the LED is inverted. */
void RgbLed::writeChannel(uint8_t channel, uint8_t level) {
    ledcWrite(channel, (uint32_t)(PWM_MAX - level));
}
