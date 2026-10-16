#include "RgbLed.h"

// Three dedicated LEDC channels, 8-bit resolution at 5 kHz.
// The current arduino-esp32 core is channel based: configure a channel with
// ledcSetup(), bind the pin with ledcAttachPin(), then write the duty cycle
// per channel with ledcWrite().
static const uint8_t CH_R = 0U;
static const uint8_t CH_G = 1U;
static const uint8_t CH_B = 2U;
static const uint32_t PWM_FREQ_HZ = 5000U;
static const uint8_t PWM_RES_BITS = 8U;

RgbLed::RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin)
 : _red_pin(red_pin), _green_pin(green_pin), _blue_pin(blue_pin) {}

void RgbLed::begin() {
 ledcSetup(CH_R, PWM_FREQ_HZ, PWM_RES_BITS);
 ledcSetup(CH_G, PWM_FREQ_HZ, PWM_RES_BITS);
 ledcSetup(CH_B, PWM_FREQ_HZ, PWM_RES_BITS);

 ledcAttachPin(_red_pin, CH_R);
 ledcAttachPin(_green_pin, CH_G);
 ledcAttachPin(_blue_pin, CH_B);

 setColour(0, 0, 0);
}

void RgbLed::setColour(const Colour &c) {
 setColour(c.red, c.green, c.blue);
}

void RgbLed::setColour(uint8_t red, uint8_t green, uint8_t blue) {
 // Inverted logic for a common-anode LED
 ledcWrite(CH_R, 255U - red);
 ledcWrite(CH_G, 255U - green);
 ledcWrite(CH_B, 255U - blue);
}
