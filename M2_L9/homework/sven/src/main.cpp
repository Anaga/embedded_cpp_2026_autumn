/*
 * Lesson 09 - homework by sven. See README.md for requirements.
 */

#include <Arduino.h>
#include <stdint.h>

#include "Button.h"
#include "Colour.h"
#include "RgbLed.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t PIN_BUTTON_A = 0U;
static const uint8_t PIN_BUTTON_B = 1U;
static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

static const Colour PLAYER_1_COLOUR = {255U, 0U, 0U}; // red
static const Colour PLAYER_2_COLOUR = {0U, 0U, 255U}; // blue

// ---------------------------------------------------------------------------
// OBJECTS
// ---------------------------------------------------------------------------

static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);
static Button player1(PIN_BUTTON_A);
static Button player2(PIN_BUTTON_B);

// ---------------------------------------------------------------------------
// BUTTON PRESS TRACKING
// ---------------------------------------------------------------------------

static uint16_t g_presses_a = 0U;
static uint16_t g_presses_b = 0U;

/* Reference parameters: the function updates the caller's counter. */
static void recordPress(const char *who, uint16_t &counter, const Colour &colour) {
    counter = (uint16_t) (counter + 1U);
    led.setColour(colour);
    Serial.printf("%s pressed   A: %u   B: %u\n",
                  who, (unsigned) g_presses_a, (unsigned) g_presses_b);
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    led.begin();
    player1.begin(); // sets INPUT_PULLUP, needed in both modes
    player2.begin();

    Serial.println();
    Serial.println("The Reaction Game");
}

void loop(void) {
    bool pressed_a = false;
    bool pressed_b = false;

    pressed_a = player1.wasPressed();
    pressed_b = player2.wasPressed();

    if (pressed_a) {
        recordPress("A", g_presses_a, PLAYER_1_COLOUR);
    }
    if (pressed_b) {
        recordPress("B", g_presses_b, PLAYER_2_COLOUR);
    }
}
