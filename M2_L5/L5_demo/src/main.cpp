/*
 * Lesson 05 - instructor demo
 * Variables, Types and Arithmetic
 *
 * Runs the four classroom experiments once at startup, then enters the live
 * potentiometer loop that the homework builds on.
 *
 * Wiring:
 *   potentiometer middle pin -> GPIO 4
 *   one outer pin            -> 3V3 (through a 3.3k resistor if fitted)
 *   other outer pin          -> GND
 *
 * Serial monitor: 115200
 *
 * Note on the mapping constants: ADC_MIN_COUNTS and ADC_MAX_COUNTS come from
 * measuring one board with one potentiometer. Re-measure after changing the
 * wiring, in particular after adding or removing the series resistor.
 */

#include <Arduino.h>
#include <stdint.h>
#include <math.h>

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t POT_PIN = 4U;              // ADC1 channel on GPIO 4
static const uint8_t SAMPLE_COUNT = 16U;        // readings averaged per value
static const uint16_t SAMPLE_GAP_US = 200U;
static const uint32_t PRINT_PERIOD_MS = 250U;

// A value is only printed when it has actually moved. The deadband is in ADC
// counts and keeps a reading that sits between two degrees from flickering.
static const uint16_t PRINT_DEADBAND_COUNTS = 8U;

// Measured ends of the potentiometer travel, in ADC counts.
// Direct connection to 3V3: the top of the travel clips, so the usable
// maximum sits below the full scale value of 4095.
static const uint16_t ADC_MIN_COUNTS = 0U;
static const uint16_t ADC_MAX_COUNTS = 3850U;

// Temperature range the knob is mapped onto, in whole degrees Fahrenheit.
static const int16_t F_MIN = -40;
static const int16_t F_MAX = 140;

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_print_ms = 0U;
static uint16_t g_last_counts = 0U;
static int16_t g_last_fahrenheit = 0;
static bool g_printed_once = false;

// ---------------------------------------------------------------------------
// POTENTIOMETER
// ---------------------------------------------------------------------------

/* Average several readings so the last digit stops jumping.
 * Why this is needed is a question for Session 11. */
static uint16_t readPotentiometer(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t)analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t)(sum / (uint32_t)SAMPLE_COUNT);
}

/* Map raw ADC counts onto the Fahrenheit range, in whole degrees.
 * Whole-number arithmetic only: this chip has no floating point unit. */
static int16_t mapToFahrenheit(uint16_t counts) {
    if (counts < ADC_MIN_COUNTS) {
        counts = ADC_MIN_COUNTS;
    }
    if (counts > ADC_MAX_COUNTS) {
        counts = ADC_MAX_COUNTS;
    }

    const int32_t offset = (int32_t)counts - (int32_t)ADC_MIN_COUNTS;
    const int32_t span = (int32_t)ADC_MAX_COUNTS - (int32_t)ADC_MIN_COUNTS;
    const int32_t degrees = (int32_t)F_MAX - (int32_t)F_MIN;

    return (int16_t)((int32_t)F_MIN + (offset * degrees) / span);
}

// ---------------------------------------------------------------------------
// CLASSROOM EXPERIMENTS
// ---------------------------------------------------------------------------

/* Part 1: the same value 7 in three different boxes. */
static void demoSizeof(void) {
    const uint8_t small = 7U;
    const uint16_t medium = 7U;
    const uint32_t large = 7U;

    Serial.println();
    Serial.println("--- 1. How big is a variable ---");
    Serial.printf("uint8_t  holds %u and takes %u byte(s)\n",
                  (unsigned)small, (unsigned)sizeof(small));
    Serial.printf("uint16_t holds %u and takes %u byte(s)\n",
                  (unsigned)medium, (unsigned)sizeof(medium));
    Serial.printf("uint32_t holds %u and takes %u byte(s)\n",
                  (unsigned)large, (unsigned)sizeof(large));
}

/* Part 2: what happens one step past the last value of a type. */
static void demoOverflow(void) {
    Serial.println();
    Serial.println("--- 2. The edge of a type ---");

    Serial.print("uint8_t counting up from 250: ");
    uint8_t counter = 250U;
    for (uint8_t i = 0U; i < 8U; i++) {
        Serial.print(counter);
        Serial.print(" ");
        counter = (uint8_t)(counter + 1U);
    }
    Serial.println();

    int8_t warm = 120;
    warm = (int8_t)(warm + 10);
    Serial.printf("int8_t: 120 + 10 gives %d\n", (int)warm);
    Serial.println("No error, no warning. The value wrapped around.");
}

/* Part 3: integer division drops the fraction, so order matters. */
static void demoIntegerMath(void) {
    Serial.println();
    Serial.println("--- 3. Whole-number arithmetic ---");
    Serial.printf("5 / 9  = %d\n", 5 / 9);
    Serial.printf("9 / 5  = %d\n", 9 / 5);
    Serial.printf("-5 / 2 = %d   (cut toward zero, not rounded down)\n", -5 / 2);

    const int32_t value = 250;
    Serial.println("60 percent of 250, written two ways:");
    Serial.printf("  value * 60 / 100 = %d   (multiply first)\n",
                  (int)(value * 60 / 100));
    Serial.printf("  value / 100 * 60 = %d   (divide first)\n",
                  (int)(value / 100 * 60));
    Serial.println("Both compile. Only one is right.");
}

/* Part 4: the sensor that measures something other than what you expect. */
static void demoChipTemperature(void) {
    // temperatureRead() returns a float. It is converted to tenths of a
    // degree straight away, so the rest of the program stays integer only.
    const float celsius = temperatureRead();
    const int16_t deci = (int16_t)lroundf(celsius * 10.0f);

    Serial.println();
    Serial.println("--- 4. The chip has its own thermometer ---");
    Serial.printf("Die temperature: %d.%u C\n",
                  (int)(deci / 10), (unsigned)(abs(deci) % 10));
    Serial.println("Warmer than the room. It measures the silicon, not the air.");
    Serial.println("Read the datasheet before trusting a number.");
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(5000U);  // native USB CDC needs a moment before the first print

    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println();
    Serial.println("Lesson 05 - Variables, Types and Arithmetic");

    demoSizeof();
    demoOverflow();
    demoIntegerMath();
    demoChipTemperature();

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

    Serial.print("counts = ");
    Serial.print(counts);
    Serial.print("   F = ");
    Serial.println(fahrenheit);
}
