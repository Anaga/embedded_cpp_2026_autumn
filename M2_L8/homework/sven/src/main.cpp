/*
 * Homework: Lesson 08: colour wheel.
 * See README.md for more.
 */

#include <Arduino.h>
#include <stdint.h>

#include "Colour.h"
#include "Potentiometer.h"
#include "RgbLed.h"

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_update_ms = 0U;

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

static const uint32_t UPDATE_PERIOD_MS = 250U;

// ---------------------------------------------------------------------------
// THE POTENTIOMETER
// ---------------------------------------------------------------------------

// Created here, before setup() runs.
static Potentiometer potentiometer{};

// ---------------------------------------------------------------------------
// THE LED
// ---------------------------------------------------------------------------

// Created here, before setup() runs. The constructor only remembers the
// pins; begin() in setup() is where the hardware is touched.
static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);

// ---------------------------------------------------------------------------
// HELPERS
// ---------------------------------------------------------------------------

/* -> reaches a field through a pointer. c->red is the same as (*c).red. */
static void printColour(const char *label, const Colour *c) {
    Serial.printf("  %-10s  red %3u   green %3u   blue %3u\n",
                  label,
                  (unsigned) c->red,
                  (unsigned) c->green,
                  (unsigned) c->blue);
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    led.begin();

    Serial.println();
    Serial.println("Homework: Lesson 08: colour wheel");

    Serial.println();
    Serial.println("--- Turn the knob for color wheel ---");
}

void loop(void) {
    const uint32_t now = millis();
    if ((now - g_last_update_ms) < UPDATE_PERIOD_MS) {
        return;
    }
    g_last_update_ms = now;

    const uint16_t potentiometerReading = potentiometer.readPotentiometer();
    Serial.printf("potentiometerReading = %+4d\n", (int) potentiometerReading);
    const uint16_t potentiometerReadingAsRing = potentiometer.readPotentiometerAsRing();

    const Colour hueColor = led.hueToColour(potentiometerReadingAsRing);
    printColour("Hue colors: ", &hueColor);
    led.setColour(hueColor);
}
