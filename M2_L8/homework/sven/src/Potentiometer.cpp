#include <Arduino.h>

#include "Potentiometer.h"

static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;
static const uint8_t POT_PIN = 4U;

Potentiometer::Potentiometer() {
}

uint16_t Potentiometer::readPotentiometer(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t) analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t) (sum / (uint32_t) SAMPLE_COUNT);
}

uint16_t Potentiometer::readPotentiometerAsRing(void) {

}

void Potentiometer::begin(void) {
    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);
}
