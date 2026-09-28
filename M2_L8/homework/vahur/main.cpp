#include <Arduino.h>
#include <stdint.h>

#include "Colour.h"
#include "RgbLed.h"

static const uint8_t POT_PIN = 4U;

static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;

static const uint16_t ADC_MIN_COUNTS = 0U;
static const uint16_t ADC_MAX_COUNTS = 3850U;

static const uint16_t HUE_MIN = 0U;
static const uint16_t HUE_MAX = 359U;

static const uint16_t HUE_SLICE_SIZE = 60U;
static const uint16_t COLOUR_MAX = 255U;

static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);

static uint16_t readPotentiometer(void) {
    uint32_t sum = 0U;

    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t)analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }

    return (uint16_t)(sum / (uint32_t)SAMPLE_COUNT);
}

static uint16_t mapToHue(uint16_t counts) {
    if (counts > ADC_MAX_COUNTS) {
        counts = ADC_MAX_COUNTS;
    }

    const uint32_t offset =
        (uint32_t)counts - (uint32_t)ADC_MIN_COUNTS;

    const uint32_t span =
        (uint32_t)ADC_MAX_COUNTS - (uint32_t)ADC_MIN_COUNTS;

    const uint32_t hue_range =
        (uint32_t)HUE_MAX - (uint32_t)HUE_MIN;

    return (uint16_t)(
        (uint32_t)HUE_MIN +
        (offset * hue_range) / span
    );
}

static Colour hueToColour(uint16_t hue) {
    const uint16_t slice = hue / HUE_SLICE_SIZE;
    const uint16_t position = hue % HUE_SLICE_SIZE;

    const uint16_t rising =
        (position * COLOUR_MAX) / HUE_SLICE_SIZE;

    const uint16_t falling =
        COLOUR_MAX - rising;

    Colour colour = {0U, 0U, 0U};

    switch (slice) {
        case 0U:
            colour.red = 255U;
            colour.green = (uint8_t)rising;
            break;

        case 1U:
            colour.red = (uint8_t)falling;
            colour.green = 255U;
            break;

        case 2U:
            colour.green = 255U;
            colour.blue = (uint8_t)rising;
            break;

        case 3U:
            colour.green = (uint8_t)falling;
            colour.blue = 255U;
            break;

        case 4U:
            colour.red = (uint8_t)rising;
            colour.blue = 255U;
            break;

        default:
            colour.red = 255U;
            colour.blue = (uint8_t)falling;
            break;
    }

    return colour;
}

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    led.begin();

    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println();
    Serial.println("Lesson 08 - Colour Wheel");
}

void loop(void) {
    static uint16_t previous_hue = 360U;

    const uint16_t counts = readPotentiometer();
    const uint16_t hue = mapToHue(counts);

    if (hue == previous_hue) {
        return;
    }

    previous_hue = hue;

    const Colour colour = hueToColour(hue);

    led.setColour(colour);

    Serial.printf(
        "Hue %3u   R %3u   G %3u   B %3u\n",
        (unsigned)hue,
        (unsigned)colour.red,
        (unsigned)colour.green,
        (unsigned)colour.blue
    );
}