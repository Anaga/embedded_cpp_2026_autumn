/*
 * Lesson 07 - instructor demo
 * Pointers, Arrays and Strings
 *
 * At startup: four experiments, one per part of the lesson.
 * Then: a button press counter, in one of three modes.
 *
 * Wiring:
 *   button, one leg   -> GPIO 0
 *   button, other leg -> GND
 *   no resistor: the internal pull-up is switched on in setup()
 *
 * Serial monitor: 115200
 *
 * HOW TO RUN THE BUTTON PART IN CLASS
 *
 *   1. BUTTON_MODE = MODE_LEVEL      Counts while the button is held.
 *                                    One press adds thousands.
 *   2. BUTTON_MODE = MODE_EDGE       Counts the change from released to
 *                                    pressed. Mostly right, but now and then
 *                                    one press adds 2, 3 or 5: that is bounce.
 *   3. BUTTON_MODE = MODE_DEBOUNCED  Uses isButtonPressed(). Exactly one per
 *                                    press, every time.
 *
 * Change the constant, upload, press the button a few times. Each line shows
 * the new total and how much one press added.
 */

#include <Arduino.h>
#include <stdint.h>
#include <string.h>

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t BUTTON_PIN = 0U;

static const uint8_t MODE_LEVEL = 0U;
static const uint8_t MODE_EDGE = 1U;
static const uint8_t MODE_DEBOUNCED = 2U;

// Change this in class: MODE_LEVEL, then MODE_EDGE, then MODE_DEBOUNCED.
static const uint8_t BUTTON_MODE = MODE_LEVEL;
//static const uint8_t BUTTON_MODE = MODE_DEBOUNCED;

// How long the pin must stay quiet before a new press is believed.
static const uint32_t DEBOUNCE_MS = 30U;

// The counter is printed at most this often, so the monitor stays readable.
static const uint32_t PRINT_PERIOD_MS = 100U;

static const uint8_t READING_COUNT = 5U;
static const uint8_t LINE_SIZE = 32U;

// ---------------------------------------------------------------------------
// PART 1: ARRAYS
// ---------------------------------------------------------------------------

static void demoArrays(void) {
    const uint8_t readings[READING_COUNT] = { 10U, 20U, 30U, 40U, 50U };

    Serial.println();
    Serial.println("--- 1. An array: many boxes, one name ---");
    Serial.printf("readings[0] = %u   (the first)\n", (unsigned)readings[0]);
    Serial.printf("readings[4] = %u   (the last)\n", (unsigned)readings[4]);
    Serial.printf("sizeof(readings) = %u bytes\n", (unsigned)sizeof(readings));

    uint16_t sum = 0U;
    for (uint8_t i = 0U; i < READING_COUNT; i++) {
        sum = (uint16_t)(sum + readings[i]);
    }
    Serial.printf("sum of all five = %u\n", (unsigned)sum);
}

// ---------------------------------------------------------------------------
// PART 2: ADDRESSES
// ---------------------------------------------------------------------------

uint8_t g_global = 1U;  // a global: one fixed place in RAM

/* A local lives in the frame of whichever call created it. */
static void printLocalAddress(const char *label) {
    uint8_t local = 3U;
    Serial.printf("  %-20s %p\n", label, (void *)&local);
}

/* A bigger frame in between, so the next local lands further down. */
static void callFromDeeper(void) {
    volatile uint8_t padding[32] = { 0U };
    (void)padding;
    printLocalAddress("local, one deeper");
}

static void demoAddresses(void) {
    static uint8_t s_calls = 0U;       // static local: not on the stack
    const char *text = "Hello";         // the letters live in flash

    Serial.println();
    Serial.println("--- 2. Where things live ---");
    Serial.printf("  %-20s %p\n", "global", (void *)&g_global);
    Serial.printf("  %-20s %p\n", "static local", (void *)&s_calls);
    printLocalAddress("local");
    callFromDeeper();
    Serial.printf("  %-20s %p\n", "text \"Hello\"", (void *)text);
    Serial.printf("  %-20s %p\n", "the pointer to it", (void *)&text);
    Serial.printf("  %-20s %p\n", "the code of setup", (void *)&setup);
}

// ---------------------------------------------------------------------------
// PART 3: POINTERS
// ---------------------------------------------------------------------------

/* Changes the caller's variable through its address. */
static void addOne(uint8_t *target) {
    *target = (uint8_t)(*target + 1U);
}

/* Two answers from one call: both go back through pointers. */
static void minMax(const uint8_t *values, uint8_t count,
                   uint8_t *min_out, uint8_t *max_out) {
    uint8_t lowest = values[0];
    uint8_t highest = values[0];

    for (uint8_t i = 1U; i < count; i++) {
        if (values[i] < lowest) {
            lowest = values[i];
        }
        if (values[i] > highest) {
            highest = values[i];
        }
    }

    *min_out = lowest;
    *max_out = highest;
}

