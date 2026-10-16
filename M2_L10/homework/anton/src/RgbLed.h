#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>
#include <stdint.h>

struct Colour {
 uint8_t red;
 uint8_t green;
 uint8_t blue;
};

class RgbLed {
public:
 RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin);
 void begin();
 void setColour(const Colour &c);
 void setColour(uint8_t red, uint8_t green, uint8_t blue);

private:
 uint8_t _red_pin;
 uint8_t _green_pin;
 uint8_t _blue_pin;
};

#endif // RGB_LED_H