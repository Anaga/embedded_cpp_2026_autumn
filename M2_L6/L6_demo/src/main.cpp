/*
 * Lesson 06 - instructor demo
 * Functions, Scope and Bits
 *
 * Runs four experiments at startup, then walks through the five colours the
 * homework uses, then enters the live potentiometer loop.
 *
 * Wiring:
 *   potentiometer middle pin -> GPIO 4
 *   potentiometer outer pins -> 3V3 (through 3.3k) and GND
 *
 *   RGB LED, common anode:
 *     longest pin -> 3V3
 *     red         -> GPIO 5, through 100 ohm
 *     green       -> GPIO 6, through 100 ohm
 *     blue        -> GPIO 7, through 100 ohm
 *
 * Common anode means the logic is inverted: LOW switches a colour ON.
 *
 * Serial monitor: 115200
 */

#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t POT_PIN = 4U;
static const uint8_t LED_RED_PIN = 5U;
static const uint8_t LED_GREEN_PIN = 6U;
static const uint8_t LED_BLUE_PIN = 7U;

static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;
static const uint32_t PRINT_PERIOD_MS = 250U;
static const uint16_t PRINT_DEADBAND_COUNTS = 8U;
static const uint32_t COLOUR_DEMO_MS = 800U;

static const uint16_t ADC_MIN_COUNTS = 0U;
static const uint16_t ADC_MAX_COUNTS = 3850U;

static const int16_t F_MIN = -40;
static const int16_t F_MAX = 140;

// A colour is three bits: bit 0 red, bit 1 green, bit 2 blue.
static const uint8_t BIT_RED = 0x01U;
static const uint8_t BIT_GREEN = 0x02U;
static const uint8_t BIT_BLUE = 0x04U;

static const uint8_t COLOUR_OFF = 0x00U;
static const uint8_t COLOUR_RED = BIT_RED;
static const uint8_t COLOUR_YELLOW = BIT_RED | BIT_GREEN;
static const uint8_t COLOUR_GREEN = BIT_GREEN;
static const uint8_t COLOUR_CYAN = BIT_GREEN | BIT_BLUE;
static const uint8_t COLOUR_BLUE = BIT_BLUE;

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_print_ms = 0U;
static uint16_t g_last_counts = 0U;
static int16_t g_last_fahrenheit = 0;
static bool g_printed_once = false;

// ---------------------------------------------------------------------------
// PART 1: FUNCTIONS
// ---------------------------------------------------------------------------

/* The smallest useful function: one input, one answer. */
static int16_t doubleIt(int16_t value) {
    return value * 2;
}

static void demoFunctions(void) {
    Serial.println();
    Serial.println("--- 1. A function takes something in and gives something back ---");
    Serial.printf("doubleIt(7)   = %d\n", (int)doubleIt(7));
    Serial.printf("doubleIt(-21) = %d\n", (int)doubleIt(-21));
    Serial.printf("doubleIt(doubleIt(3)) = %d\n", (int)doubleIt(doubleIt(3)));
}

// ---------------------------------------------------------------------------
// PART 2: SCOPE AND LIFETIME
// ---------------------------------------------------------------------------

/* A fresh local every call: it never remembers anything. */
static void countWithLocal(void) {
    uint8_t calls = 0U;
    calls = (uint8_t)(calls + 1U);
    Serial.printf("  local  says %u\n", (unsigned)calls);
}

/* A static local: created once, survives every return. */
static void countWithStatic(void) {
    static uint8_t calls = 0U;
    calls = (uint8_t)(calls + 1U);
    Serial.printf("  static says %u\n", (unsigned)calls);
}

static void demoScope(void) {
    Serial.println();
    Serial.println("--- 2. Same code, one keyword of difference ---");
    for (uint8_t i = 0U; i < 4U; i++) {
        countWithLocal();
        countWithStatic();
    }
    Serial.println("The local starts again every call. The static does not.");
}

