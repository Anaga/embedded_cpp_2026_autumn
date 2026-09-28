/*
 * Lesson 08 - homework
 * The colour wheel with the star
 */

#include <Arduino.h>
#include <stdint.h>

#include "Colour.h"
#include "RgbLed.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t POT_PIN = 4U;
static const uint8_t LED_RED_PIN = 5U;
static const uint8_t LED_GREEN_PIN = 6U;
static const uint8_t LED_BLUE_PIN = 7U;
static const uint8_t BUTTON_PIN = 0U;

static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;
// Measured around 3450 in lesson 05, a bit less so the top always reaches 359.
static const uint16_t ADC_MAX_COUNTS = 3400U;

static const uint32_t DEBOUNCE_MS = 30U;

static const uint32_t UPDATE_PERIOD_MS = 10U;

static const uint16_t HUE_COUNT = 360U;
static const uint16_t HUE_MAX = HUE_COUNT - 1U;
static const uint16_t SLICE_DEGREES = 60U;
static const uint8_t CHANNEL_MAX = 255U;

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static RgbLed led(LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);

static uint32_t g_last_update_ms = 0U;
static bool g_spin_mode = false;
// Starts outside 0 - 359, so the first hue always gets printed.
static uint16_t g_hue = HUE_COUNT;

// ---------------------------------------------------------------------------
// KNOB
// ---------------------------------------------------------------------------

// From lesson 06
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
    return (uint16_t)(((uint32_t)counts * HUE_MAX) / ADC_MAX_COUNTS);
}

// ---------------------------------------------------------------------------
// COLOUR WHEEL
// ---------------------------------------------------------------------------

Colour hueToColour(uint16_t hue) {
    const uint16_t slice = hue / SLICE_DEGREES;
    const uint16_t position = hue % SLICE_DEGREES;

    const uint8_t rise = (uint8_t)((position * CHANNEL_MAX) / SLICE_DEGREES);
    const uint8_t fall = (uint8_t)(CHANNEL_MAX - rise);

    switch (slice) {
        case 0U: return { CHANNEL_MAX, rise, 0U };  // red -> yellow
        case 1U: return { fall, CHANNEL_MAX, 0U };  // yellow -> green
        case 2U: return { 0U, CHANNEL_MAX, rise };  // green -> cyan
        case 3U: return { 0U, fall, CHANNEL_MAX };  // cyan -> blue
        case 4U: return { rise, 0U, CHANNEL_MAX };  // blue -> magenta
        default: return { CHANNEL_MAX, 0U, fall };  // magenta -> red
    }
}

// ---------------------------------------------------------------------------
// BUTTON
// ---------------------------------------------------------------------------

// From lesson 07
static bool isButtonPressed(void) {
    static uint8_t previous = HIGH;
    static uint32_t last_change_ms = 0U;

    const uint8_t current = (uint8_t)digitalRead(BUTTON_PIN);
    const uint32_t now = millis();
    bool pressed = false;

    if (current != previous) {
        if ((current == LOW) && ((now - last_change_ms) >= DEBOUNCE_MS)) {
            pressed = true;
        }
        last_change_ms = now;
        previous = current;
    }

    return pressed;
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    led.begin();

    Serial.println();
    Serial.println("Lesson 08 homework - the colour wheel");
    Serial.println("Turn the knob. Press the button to switch knob / spin mode.");
}

void loop(void) {
    if (isButtonPressed()) {
        g_spin_mode = !g_spin_mode;
        Serial.println(g_spin_mode ? "Mode: spin" : "Mode: knob");
    }

    const uint32_t now = millis();
    if ((now - g_last_update_ms) < UPDATE_PERIOD_MS) {
        return;
    }
    g_last_update_ms = now;

    uint16_t hue;
    if (g_spin_mode) {
        hue = (uint16_t)((g_hue + 1U) % HUE_COUNT);
    } else {
        hue = mapToHue(readPotentiometer());
    }

    if (hue == g_hue) {
        return;
    }
    g_hue = hue;

    const Colour c = hueToColour(hue);
    led.setColour(c);

    Serial.printf("hue %3u   red %3u   green %3u   blue %3u\n",
                  (unsigned)hue,
                  (unsigned)c.red,
                  (unsigned)c.green,
                  (unsigned)c.blue);
}
