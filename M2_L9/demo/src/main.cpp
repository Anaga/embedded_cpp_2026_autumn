/*
 * Lesson 09 - instructor demo
 * References, Overloading and a Button Class
 *
 * At startup: references, then overloading. After that: two buttons, each
 * with a press counter, read in one of two ways.
 *
 * Wiring:
 *   button A (player 1) -> GPIO 0 and GND
 *   button B (player 2) -> GPIO 1 and GND
 *   RGB LED, common anode -> 3V3; red GPIO 5, green GPIO 6, blue GPIO 7,
 *                            each through 100 ohm
 *
 * Serial monitor: 115200
 *
 * HOW TO RUN THE BUTTON PART IN CLASS
 *
 *   1. BUTTON_MODE = MODE_SHARED_STATIC
 *        One function with static variables, called for both pins.
 *        Press A alone, press B alone: both seem to work.
 *        Now hold A down and press B. B is never counted.
 *
 *   2. BUTTON_MODE = MODE_OBJECTS
 *        Two Button objects. Hold A, press B: B counts.
 *        Every object has its own memory.
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

static const uint8_t MODE_SHARED_STATIC = 0U;
static const uint8_t MODE_OBJECTS = 1U;

// Change this in class: MODE_SHARED_STATIC first, then MODE_OBJECTS.
static const uint8_t BUTTON_MODE = MODE_SHARED_STATIC;

static const uint32_t DEBOUNCE_MS = 30U;
static const uint32_t SHOW_MS = 1200U;

static const Colour PLAYER_1_COLOUR = { 255U, 0U, 0U };   // red
static const Colour PLAYER_2_COLOUR = { 0U, 0U, 255U };   // blue

// ---------------------------------------------------------------------------
// OBJECTS
// ---------------------------------------------------------------------------

static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);
static Button player1(PIN_BUTTON_A);
static Button player2(PIN_BUTTON_B);

// ---------------------------------------------------------------------------
// PART 1: REFERENCES
// ---------------------------------------------------------------------------

static void addOneToCopy(uint8_t value) {
    value = (uint8_t)(value + 1U);   // changes the copy only
    (void)value;                     // and the copy is thrown away here
}

static void addOneThroughPointer(uint8_t *value) {
    *value = (uint8_t)(*value + 1U);
}

static void addOneThroughReference(uint8_t &value) {
    value = (uint8_t)(value + 1U);   // no * needed: value IS the original
}

static void demoReferences(void) {
    Serial.println();
    Serial.println("--- 1. References ---");

    uint8_t count = 5U;
    uint8_t &alias = count;
    alias = 7U;
    Serial.printf("count = 5, alias = count, alias = 7  ->  count is %u\n",
                  (unsigned)count);

    uint8_t a = 5U;
    uint8_t b = 5U;
    uint8_t c = 5U;
    addOneToCopy(a);
    addOneThroughPointer(&b);
    addOneThroughReference(c);
    Serial.println("All three start at 5, each function adds one:");
    Serial.printf("  by copy       f(a)   ->  %u\n", (unsigned)a);
    Serial.printf("  by pointer    f(&b)  ->  %u\n", (unsigned)b);
    Serial.printf("  by reference  f(c)   ->  %u\n", (unsigned)c);
    Serial.println("The reference call looks like the copy, and works like the pointer.");
}

// ---------------------------------------------------------------------------
// PART 2: OVERLOADING
// ---------------------------------------------------------------------------

static void demoOverloading(void) {
    Serial.println();
    Serial.println("--- 2. Overloading: two setColour, one name ---");

    const Colour warm = { 255U, 120U, 0U };

    Serial.println("  led.setColour(warm)             a Colour, by const reference");
    led.setColour(warm);
    delay(SHOW_MS);

    Serial.println("  led.setColour(0U, 255U, 255U)   three numbers");
    led.setColour(0U, 255U, 255U);
    delay(SHOW_MS);

    led.off();
    Serial.println("The compiler picked the version by the arguments.");
}

// ---------------------------------------------------------------------------
// PART 3: TWO BUTTONS
// ---------------------------------------------------------------------------

/*
 * The obvious way, and the wrong one: the debounce from lesson 07, now with
 * a pin parameter. It compiles and looks fine. But the two statics belong to
 * the FUNCTION, so both buttons share one previous state and one timer.
 */
static bool isButtonPressedShared(uint8_t pin) {
    static uint8_t previous = HIGH;
    static uint32_t last_change_ms = 0U;

    const uint8_t current = (uint8_t)digitalRead(pin);
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

static uint16_t g_presses_a = 0U;
static uint16_t g_presses_b = 0U;

/* Reference parameters: the function updates the caller's counter. */
static void recordPress(const char *who, uint16_t &counter, const Colour &colour) {
    counter = (uint16_t)(counter + 1U);
    led.setColour(colour);
    Serial.printf("%s pressed   A: %u   B: %u\n",
                  who, (unsigned)g_presses_a, (unsigned)g_presses_b);
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    led.begin();
    player1.begin();   // sets INPUT_PULLUP, needed in both modes
    player2.begin();

    Serial.println();
    Serial.println("Lesson 09 - References, Overloading and a Button Class");

    demoReferences();
    demoOverloading();

    Serial.println();
    Serial.println("--- 3. Two buttons ---");
    if (BUTTON_MODE == MODE_SHARED_STATIC) {
        Serial.println("Mode: one function, shared static variables");
        Serial.println("Try: hold A down, then press B.");
    } else {
        Serial.println("Mode: two Button objects");
        Serial.println("Try the same: hold A down, then press B.");
    }
}

void loop(void) {
    bool pressed_a = false;
    bool pressed_b = false;

    if (BUTTON_MODE == MODE_SHARED_STATIC) {
        pressed_a = isButtonPressedShared(PIN_BUTTON_A);
        pressed_b = isButtonPressedShared(PIN_BUTTON_B);
    } else {
        pressed_a = player1.wasPressed();
        pressed_b = player2.wasPressed();
    }

    if (pressed_a) {
        recordPress("A", g_presses_a, PLAYER_1_COLOUR);
    }
    if (pressed_b) {
        recordPress("B", g_presses_b, PLAYER_2_COLOUR);
    }
}
