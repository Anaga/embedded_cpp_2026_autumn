#include "RgbLed.h"

RgbLed::RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin)
 : _red_pin(red_pin), _green_pin(green_pin), _blue_pin(blue_pin) {}

void RgbLed::begin() {
 pinMode(_red_pin, OUTPUT);
 pinMode(_green_pin, OUTPUT);
 pinMode(_blue_pin, OUTPUT);

 ledcAttach(_red_pin, 5000, 8);
 ledcAttach(_green_pin, 5000, 8);
 ledcAttach(_blue_pin, 5000, 8);

 setColour(0, 0, 0);
}

void RgbLed::setColour(const Colour &c) {
 setColour(c.red, c.green, c.blue);
}

void RgbLed::setColour(uint8_t red, uint8_t green, uint8_t blue) {
 // Inverted logic for Common Anode LED
 ledcWrite(_red_pin, 255U - red);
 ledcWrite(_green_pin, 255U - green);
 ledcWrite(_blue_pin, 255U - blue);
}