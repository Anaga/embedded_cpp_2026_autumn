/*
 * Lesson 05 - homework
 * Temperature conversion
 *
 * The board reads a potentiometer and turns the knob position into a
 * temperature in degrees Fahrenheit, from -40 F to +140 F.
 *
 * Your job: print the same temperature in degrees Celsius as well.
 *
 * Wiring:
 *   potentiometer middle pin -> GPIO 4
 *   one outer pin            -> 3V3 (through a resistor, see README)
 *   other outer pin          -> GND
 *
 * Serial monitor: 115200
 *
 * Rules:
 *   - whole numbers only, no float and no double
 *   - fixed-width types from stdint.h
 *   - no magic numbers inside functions
 */

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// "NO MAGIC NUMBERS INSIDE FUNCTIONS” RULE
// ---------------------------------------------------------------------------

static const uint8_t F_TO_C_OFFSET = 32;
static const uint8_t F_TO_C_NUMERATOR = 5;
static const uint8_t F_TO_C_DENOMINATOR = 9;
static const int32_t TENTHS_PER_DEGREE = 10;

// ---------------------------------------------------------------------------
// CONFIGURATION - already set up for you
// ---------------------------------------------------------------------------

static const uint8_t POT_PIN = 4U;
static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;
static const uint32_t PRINT_PERIOD_MS = 250U;
static const uint16_t PRINT_DEADBAND_COUNTS = 8U;

static const uint16_t ADC_MIN_COUNTS = 0U;
static const uint16_t ADC_MAX_COUNTS = 3850U;

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
// GIVEN TO YOU - no changes needed below this line until the TODO
// ---------------------------------------------------------------------------

/* Average several readings so the value stops jumping around. */
static uint16_t readPotentiometer(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t)analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t)(sum / (uint32_t)SAMPLE_COUNT);
}

/* Turn the knob position into a temperature in whole degrees Fahrenheit. */
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
// YOUR WORK STARTS HERE
// ---------------------------------------------------------------------------

/*
 * Return the same temperature in whole degrees Celsius.
 *
 * The formula is on the board. Getting it into C is the exercise:
 * the order of the operations decides whether the answer is right.
 *
 * Replace the return value below.
 */
static int16_t fahrenheitToCelsius(int16_t fahrenheit) {
    // Note the duplication/similarity with fahrenheitToCelsiusInTenths
    return (int16_t) ((fahrenheit - F_TO_C_OFFSET) * F_TO_C_NUMERATOR / F_TO_C_DENOMINATOR);
}

/*
 * Tenths of a degree.
 *
 * Write a second function that returns Celsius in tenths, so that 21.5 C
 * comes back as 215. Print it as a whole part and one digit after the point.
 * Still no float.
 */
static int16_t fahrenheitToCelsiusInTenths(int16_t fahrenheit) {
    // Note the duplication/similarity with fahrenheitToCelsius
    return (int16_t) ((fahrenheit - F_TO_C_OFFSET) * F_TO_C_NUMERATOR * TENTHS_PER_DEGREE / F_TO_C_DENOMINATOR);
}

/*
 * (bonus, nothing to write): change the type of the variable that
 * holds the Fahrenheit value in loop() from int16_t to int8_t. Turn the knob
 * all the way up and write down what you see, and why. Put the answer in your
 * commit message.
 *
 * Reasoning: int8_t holds −128 through +127, but the knob reaches can reach +140
 * where it would then overflow into negative numbers, like: 140 − 256 = −116.
 */

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    analogReadResolution(12);
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println();
    Serial.println("Lesson 05 homework - temperature conversion");
    Serial.println("Turn the knob. A line appears when the value changes.");
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

    const int16_t celsius = fahrenheitToCelsius(fahrenheit);

    const int16_t celsiusTenths = fahrenheitToCelsiusInTenths(fahrenheit);
    const int16_t celsiusTenthsMagnitude = (int16_t) abs(celsiusTenths);
    const int16_t celsiusTenthsWhole = (int16_t) (celsiusTenthsMagnitude / TENTHS_PER_DEGREE);
    const int16_t celsiusTenthsFraction = (int16_t) (celsiusTenthsMagnitude % TENTHS_PER_DEGREE);

    Serial.printf("F = %+4d   C = %+4d   Ctenths = %s%d.%d\n",
                  (int) fahrenheit, (int) celsius,
                  celsiusTenths >= 0 ? "+" : "-",
                  (int) celsiusTenthsWhole,
                  (int) celsiusTenthsFraction
    );
}
