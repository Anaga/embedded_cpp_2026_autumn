/*
 * Lesson 10 - homework by sven. See README.md for requirements.
 */

#include <Arduino.h>
#include <stdint.h>

#include "Button.h"
#include "Game.h"
#include "RgbLed.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t PIN_BUTTON_A = 0U;
static const uint8_t PIN_BUTTON_B = 1U;
static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

// ---------------------------------------------------------------------------
// OBJECTS
// ---------------------------------------------------------------------------

static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);
static Button player1(PIN_BUTTON_A);
static Button player2(PIN_BUTTON_B);
static Game game(player1, player2, led);

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    Serial.println();
    Serial.println("The Reaction Game");
    game.begin();
}

void loop(void) {
    game.update();
}