/* A passed array arrives as a pointer: sizeof measures the pointer. */
static void showSizeInside(const uint8_t *values, uint8_t count) {
    (void)count;
    Serial.printf("  inside a function, sizeof(values) = %u\n",
                  (unsigned)sizeof(values));
}

static void demoPointers(void) {
    Serial.println();
    Serial.println("--- 3. Pointers ---");

    uint8_t count = 5U;
    addOne(&count);
    Serial.printf("after addOne(&count), count = %u\n", (unsigned)count);

    const uint8_t readings[READING_COUNT] = { 42U, 7U, 99U, 13U, 58U };
    uint8_t lowest = 0U;
    uint8_t highest = 0U;
    minMax(readings, READING_COUNT, &lowest, &highest);
    Serial.printf("minMax gives %u and %u\n", (unsigned)lowest, (unsigned)highest);

    Serial.printf("  where it was declared, sizeof(readings) = %u\n",
                  (unsigned)sizeof(readings));
    showSizeInside(readings, READING_COUNT);
    Serial.println("Same array, two answers. That is why count travels with it.");
}

// ---------------------------------------------------------------------------
// PART 4: STRINGS AND FORMATS
// ---------------------------------------------------------------------------

static void demoStrings(void) {
    Serial.println();
    Serial.println("--- 4. Text is an array too ---");

    const char greeting[] = "Hi";
    Serial.printf("\"Hi\": sizeof = %u, strlen = %u   (the extra one is the zero)\n",
                  (unsigned)sizeof(greeting), (unsigned)strlen(greeting));

    char line[LINE_SIZE];
    const uint8_t a = 7U;
    const uint8_t b = 8U;
    snprintf(line, sizeof(line), "%u x %u = %u",
             (unsigned)a, (unsigned)b, (unsigned)(a * b));
    Serial.print("built with snprintf: ");
    Serial.println(line);

    const char *formats[] = { "%u", "%o", "%X" };
    const char *names[] = { "decimal", "octal", "hex" };
    const uint8_t format_count = (uint8_t)(sizeof(formats) / sizeof(formats[0]));

    Serial.println("255 written three ways:");
    for (uint8_t i = 0U; i < format_count; i++) {
        snprintf(line, sizeof(line), formats[i], 255U);
        Serial.printf("  %-8s %s\n", names[i], line);
    }

    Serial.printf("with a width of 4: [%4u] [%4o] [%4X]\n", 7U, 7U, 7U);
}

// ---------------------------------------------------------------------------
// PART 5: THE BUTTON
// ---------------------------------------------------------------------------

/* Mode 1: counts every pass of loop() while the button is down. */
static bool readLevel(void) {
    return digitalRead(BUTTON_PIN) == LOW;
}

/* Mode 2: counts the moment of change from released to pressed. */
static bool readEdge(void) {
    static uint8_t previous = HIGH;

    const uint8_t current = (uint8_t)digitalRead(BUTTON_PIN);
    const bool pressed = (previous == HIGH) && (current == LOW);
    previous = current;

    return pressed;
}

/*
 * Mode 3: a press only counts if the pin had been quiet for DEBOUNCE_MS
 * before it. Every change restarts the quiet time, so the bounces that follow
 * a press, and the bounces of a release, are all ignored.
 */
static bool isButtonPressed(void) {
    static uint8_t previous = HIGH;
    static uint32_t last_change_ms = 0U;

    const uint8_t current = (uint8_t)digitalRead(BUTTON_PIN);
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

static bool buttonEvent(void) {
    if (BUTTON_MODE == MODE_LEVEL) {
        return readLevel();
    }
    if (BUTTON_MODE == MODE_EDGE) {
        return readEdge();
    }
    return isButtonPressed();
}

static const char *modeName(void) {
    if (BUTTON_MODE == MODE_LEVEL) {
        return "LEVEL: counts while held";
    }
    if (BUTTON_MODE == MODE_EDGE) {
        return "EDGE: counts each change, no debounce";
    }
    return "DEBOUNCED: isButtonPressed()";
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

static uint32_t g_presses = 0U;
static uint32_t g_last_printed = 0U;
static uint32_t g_last_print_ms = 0U;

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.println();
    Serial.println("Lesson 07 - Pointers, Arrays and Strings");

    demoArrays();
    demoAddresses();
    demoPointers();
    demoStrings();

    Serial.println();
    Serial.println("--- 5. The button ---");
    Serial.print("Mode: ");
    Serial.println(modeName());
    Serial.println("Press the button a few times.");
}

void loop(void) {
    // No delay here on purpose: the loop runs as fast as it can, which is
    // exactly what lets it see every bounce.
    if (buttonEvent()) {
        g_presses++;
    }

    const uint32_t now = millis();
    if ((now - g_last_print_ms) < PRINT_PERIOD_MS) {
        return;
    }
    g_last_print_ms = now;

    if (g_presses != g_last_printed) {
        Serial.printf("presses = %lu   (+%lu)\n",
                      (unsigned long)g_presses,
                      (unsigned long)(g_presses - g_last_printed));
        g_last_printed = g_presses;
    }
}
