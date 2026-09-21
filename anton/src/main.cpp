#include <Arduino.h>
#include <stdint.h>

// GPIO Pins
static const uint8_t PIN_POT = 4U;
static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

// Colour Bitmasks (Bit 0: Red, Bit 1: Green, Bit 2: Blue)
static const uint8_t COLOUR_OFF = 0U;
static const uint8_t COLOUR_RED = (1U << 0);
static const uint8_t COLOUR_GREEN = (1U << 1);
static const uint8_t COLOUR_YELLOW = (1U << 0) | (1U << 1);
static const uint8_t COLOUR_BLUE = (1U << 2);
static const uint8_t COLOUR_CYAN = (1U << 1) | (1U << 2);

// Temperature Threshold Boundaries
static const int16_t TEMP_COLD_MAX = 0;
static const int16_t TEMP_COOL_MAX = 14;
static const int16_t TEMP_NORMAL_MAX = 24;
static const int16_t TEMP_WARM_MAX = 34;

// Hysteresis Margin (Star Task)
static const int16_t HYSTERESIS = 1;

uint8_t colourForTemperature(int16_t temp_c) {
 static uint8_t current_colour = COLOUR_OFF;

 if (current_colour == COLOUR_BLUE && temp_c >= (TEMP_COLD_MAX + HYSTERESIS)) {
 current_colour = COLOUR_CYAN;
 } else if (current_colour == COLOUR_CYAN) {
 if (temp_c < TEMP_COLD_MAX) current_colour = COLOUR_BLUE;
 else if (temp_c >= (TEMP_COOL_MAX + HYSTERESIS)) current_colour = COLOUR_GREEN;
 } else if (current_colour == COLOUR_GREEN) {
 if (temp_c < (TEMP_COOL_MAX - HYSTERESIS + 1)) current_colour = COLOUR_CYAN;
 else if (temp_c >= (TEMP_NORMAL_MAX + HYSTERESIS)) current_colour = COLOUR_YELLOW;
 } else if (current_colour == COLOUR_YELLOW) {
 if (temp_c < (TEMP_NORMAL_MAX - HYSTERESIS + 1)) current_colour = COLOUR_GREEN;
 else if (temp_c >= (TEMP_WARM_MAX + HYSTERESIS)) current_colour = COLOUR_RED;
 } else if (current_colour == COLOUR_RED && temp_c < (TEMP_WARM_MAX - HYSTERESIS + 1)) {
 current_colour = COLOUR_YELLOW;
 } else if (current_colour == COLOUR_OFF) {
 if (temp_c < TEMP_COLD_MAX) current_colour = COLOUR_BLUE;
 else if (temp_c <= TEMP_COOL_MAX) current_colour = COLOUR_CYAN;
 else if (temp_c <= TEMP_NORMAL_MAX) current_colour = COLOUR_GREEN;
 else if (temp_c <= TEMP_WARM_MAX) current_colour = COLOUR_YELLOW;
 else current_colour = COLOUR_RED;
 }

 return current_colour;
}

void setColour(uint8_t mask) {
 digitalWrite(PIN_RED, (mask & COLOUR_RED) ? LOW : HIGH);
 digitalWrite(PIN_GREEN, (mask & COLOUR_GREEN) ? LOW : HIGH);
 digitalWrite(PIN_BLUE, (mask & COLOUR_BLUE) ? LOW : HIGH);
}

void setup() {
 Serial.begin(115200);

 pinMode(PIN_RED, OUTPUT);
 pinMode(PIN_GREEN, OUTPUT);
 pinMode(PIN_BLUE, OUTPUT);

 setColour(COLOUR_OFF);
}

void loop() {
 uint16_t raw_adc = analogRead(PIN_POT);
 int16_t temp_c = map(raw_adc, 150, 3800, -40, 140);

 uint8_t colour_mask = colourForTemperature(temp_c);
 setColour(colour_mask);

 Serial.printf("Temp: %3d C | Mask: 0b%c%c%c\n",
  temp_c,
  (colour_mask & COLOUR_BLUE) ? '1' : '0',
  (colour_mask & COLOUR_GREEN) ? '1' : '0',
  (colour_mask & COLOUR_RED) ? '1' : '0');

 delay(200);
}
