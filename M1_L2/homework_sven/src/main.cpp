// Lesson 02 - Homework scaffold.
//
// Task: make the LED blink the SOS pattern in a morse style:
//   three short pulses, three long pulses, three short pulses, a pause,
//   then the pattern repeats.
// While the BOOT button is held down, the LED stays off and the
// pattern does not advance.
//
// Definition of done
// - The rhythm is readable as SOS from across the room.
// - Holding BOOT stops the LED; releasing it resumes the pattern.
// - One line is printed over Serial per completed cycle.
// - No magic numbers inside `loop()` - timing lives in the named
//   constants at the top.
//
// Full task description and grading notes are in README.md.
// Wiring: nothing to wire - the LED and the button are on the board.

#include <Arduino.h>

// Board pinout - same as the demo.
static const uint8_t LED_PIN = 8U; // blue status LED, LOW = ON
static const uint8_t BUTTON_PIN = 9U; // BOOT button, LOW = pressed

// Timing constants for the pattern. Tune them if your rhythm
// does not read as SOS, but keep them named - no magic numbers below.
static const uint32_t SHORT_MS = 150U;
static const uint32_t LONG_MS = 450U;
static const uint32_t GAP_MS = 150U;
static const uint32_t PAUSE_MS = 1000U;

// Turns the LED on for `duration_ms`, then off for GAP_MS.
// Provided so the pattern code reads as a sequence of pulses.
static void pulse(uint32_t duration_ms) {
    digitalWrite(LED_PIN, LOW); // ON (active low)
    delay(duration_ms);
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(GAP_MS);
}

void setup() {
    Serial.begin(115200U);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("lesson-02 homework from sven: SOS blinker");
}

void loop() {
    // TODO: if the BOOT button is held down, keep the LED off and return
    //       from loop() without playing the pattern.

    // play one full SOS cycle using pulse():
    // Pulse S - three short pulses
    pulse(SHORT_MS);
    pulse(SHORT_MS);
    pulse(SHORT_MS);
    // Pulse O - three long pulses
    pulse(LONG_MS);
    pulse(LONG_MS);
    pulse(LONG_MS);
    // Pulse S - three short pulses
    pulse(SHORT_MS);
    pulse(SHORT_MS);
    pulse(SHORT_MS);

    // after the cycle, print one line over Serial and wait
    // PAUSE_MS before the pattern repeats.
    Serial.println("SOS");
    delay(PAUSE_MS);
}
