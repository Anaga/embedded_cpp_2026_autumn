#include <Arduino.h>

#include "Potentiometer.h"

static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;
static const uint8_t POT_PIN = 26U; // GP26 / ADC0, physical pin 31
static const uint16_t POT_MIN = 250U;
static const uint16_t POT_MAX = 3350U;
static const uint16_t POT_READING_MAX = 1023U;

Potentiometer::Potentiometer() {
}

uint16_t Potentiometer::read_potentiometer(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t) analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t) (sum / (uint32_t) SAMPLE_COUNT);
}

uint16_t Potentiometer::read_potentiometer_10bit(void) {
    const uint16_t reading = read_potentiometer();
    const uint16_t clamped_reading = constrain(reading, POT_MIN, POT_MAX);
    return (uint16_t) map(clamped_reading, POT_MAX, POT_MIN, 0L, POT_READING_MAX);
}

void Potentiometer::begin(void) {
    analogReadResolution(12);
}
