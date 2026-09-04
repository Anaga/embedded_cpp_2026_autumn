// Lesson 02 - Homework scaffold.
//
// Task: make the LED blink the SOS pattern in a morse style:
//   three short pulses, three long pulses, three short pulses, a pause,
//   then the pattern repeats.
// While the BOOT button is held down, the LED stays off and the
// pattern does not advance.
//
// Full task description and grading notes are in README.md.
// Wiring: nothing to wire - the LED and the button are on the board.
#include <Arduino.h>

// Board pinout - same as the demo.
static const uint8_t LED_PIN = 8U;     // blue status LED, LOW = ON
static const uint8_t BUTTON_PIN = 9U;  // BOOT button, LOW = pressed

// Timing constants for the pattern. Tune them if your rhythm
// does not read as SOS, but keep them named - no magic numbers below.
static const uint32_t SHORT_MS = 150U;
static const uint32_t LONG_MS = 450U;
static const uint32_t GAP_MS = 150U;
static const uint32_t PAUSE_MS = 1000U;

// Turns the LED on for `duration_ms`, then off for GAP_MS.
// Provided so the pattern code reads as a sequence of pulses.
static void pulse(uint32_t duration_ms) {
    digitalWrite(LED_PIN, LOW);   // ON (active low)
    delay(duration_ms);
    digitalWrite(LED_PIN, HIGH);  // OFF
    delay(GAP_MS);
}

// Returns true if BOOT is currently held down.
static bool bootHeld() {
    return digitalRead(BUTTON_PIN) == LOW;
}

void setup() {
    Serial.begin(115200U);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("lesson-02 homework: SOS blinker");
}

void loop() {
    // If BOOT is held, keep the LED off and do not advance the pattern.
    if (bootHeld()) {
        digitalWrite(LED_PIN, HIGH);
        return;
    }

    // Three short pulses.
    for (uint8_t i = 0; i < 3U; i++) {
        if (bootHeld()) { digitalWrite(LED_PIN, HIGH); return; }
        pulse(SHORT_MS);
    }
    // Three long pulses.
    for (uint8_t i = 0; i < 3U; i++) {
        if (bootHeld()) { digitalWrite(LED_PIN, HIGH); return; }
        pulse(LONG_MS);
    }
    // Three short pulses.
    for (uint8_t i = 0; i < 3U; i++) {
        if (bootHeld()) { digitalWrite(LED_PIN, HIGH); return; }
        pulse(SHORT_MS);
    }

    Serial.println("SOS cycle complete");
    delay(PAUSE_MS);
}
