/*
 * Lesson 07 - Home Task: The Multiplication Table. See README.md.
 */

#include <Arduino.h>
#include <stdint.h>
#include <string.h>

// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

static const uint8_t BUTTON_PIN = 0U;

static const uint8_t MODE_DEBOUNCED = 2U;

// How long the pin must stay quiet before a new press is believed.
static const uint32_t DEBOUNCE_MS = 30U;

// The counter is printed at most this often, so the monitor stays readable.
static const uint32_t PRINT_PERIOD_MS = 100U;

// ---------------------------------------------------------------------------
// THE BUTTON
// ---------------------------------------------------------------------------

/*
 * a press only counts if the pin had been quiet for DEBOUNCE_MS
 * before it. Every change restarts the quiet time, so the bounces that follow
 * a press, and the bounces of a release, are all ignored.
 */
static bool isButtonPressed(void) {
    static uint8_t previous = HIGH;
    static uint32_t last_change_ms = 0U;

    const uint8_t current = (uint8_t) digitalRead(BUTTON_PIN);
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

// ---------------------------------------------------------------------------
// MULTIPLICATIONS
// ---------------------------------------------------------------------------

// To escape the parameter array decay we would not implement a reusable function.
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

void calculateMultiplications(
    const uint8_t *array_to_multiply,
    size_t array_to_multiply_size,
    uint16_t *multiplications
) {
    uint8_t multiplicationCounter = 0;
    for (size_t i = 0; i < array_to_multiply_size; ++i) {
        for (size_t j = 0; j < array_to_multiply_size; ++j) {
            *(multiplications + multiplicationCounter) = array_to_multiply[i] * array_to_multiply[j];
            multiplicationCounter++;
        }
    }
}

// DEC: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
static const uint8_t decs[] = {
    1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U,
};

// OCT: 1, 2, 3, 4, 5, 6, 7, 8
static const uint8_t octs[] = {
    1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
};

// HEX: 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F, 10
static const uint8_t hexs[] = {
    0x1U, 0x2U, 0x3U, 0x4U, 0x5U, 0x6U, 0x7U, 0x8U, 0x9U, 0xAU, 0xBU, 0xCU, 0xDU, 0xEU, 0xFU, 0x10U,
};

const uint8_t MODE_DEC = 1U;
const uint8_t MODE_OCT = 2U;
const uint8_t MODE_HEX = 4U;

void printMultiplicationsAsTable(
    const uint8_t *array_to_multiply,
    size_t array_to_multiply_size,
    const uint16_t *multiplications,
    size_t multiplications_size,
    const uint8_t mode // DEC, OCT, HEX
) {
    // Figure out the mode, we default to some error :D
    const char *printf_format;
    switch (mode) {
        case MODE_DEC:
            printf_format = "%4u";
            break;
        case MODE_OCT:
            printf_format = "%4o";
            break;
        case MODE_HEX:
            printf_format = "%4X";
            break;
        default:
            return;
    }
    // Header: I fit my values padding into 4 +1 for the terminator character.
    // We also make sure the first value is the column for the table and last one fits \n.
    const uint8_t padding = 4U;
    const uint8_t table_column_left = 1U;
    const uint8_t newline = 1U;
    const uint8_t header_capacity =
            (padding * (table_column_left + array_to_multiply_size)) + newline;

    // Table rows: every table has column and a newline.
    const uint16_t multiplication_table_row_capacity =
            (padding * (table_column_left + array_to_multiply_size)) + newline;
    // Multiplication table is built based on array to multiply rows.
    const uint8_t rows = multiplications_size / array_to_multiply_size;
    const uint16_t multiplication_table_capacity = multiplication_table_row_capacity * rows;

    // Final table capacity
    const uint8_t terminator_size = 1U;
    const uint16_t table_capacity = // I need 16 bit to fit my hex table.
            header_capacity + multiplication_table_capacity + terminator_size;

    // Final table.
    char table[table_capacity];

    // Used to concat for snprintf
    uint16_t snprintf_offset = 0;

    // Add the first column marker.
    const char header_left_marker = 42U; // DEC code for *
    const uint8_t first_column_written = snprintf(
        table + snprintf_offset,
        table_capacity - snprintf_offset,
        "%4c",
        header_left_marker
    );
    snprintf_offset += first_column_written;

    // Add the values
    for (size_t i = 0; i < array_to_multiply_size; ++i) {
        const uint8_t written = snprintf(
            table + snprintf_offset,
            table_capacity - snprintf_offset,
            printf_format,
            array_to_multiply[i]
        );
        snprintf_offset += written;
    }

    // Add the newline.
    const char new_line = 10U; // DEC code for line feed
    const uint8_t line_end_written = snprintf(
        table + snprintf_offset,
        table_capacity - snprintf_offset,
        "%c",
        new_line
    );
    snprintf_offset += line_end_written;

    // Next we are printing the multiplication values.
    // The initial array drives the matrix content fetched from multiplications.

    // Build rows from multiplication values.
    uint8_t row_counter = 0;
    for (size_t multiplications_i = 0; multiplications_i < multiplications_size;) {
        // Used to track what multiplier is been plotted.
        size_t array_to_multiply_i = 0;

        // Every new row starts with column entry.
        const uint8_t column_written = snprintf(
            table + snprintf_offset,
            table_capacity - snprintf_offset,
            printf_format,
            array_to_multiply[row_counter]
        );
        snprintf_offset += column_written;

        // Then we plot the row of multiplications.
        for (; array_to_multiply_i < array_to_multiply_size; ++array_to_multiply_i) {
            const uint16_t multiplication_to_plot = multiplications[multiplications_i++];
            const uint8_t multiplication_written = snprintf(
                table + snprintf_offset,
                table_capacity - snprintf_offset,
                printf_format,
                multiplication_to_plot
            );
            snprintf_offset += multiplication_written;
        }

        // New-line is added for the new row.
        const uint8_t line_end_written = snprintf(
            table + snprintf_offset,
            table_capacity - snprintf_offset,
            "%c",
            new_line
        );
        snprintf_offset += line_end_written;

        // Increase the row counter
        row_counter++;
    }

    printf("%s \n", table);
}

void calcAndPrintMultiplications(
    const uint8_t *multiplications_base,
    const size_t multiplications_base_size,
    const uint8_t multiplications_base_mode // DEC, OCT, HEX
) {
    size_t multiplications_size = multiplications_base_size * multiplications_base_size;
    uint16_t multiplications[multiplications_size];
    calculateMultiplications(
        multiplications_base,
        multiplications_base_size,
        multiplications
    );
    printMultiplicationsAsTable(
        multiplications_base,
        multiplications_base_size,
        multiplications,
        multiplications_size,
        multiplications_base_mode
    );
}

// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

static uint32_t g_presses = 0U;
static uint32_t g_last_printed = 0U;
static uint32_t g_last_print_ms = 0U;

void setup(void) {
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    delay(1500U * 3); // My serial needed extra time to initialize.

    Serial.println();
    Serial.println("You can press the button now.");
}

void loop(void) {
    // No delay here on purpose: the loop runs as fast as it can, which is
    // exactly what lets it see every bounce.
    if (isButtonPressed()) {
        g_presses++;
    }

    const uint32_t now = millis();
    if ((now - g_last_print_ms) < PRINT_PERIOD_MS) {
        return;
    }
    g_last_print_ms = now;

    if (g_presses != g_last_printed) {
        // I will only allow pressing over 1, 2, 3 as depending on the press I need to show either DEC, OCT, or HEX table.
        if (g_presses > 3) {
            g_presses = 1;
        }
        if (g_presses == 1) {
            Serial.println("Mode: DEC");
            calcAndPrintMultiplications(decs, ARRAY_LENGTH(decs), MODE_DEC);
        }
        if (g_presses == 2) {
            Serial.println("Mode: OCT");
            calcAndPrintMultiplications(octs, ARRAY_LENGTH(octs), MODE_OCT);
        }
        if (g_presses == 3) {
            Serial.println("Mode: HEX");
            calcAndPrintMultiplications(hexs, ARRAY_LENGTH(hexs), MODE_HEX);
        }

        g_last_printed = g_presses;
    }
}
