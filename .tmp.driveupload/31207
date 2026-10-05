#include <Arduino.h>
#include "RgbLed.h"

RgbLed::RgbLed(uint8_t red_pin, uint8_t green_pin, uint8_t blue_pin)
    : m_red_pin(red_pin),
      m_green_pin(green_pin),
      m_blue_pin(blue_pin)
{
}

void RgbLed::begin(void)
{
    pinMode(m_red_pin, OUTPUT);
    pinMode(m_green_pin, OUTPUT);
    pinMode(m_blue_pin, OUTPUT);

    setColour(0U, 0U, 0U);
}

void RgbLed::setColour(const Colour &colour)
{
    setColour(colour.red, colour.green, colour.blue);
}

void RgbLed::setColour(uint8_t red, uint8_t green, uint8_t blue)
{
    analogWrite(m_red_pin, 255U - red);
    analogWrite(m_green_pin, 255U - green);
    analogWrite(m_blue_pin, 255U - blue);
}