/*
 * Lesson 10 - instructor demo
 * States, Templates and Interrupts
 *
 * At startup: two templates at work. After that, three things run side by
 * side in loop(), none of them with delay():
 *
 *   1. A traffic light, as a state machine with enum class.
 *      Button A is the pedestrian request.
 *   2. The potentiometer, raw and smoothed by a moving average in a
 *      RingBuffer, printed once a second.
 *   3. Button B, read twice: once by a debounced Button object, once by an
 *      interrupt that counts every falling edge it sees.
 *
 * Wiring:
 *   button A -> GPIO 0 and GND
 *   button B -> GPIO 1 and GND
 *   potentiometer middle pin -> GPIO 4; outer pins -> 3V3 (through 3.3k), GND
 *   RGB LED, common anode -> 3V3; red GPIO 5, green GPIO 6, blue GPIO 7,
 *                            each through 100 ohm
 *
 * Serial monitor: 115200
 */

#include <Arduino.h>
#include <stdint.h>

#include "Button.h"
#include "Colour.h"
#include "RgbLed.h"
#include "RingBuffer.h"

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t PIN_BUTTON_A = 0U;
static const uint8_t PIN_BUTTON_B = 1U;
static const uint8_t PIN_POT = 4U;
static const uint8_t PIN_RED = 5U;
static const uint8_t PIN_GREEN = 6U;
static const uint8_t PIN_BLUE = 7U;

// Traffic light timing.
static const uint32_t GREEN_MIN_MS = 4000U;    // green stays at least this long
static const uint32_t YELLOW_MS = 1500U;
static const uint32_t RED_MS = 4000U;
static const uint32_t RED_YELLOW_MS = 1000U;

// Potentiometer smoothing.
static const uint8_t SMOOTH_SAMPLES = 16U;
static const uint32_t SAMPLE_PERIOD_MS = 20U;
static const uint32_t POT_PRINT_PERIOD_MS = 1000U;

static const Colour COLOUR_GREEN = { 0U, 255U, 0U };
static const Colour COLOUR_YELLOW = { 255U, 160U, 0U };
static const Colour COLOUR_RED = { 255U, 0U, 0U };
static const Colour COLOUR_RED_YELLOW = { 255U, 60U, 0U };

// ---------------------------------------------------------------------------
// OBJECTS
// ---------------------------------------------------------------------------

static RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);
static Button pedestrian(PIN_BUTTON_A);
static Button buttonB(PIN_BUTTON_B);

// The last 16 potentiometer readings: 32 bytes of data, fixed at compile time.
static RingBuffer<uint16_t, SMOOTH_SAMPLES> g_pot_samples;

// ---------------------------------------------------------------------------
// PART 1: TEMPLATES
// ---------------------------------------------------------------------------

/* One function for any type that can be compared with >. */
template <typename T>
T largest(T a, T b) {
    return (a > b) ? a : b;
}

static void demoTemplates(void) {
    Serial.println();
    Serial.println("--- 1. Templates ---");
    Serial.printf("largest(3U, 7U)   = %u\n", (unsigned)largest(3U, 7U));
    Serial.printf("largest(-40, 12)  = %d\n", (int)largest(-40, 12));

    RingBuffer<uint8_t, 4> small;
    for (uint8_t v = 1U; v <= 6U; v++) {
        small.push(v);
    }
    Serial.print("RingBuffer<uint8_t, 4> after pushing 1 to 6: ");
    for (uint8_t i = 0U; i < small.count(); i++) {
        Serial.printf("%u ", (unsigned)small.at(i));
    }
    Serial.println("  (the last four, oldest first)");

    Serial.printf("sizeof(RingBuffer<uint16_t, 16>) = %u bytes, known before the program runs\n",
                  (unsigned)sizeof(g_pot_samples));
}

// ---------------------------------------------------------------------------
// PART 2: THE TRAFFIC LIGHT, A STATE MACHINE
// ---------------------------------------------------------------------------

enum class Light : uint8_t {
    Green,
    Yellow,
    Red,
    RedYellow
};

static Light g_light = Light::Green;
static uint32_t g_entered_ms = 0U;      // when the current state began
static bool g_request = false;          // pedestrian button was pressed

static const char *lightName(Light light) {
    switch (light) {
        case Light::Green:     return "GREEN";
        case Light::Yellow:    return "YELLOW";
        case Light::Red:       return "RED";
        case Light::RedYellow: return "RED + YELLOW";
    }
    return "?";
}

/* Every change of state goes through here: one place sets the LED. */
static void enterState(Light next) {
    g_light = next;
    g_entered_ms = millis();

    switch (next) {
        case Light::Green:     led.setColour(COLOUR_GREEN);      break;
        case Light::Yellow:    led.setColour(COLOUR_YELLOW);     break;
        case Light::Red:       led.setColour(COLOUR_RED);        break;
        case Light::RedYellow: led.setColour(COLOUR_RED_YELLOW); break;
    }

    Serial.printf("[light] %s\n", lightName(next));
}

