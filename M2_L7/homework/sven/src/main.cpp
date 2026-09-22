/*
 * Lesson 07 - Home Task: The Multiplication Table. See README.md.
 */

#include <Arduino.h>
#include <stdint.h>
#include <string.h>

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t BUTTON_PIN = 0U;

static const uint8_t MODE_DEBOUNCED = 2U;

// How long the pin must stay quiet before a new press is believed.
static const uint32_t DEBOUNCE_MS = 30U;

// The counter is printed at most this often, so the monitor stays readable.
static const uint32_t PRINT_PERIOD_MS = 100U;

// ---------------------------------------------------------------------------
// THE BUTTON
// ---------------------------------------------------------------------------

/*
 * a press only counts if the pin had been quiet for DEBOUNCE_MS
 * before it. Every change restarts the quiet time, so the bounces that follow
 * a press, and the bounces of a release, are all ignored.
 */
static bool isButtonPressed(void) {
    static uint8_t previous = HIGH;
    static uint32_t last_change_ms = 0U;

    const uint8_t current = (uint8_t) digitalRead(BUTTON_PIN);
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

static uint32_t g_presses = 0U;
static uint32_t g_last_printed = 0U;
static uint32_t g_last_print_ms = 0U;

void setup(void) {
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    delay(1500U * 3); // My serial needed extra time to initialize.
    Serial.println();
    Serial.println("You can press the button now.");
}

void loop(void) {
    // No delay here on purpose: the loop runs as fast as it can, which is
    // exactly what lets it see every bounce.
    if (isButtonPressed()) {
        g_presses++;
    }

    const uint32_t now = millis();
    if ((now - g_last_print_ms) < PRINT_PERIOD_MS) {
        return;
    }
    g_last_print_ms = now;

    if (g_presses != g_last_printed) {
        Serial.printf("presses = %lu   (+%lu)\n",
                      (unsigned long) g_presses,
                      (unsigned long) (g_presses - g_last_printed));
        g_last_printed = g_presses;
    }
}
