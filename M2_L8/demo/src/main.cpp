/*
 * Lesson 08 - instructor demo
 * Structs, Classes and PWM
 *
 * At startup: what a struct copy does, then a walk through a palette of
 * named colours, then one slow fade. After that the palette repeats.
 *
 * Wiring, RGB LED with a common anode:
 *   longest pin -> 3V3
 *   red         -> GPIO 5, through 100 ohm
 *   green       -> GPIO 6, through 100 ohm
 *   blue        -> GPIO 7, through 100 ohm
 *
 * Serial monitor: 115200
 *
 * To see the class pay off: open src/RgbLed.cpp, set USE_PWM to false,
 * upload. The LED falls back to eight colours. This file is not touched.
 */

#include <Arduino.h>
#include <stdint.h>

#include "Colour.h"
#include "RgbLed.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

static const uint32_t PALETTE_STEP_MS = 1000U;
static const uint32_t FADE_STEP_MS = 4U;
static const uint8_t FADE_MAX = 255U;

// ---------------------------------------------------------------------------
// COLOURS
// ---------------------------------------------------------------------------

static const Colour RED = { 255U, 0U, 0U };
static const Colour ORANGE = { 255U, 120U, 0U };
static const Colour YELLOW = { 255U, 255U, 0U };
static const Colour GREEN = { 0U, 255U, 0U };
static const Colour CYAN = { 0U, 255U, 255U };
static const Colour BLUE = { 0U, 0U, 255U };
static const Colour MAGENTA = { 255U, 0U, 255U };
static const Colour WHITE = { 255U, 255U, 255U };
static const Colour DIM_WHITE = { 40U, 40U, 40U };

/* A struct can hold another struct. An array can hold structs. */
struct NamedColour {
    const char *name;
    Colour colour;
};

static const NamedColour PALETTE[] = {
    { "red", RED },
    { "orange", ORANGE },
    { "yellow", YELLOW },
    { "green", GREEN },
    { "cyan", CYAN },
    { "blue", BLUE },
    { "magenta", MAGENTA },
    { "white", WHITE },
    { "dim white", DIM_WHITE },
};

static const uint8_t PALETTE_COUNT = (uint8_t)(sizeof(PALETTE) / sizeof(PALETTE[0]));

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
                  (unsigned)c->red,
                  (unsigned)c->green,
                  (unsigned)c->blue);
}

// ---------------------------------------------------------------------------
// PART 1: STRUCTS
// ---------------------------------------------------------------------------

static void demoStructs(void) {
    Serial.println();
    Serial.println("--- 1. A struct is a value ---");

    const Colour warm = { 255U, 120U, 0U };
    printColour("warm", &warm);
    Serial.printf("  sizeof(Colour) = %u bytes\n", (unsigned)sizeof(Colour));

    Colour a = { 10U, 20U, 30U };
    Colour b = a;
    b.red = 99U;
    Serial.println("  b = a, then b.red = 99:");
    printColour("a", &a);
    printColour("b", &b);
    Serial.println("  a did not change: b is a copy, not a second name for a.");
}

// ---------------------------------------------------------------------------
// PART 2: THE CLASS AT WORK
// ---------------------------------------------------------------------------

static void demoPalette(void) {
    Serial.println();
    Serial.println("--- 2. One object, many colours ---");

    for (uint8_t i = 0U; i < PALETTE_COUNT; i++) {
        printColour(PALETTE[i].name, &PALETTE[i].colour);
        led.setColour(PALETTE[i].colour);
        delay(PALETTE_STEP_MS);
    }
    led.off();
}

// ---------------------------------------------------------------------------
// PART 3: PWM
// ---------------------------------------------------------------------------

/* Red from dark to full and back. With USE_PWM false this becomes a
 * single hard switch halfway up. */
static void demoFade(void) {
    Serial.println();
    Serial.println("--- 3. PWM: red, dark to full and back ---");

    Colour c = { 0U, 0U, 0U };

    for (uint16_t level = 0U; level <= FADE_MAX; level++) {
        c.red = (uint8_t)level;
        led.setColour(c);
        delay(FADE_STEP_MS);
    }
    for (int16_t level = FADE_MAX; level >= 0; level--) {
        c.red = (uint8_t)level;
        led.setColour(c);
        delay(FADE_STEP_MS);
    }
    led.off();
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    led.begin();

    Serial.println();
    Serial.println("Lesson 08 - Structs, Classes and PWM");

    demoStructs();
    demoPalette();
    demoFade();

    Serial.println();
    Serial.println("--- Palette, round and round ---");
}

void loop(void) {
    static uint32_t last_step_ms = 0U;
    static uint8_t index = 0U;

    const uint32_t now = millis();
    if ((now - last_step_ms) < PALETTE_STEP_MS) {
        return;
    }
    last_step_ms = now;

    printColour(PALETTE[index].name, &PALETTE[index].colour);
    led.setColour(PALETTE[index].colour);

    index = (uint8_t)(index + 1U);
    if (index >= PALETTE_COUNT) {
        index = 0U;
    }
}
