/*
 * Lesson 06 - homework
 * The colour thermometer
 *
 * The knob sets a temperature between -40 and +140 degrees Celsius, the
 * range an engine cooling circuit lives in. Your job is to make the LED show
 * which band that temperature falls into.
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
 * Serial monitor: 115200
 *
 * Rules:
 *   - whole numbers only, no float
 *   - fixed-width types from stdint.h
 *   - no bare numbers inside a function body; name them at the top
 */

#include <Arduino.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// CONFIGURATION - already set up for you
// ---------------------------------------------------------------------------

static const uint8_t POT_PIN = 4U;
static const uint8_t LED_RED_PIN = 5U;
static const uint8_t LED_GREEN_PIN = 6U;
static const uint8_t LED_BLUE_PIN = 7U;

static const uint8_t SAMPLE_COUNT = 16U;
static const uint16_t SAMPLE_GAP_US = 200U;
static const uint32_t UPDATE_PERIOD_MS = 250U;

static const uint16_t ADC_MIN_COUNTS = 0U;
static const uint16_t ADC_MAX_COUNTS = 3850U;

static const int16_t C_MIN = -40;
static const int16_t C_MAX = 140;

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

static const int16_t FROZEN = 0;
static const int16_t COLD = 60;
static const int16_t WARM = 90;
static const int16_t HOT = 110;

static const int16_t HYSTERESIS_MARGIN = 2;


// ---------------------------------------------------------------------------
// STATE
// ---------------------------------------------------------------------------

static uint32_t g_last_update_ms = 0U;

// ---------------------------------------------------------------------------
// GIVEN TO YOU
// ---------------------------------------------------------------------------

static uint16_t readPotentiometer(void) {
    uint32_t sum = 0U;
    for (uint8_t i = 0U; i < SAMPLE_COUNT; i++) {
        sum += (uint32_t)analogRead(POT_PIN);
        delayMicroseconds(SAMPLE_GAP_US);
    }
    return (uint16_t)(sum / (uint32_t)SAMPLE_COUNT);
}

/* The function written together in class: knob position to whole degrees. */
static int16_t mapToCelsius(uint16_t counts) {
    if (counts > ADC_MAX_COUNTS) {
        counts = ADC_MAX_COUNTS;
    }

    const int32_t offset = (int32_t)counts - (int32_t)ADC_MIN_COUNTS;
    const int32_t span = (int32_t)ADC_MAX_COUNTS - (int32_t)ADC_MIN_COUNTS;
    const int32_t degrees = (int32_t)C_MAX - (int32_t)C_MIN;

    return (int16_t)((int32_t)C_MIN + (offset * degrees) / span);
}

/* Switch the three channels from the three bits. LOW turns a channel ON. */
static void setColour(uint8_t colour) {
    digitalWrite(LED_RED_PIN, ((colour & BIT_RED) != 0U) ? LOW : HIGH);
    digitalWrite(LED_GREEN_PIN, ((colour & BIT_GREEN) != 0U) ? LOW : HIGH);
    digitalWrite(LED_BLUE_PIN, ((colour & BIT_BLUE) != 0U) ? LOW : HIGH);
}

/* The function you wrote in class. Useful for seeing what you built. */
static void printBinary(uint8_t value) {
    for (int8_t i = 7; i >= 0; i--) {
        Serial.print((value >> i) & 1U);
    }
}

// ---------------------------------------------------------------------------
// YOUR WORK STARTS HERE
// ---------------------------------------------------------------------------

/*
 * TODO 1: return the colour for this temperature.
 *
 * The five bands are listed in the README. Give every boundary a name at the
 * top of the file instead of writing the number here.
 *
 * Return one of COLOUR_BLUE, COLOUR_CYAN, COLOUR_GREEN, COLOUR_YELLOW or
 * COLOUR_RED.
 */
static uint8_t colourForTemperature(int16_t celsius) {
    static bool first_measurement = true;
    static int16_t previous_measurement;
    static uint8_t previous_colour;
    uint8_t colour;

    if (first_measurement || previous_measurement - celsius > HYSTERESIS_MARGIN || celsius - previous_measurement > HYSTERESIS_MARGIN) {
        if (celsius < FROZEN) {
            colour = COLOUR_BLUE;
        } else if (celsius < COLD) {
            colour = COLOUR_CYAN;
        } else if (celsius < WARM) {
            colour = COLOUR_GREEN;
        } else if (celsius < HOT) {
            colour = COLOUR_YELLOW;
        } else {
            colour = COLOUR_RED;
        }
        previous_colour = colour;
        previous_measurement = celsius;      
    } else {
        colour = previous_colour;
    }
    first_measurement = false;
    return colour;
}

/*
 * TODO 2 (star task): stop the colour flickering.
 *
 * Park the knob exactly on a boundary and watch. The reading moves by a
 * degree on its own, so the colour jumps between two bands.
 *
 * Make the colour hold its band until the temperature has moved far enough
 * to really mean it. You need something that remembers the previous answer
 * between calls. You saw exactly that in the second hour.
 */

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
    Serial.println("Lesson 06 homework - the colour thermometer");
    Serial.println("Turn the knob.");
}

void loop(void) {
    const uint32_t now = millis();
    if ((now - g_last_update_ms) < UPDATE_PERIOD_MS) {
        return;
    }
    g_last_update_ms = now;

    const uint16_t counts = readPotentiometer();
    const int16_t celsius = mapToCelsius(counts);
    const uint8_t colour = colourForTemperature(celsius);

    setColour(colour);

    Serial.printf("C = %+4d   colour = ", (int)celsius);
    printBinary(colour);
    Serial.println();
}
