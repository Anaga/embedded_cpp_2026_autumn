#include <Arduino.h>
#include <stdint.h>
#include "RgbLed.h"

static const uint8_t PIN_POT = 4U;
static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

static const uint16_t ADC_MIN = 150U;
static const uint16_t ADC_MAX = 3800U;

static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);

Colour hueToColour(uint16_t hue) {
 if (hue >= 360U) {
 hue = 359U;
 }

 uint8_t slice = hue / 60U;
 uint8_t offset = hue % 60U;

 uint8_t rising = (255U * offset) / 60U;
 uint8_t falling = 255U - rising;

 Colour c = {0, 0, 0};

 switch (slice) {
 case 0:
 c.red = 255;
 c.green = rising;
 c.blue = 0;
 break;
 case 1:
 c.red = falling;
 c.green = 255;
 c.blue = 0;
 break;
 case 2:
 c.red = 0;
 c.green = 255;
 c.blue = rising;
 break;
 case 3:
 c.red = 0;
 c.green = falling;
 c.blue = 255;
 break;
 case 4:
 c.red = rising;
 c.green = 0;
 c.blue = 255;
 break;
 case 5:
 c.red = 255;
 c.green = 0;
 c.blue = falling;
 break;
 default:
 break;
 }

 return c;
}

void setup() {
 Serial.begin(115200);
 led.begin();
}

void loop() {
 static int16_t last_hue = -1;

 uint16_t raw_adc = analogRead(PIN_POT);

 if (raw_adc < ADC_MIN) raw_adc = ADC_MIN;
 if (raw_adc > ADC_MAX) raw_adc = ADC_MAX;

 uint16_t hue = map(raw_adc, ADC_MIN, ADC_MAX, 0, 359);

 if ((int16_t)hue != last_hue) {
 last_hue = hue;

 Colour c = hueToColour(hue);
 led.setColour(c);

 Serial.printf("Hue: %3d | R: %3d, G: %3d, B: %3d\n", hue, c.red, c.green, c.blue);
 }

 delay(20);
}