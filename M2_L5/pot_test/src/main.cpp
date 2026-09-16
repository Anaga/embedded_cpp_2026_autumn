/*
 * Lesson 05 - potentiometer calibration probe
 *
 * Instructor tool, run before the lesson. Purpose: find the real ADC range
 * of one specific board plus one specific potentiometer, so the homework
 * scaffold can map the full travel of the knob onto -40 ... +140 F without
 * a dead zone at either end.
 *
 * Wiring:
 *   potentiometer middle pin  -> GPIO 4  (ADC1)
 *   potentiometer one end     -> 3V3 through a 3.3k resistor
 *   potentiometer other end   -> GND
 *
 * The series resistor keeps the wiper below the ADC saturation point.
 * Without it the top of the travel reads a constant 4095 and the printed
 * temperature stops changing.
 *
 * It also prints the on-chip temperature sensor once per second, with the
 * change since power-on. Warming the package with a finger should move it.
 * The sensor measures the die, not the room, so the absolute number sits
 * well above ambient. That gap is the point of the classroom demo.
 *
 * How to use:
 *   1. Flash, open the serial monitor at 115200.
 *   2. Turn the knob slowly from one end to the other, twice.
 *   3. Watch min and max settle. Neither end should show SATURATED.
 *   4. Write down min and max. Those two numbers go into the scaffold.
 *   5. Send any character to reset min and max and measure again.
 */

#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t POT_PIN = 4U;             // ADC1 channel, GPIO 4
static const uint8_t SAMPLE_COUNT = 16U;       // readings averaged per print
static const uint16_t SAMPLE_GAP_US = 200U;    // spacing between samples
static const uint32_t PRINT_PERIOD_MS = 200U;
static const uint32_t CHIP_PRINT_PERIOD_MS = 1000U;  // die temperature moves slowly

static const uint16_t ADC_COUNT_MAX = 4095U;   // 12-bit converter
static const uint16_t SATURATED_HIGH = 4085U;  // treat as pinned to the top
static const uint16_t SATURATED_LOW = 10U;     // treat as pinned to the bottom

// Target range of the homework, in whole degrees Fahrenheit.
static const int16_t F_MIN = -40;
static const int16_t F_MAX = 140;

// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint16_t g_min_seen = ADC_COUNT_MAX;
static uint16_t g_max_seen = 0U;
static uint32_t g_last_print_ms = 0U;
static uint32_t g_last_chip_ms = 0U;
static int16_t g_chip_baseline_deci = 0;   // first reading, in tenths of a degree
static bool g_chip_baseline_valid = false;

// ---------------------------------------------------------------------------
// HELPERS
// ---------------------------------------------------------------------------

static uint16_t readAveraged(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t)analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t)(sum / (uint32_t)SAMPLE_COUNT);
}

/*
 * On-chip temperature sensor, returned in tenths of a degree Celsius.
 *
 * temperatureRead() hands back a float. Float is acceptable here because
 * this is an instructor tool; the student scaffold stays integer only.
 */
static int16_t readChipDeciC(void) {
    const float celsius = temperatureRead();
    return (int16_t)(celsius * 10.0f);
}

static void printChipTemperature(int16_t deci) {
    const int16_t delta = (int16_t)(deci - g_chip_baseline_deci);
    const char *sign = (delta < 0) ? "-" : "+";
    const uint16_t magnitude = (delta < 0) ? (uint16_t)(-delta) : (uint16_t)delta;

    Serial.printf("chip temperature: %d.%u C   since start: %s%u.%u C\n",
                  (int)(deci / 10),
                  (unsigned)(deci % 10),
                  sign,
                  (unsigned)(magnitude / 10U),
                  (unsigned)(magnitude % 10U));
}

/*
 * Preview of the mapping the students will get. Uses the min and max
 * observed so far, not fixed constants, so the numbers are meaningful
 * only after the knob has been turned across its whole travel.
 */
static int16_t mapToFahrenheit(uint16_t counts, uint16_t lo, uint16_t hi) {
    if (hi <= lo) {
        return F_MIN;
    }
    if (counts < lo) {
        counts = lo;
    }
    if (counts > hi) {
        counts = hi;
    }

    const int32_t span = (int32_t)hi - (int32_t)lo;
    const int32_t offset = (int32_t)counts - (int32_t)lo;
    const int32_t degrees = (int32_t)F_MAX - (int32_t)F_MIN;

    return (int16_t)((int32_t)F_MIN + (offset * degrees) / span);
}

static void resetRange(void) {
    g_min_seen = ADC_COUNT_MAX;
    g_max_seen = 0U;
    g_chip_baseline_valid = false;
    Serial.println("Range reset. Turn the knob end to end again.");
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);  // native USB CDC needs a moment before the first print

    analogReadResolution(12);

    // ADC_11db is the widest input range. On newer cores the symbol is an
    // alias for the 12 dB setting; the behaviour is the same.
    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println();
    Serial.println("Potentiometer calibration probe");
    Serial.println("GPIO 4, 12-bit, 16 samples averaged");
    Serial.println("Chip temperature printed once per second.");
    Serial.println("Turn the knob slowly from end to end. Send any key to reset.");
    Serial.println();
}

void loop(void) {
    const uint16_t counts = readAveraged();

    if (counts < g_min_seen) {
        g_min_seen = counts;
    }
    if (counts > g_max_seen) {
        g_max_seen = counts;
    }

    if (Serial.available() > 0) {
        while (Serial.available() > 0) {
            (void)Serial.read();
        }
        resetRange();
    }

    const uint32_t now = millis();

    if ((now - g_last_chip_ms) >= CHIP_PRINT_PERIOD_MS) {
        g_last_chip_ms = now;
        const int16_t chip_deci = readChipDeciC();
        if (!g_chip_baseline_valid) {
            g_chip_baseline_deci = chip_deci;
            g_chip_baseline_valid = true;
        }
        printChipTemperature(chip_deci);
    }
}