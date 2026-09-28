#include <Arduino.h>

#include "RgbLed.h"

static const uint32_t PWM_FREQ_HZ = 5000U;
static const uint8_t PWM_BITS = 8U;
static const uint8_t PWM_MAX = 255U;

static const uint8_t CHANNEL_RED = 0U;
static const uint8_t CHANNEL_GREEN = 1U;
static const uint8_t CHANNEL_BLUE = 2U;

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

    off();
}

void RgbLed::setColour(Colour c) {
    writeChannel(CHANNEL_RED, c.red);
    writeChannel(CHANNEL_GREEN, c.green);
    writeChannel(CHANNEL_BLUE, c.blue);
}

void RgbLed::off(void) {
    const Colour black = {0U, 0U, 0U};
    setColour(black);
}

void RgbLed::writeChannel(uint8_t channel, uint8_t level) {
    ledcWrite(channel, (uint32_t)(PWM_MAX - level));
}