static uint32_t timeInState(void) {
    return millis() - g_entered_ms;
}

/* Green waits for a pedestrian, but never less than GREEN_MIN_MS. */
static void handleGreen(void) {
    if (g_request && (timeInState() >= GREEN_MIN_MS)) {
        g_request = false;
        enterState(Light::Yellow);
    }
}

static void handleYellow(void) {
    if (timeInState() >= YELLOW_MS) {
        enterState(Light::Red);
    }
}

static void handleRed(void) {
    if (timeInState() >= RED_MS) {
        enterState(Light::RedYellow);
    }
}

static void handleRedYellow(void) {
    if (timeInState() >= RED_YELLOW_MS) {
        enterState(Light::Green);
    }
}

static void runTrafficLight(void) {
    if (pedestrian.wasPressed() && !g_request) {
        g_request = true;
        Serial.println("[light] pedestrian request");
    }

    switch (g_light) {
        case Light::Green:     handleGreen();     break;
        case Light::Yellow:    handleYellow();    break;
        case Light::Red:       handleRed();       break;
        case Light::RedYellow: handleRedYellow(); break;
    }
}

// ---------------------------------------------------------------------------
// PART 3: SMOOTHING THE POTENTIOMETER
// ---------------------------------------------------------------------------

static void runPotentiometer(void) {
    static uint32_t last_sample_ms = 0U;
    static uint32_t last_print_ms = 0U;
    static uint16_t last_raw = 0U;

    const uint32_t now = millis();

    // One new reading per period, not sixteen in a row.
    if ((now - last_sample_ms) >= SAMPLE_PERIOD_MS) {
        last_sample_ms = now;
        last_raw = (uint16_t)analogRead(PIN_POT);
        g_pot_samples.push(last_raw);
    }

    if ((now - last_print_ms) < POT_PRINT_PERIOD_MS) {
        return;
    }
    last_print_ms = now;

    uint32_t sum = 0U;
    uint16_t lowest = 0xFFFFU;
    uint16_t highest = 0U;
    for (uint8_t i = 0U; i < g_pot_samples.count(); i++) {
        const uint16_t v = g_pot_samples.at(i);
        sum += v;
        lowest = (v < lowest) ? v : lowest;
        highest = (v > highest) ? v : highest;
    }
    const uint16_t smooth = (uint16_t)(sum / g_pot_samples.count());

    Serial.printf("[pot]   raw %4u   smooth %4u   last 16 spread over %u counts\n",
                  (unsigned)last_raw, (unsigned)smooth, (unsigned)(highest - lowest));
}

// ---------------------------------------------------------------------------
// PART 4: AN INTERRUPT ON BUTTON B
// ---------------------------------------------------------------------------

// Written only by the interrupt handler, read by loop(): volatile.
static volatile uint16_t g_isr_edges = 0U;

/* Keep it short: count the edge and leave. No Serial, no delay. */
static void IRAM_ATTR onButtonB(void) {
    g_isr_edges = (uint16_t)(g_isr_edges + 1U);
}

/*
 * Button B is read twice. The Button object reports one press per press.
 * The interrupt counts every falling edge, bounces included. The difference
 * between the two numbers is the bounce.
 */
static void runInterruptCounter(void) {
    static uint16_t edges_seen = 0U;

    if (buttonB.wasPressed()) {
        const uint16_t edges_now = g_isr_edges;
        const uint16_t edges = (uint16_t)(edges_now - edges_seen);
        edges_seen = edges_now;

        Serial.printf("[irq]   1 press, %u falling edges seen by the interrupt since the last one\n",
                      (unsigned)edges);
    }
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    led.begin();
    pedestrian.begin();
    buttonB.begin();   // INPUT_PULLUP first, then the interrupt

    attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_B), onButtonB, FALLING);

    analogReadResolution(12);
    analogSetPinAttenuation(PIN_POT, ADC_11db);

    Serial.println();
    Serial.println("Lesson 10 - States, Templates and Interrupts");

    demoTemplates();

    Serial.println();
    Serial.println("--- 2, 3, 4 running together ---");
    Serial.println("Button A: pedestrian request. Button B: interrupt counter. Knob: smoothing.");

    // Fill the smoothing buffer once so the first average is not over one sample.
    for (uint8_t i = 0U; i < SMOOTH_SAMPLES; i++) {
        g_pot_samples.push((uint16_t)analogRead(PIN_POT));
    }

    enterState(Light::Green);
}

void loop(void) {
    runTrafficLight();
    runPotentiometer();
    runInterruptCounter();
}