// ---------------------------------------------------------------------------
// PART 3: BITS
// ---------------------------------------------------------------------------

/* Print all eight bits of a byte, highest bit first. */
static void printBinary(uint8_t value) {
    for (int8_t i = 7; i >= 0; i--) {
        Serial.print((value >> i) & 1U);
    }
}

static void printBinaryLine(const char *label, uint8_t value) {
    Serial.print("  ");
    Serial.print(label);
    Serial.print(" = ");
    printBinary(value);
    Serial.println();
}

static void demoBits(void) {
    Serial.println();
    Serial.println("--- 3. What a number looks like inside ---");
    printBinaryLine("  0", 0U);
    printBinaryLine("  1", 1U);
    printBinaryLine("  5", 5U);
    printBinaryLine("128", 128U);
    printBinaryLine("255", 255U);

    // The same eight bits, read as a signed value instead.
    const int8_t negative = -40;
    printBinaryLine("-40", (uint8_t)negative);
    Serial.println("Bit 7 is set, and the rest is not 40. That is how negatives are stored.");
}

// ---------------------------------------------------------------------------
// PART 4: THE RGB LED
// ---------------------------------------------------------------------------

/*
 * Switch the three channels according to the three bits of `colour`.
 * The LED is common anode, so LOW turns a channel ON.
 */
static void setColour(uint8_t colour) {
    digitalWrite(LED_RED_PIN, ((colour & BIT_RED) != 0U) ? LOW : HIGH);
    digitalWrite(LED_GREEN_PIN, ((colour & BIT_GREEN) != 0U) ? LOW : HIGH);
    digitalWrite(LED_BLUE_PIN, ((colour & BIT_BLUE) != 0U) ? LOW : HIGH);
}

static void showColour(const char *name, uint8_t colour) {
    Serial.print("  ");
    Serial.print(name);
    Serial.print("  bits = ");
    printBinary(colour);
    Serial.println();
    setColour(colour);
    delay(COLOUR_DEMO_MS);
}

static void demoColours(void) {
    Serial.println();
    Serial.println("--- 4. Five colours out of three switches ---");
    showColour("blue  ", COLOUR_BLUE);
    showColour("cyan  ", COLOUR_CYAN);
    showColour("green ", COLOUR_GREEN);
    showColour("yellow", COLOUR_YELLOW);
    showColour("red   ", COLOUR_RED);
    setColour(COLOUR_OFF);
}

// ---------------------------------------------------------------------------
// POTENTIOMETER
// ---------------------------------------------------------------------------

static uint16_t readPotentiometer(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t)analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t)(sum / (uint32_t)SAMPLE_COUNT);
}


// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    setColour(COLOUR_OFF);

    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println();
    Serial.println("Lesson 06 - Functions, Scope and Bits");

    demoFunctions();
    demoScope();
    demoBits();
    demoColours();

    Serial.println();
    Serial.println("--- Live: turn the knob ---");
    Serial.println("A line appears only when the value changes.");
}

void loop(void) {
    const uint32_t now = millis();
    if ((now - g_last_print_ms) < PRINT_PERIOD_MS) {
        return;
    }
    g_last_print_ms = now;

    const uint16_t counts = readPotentiometer();
    const int16_t fahrenheit = mapToFahrenheit(counts);

    const uint16_t moved = (counts > g_last_counts)
                               ? (uint16_t)(counts - g_last_counts)
                               : (uint16_t)(g_last_counts - counts);

    const bool worth_printing = (!g_printed_once) ||
                                ((fahrenheit != g_last_fahrenheit) &&
                                 (moved >= PRINT_DEADBAND_COUNTS));

    if (!worth_printing) {
        return;
    }

    g_last_counts = counts;
    g_last_fahrenheit = fahrenheit;
    g_printed_once = true;

    Serial.printf("F = %+4d\n", (int)fahrenheit);
}
