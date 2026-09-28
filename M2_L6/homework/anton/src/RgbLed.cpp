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

 setColour({0, 0, 0});
}

void RgbLed::setColour(Colour c) {
 ledcWrite(_red_pin, 255U - c.red);
 ledcWrite(_green_pin, 255U - c.green);
 ledcWrite(_blue_pin, 255U - c.blue);
}