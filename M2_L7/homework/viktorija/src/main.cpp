
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
// TABLE CALCULATION VARIABLES
// ---------------------------------------------------------------------------

static const uint8_t SIZE = 16U;
static uint16_t table[SIZE][SIZE];
static const char *MODE_NAMES[] = {"OCT", "DEC", "HEX"};
static const char *MODES[] = {"%4o", "%4u", "%4X"};
static const char *MODE_DOTS[] = {"\U0001F535", "\U0001F7E2", "\U0001F534"};
static const uint8_t MODE_COUNT = sizeof(MODES) / sizeof(MODES[0]);

// Label cell + " |" + SIZE cells of 4 characters + the closing zero.
static const uint8_t CELL_WIDTH = 4U;
static const uint8_t LINE_SIZE = ((SIZE + 1U) * CELL_WIDTH) + 2U + 1U;

static uint8_t mode = 0U;


// ---------------------------------------------------------------------------
// HARDWARE VAIRABLES
// ---------------------------------------------------------------------------

static const uint8_t BUTTON_PIN = 0U;
static const uint32_t DEBOUNCE_MS = 30U;

static const uint8_t LED_RED_PIN = 5U;
static const uint8_t LED_GREEN_PIN = 6U;
static const uint8_t LED_BLUE_PIN = 7U;

static const uint8_t BIT_RED = 0x01U;
static const uint8_t BIT_GREEN = 0x02U;
static const uint8_t BIT_BLUE = 0x04U;

static const uint8_t COLOUR_RED = BIT_RED;
static const uint8_t COLOUR_GREEN = BIT_GREEN;
static const uint8_t COLOUR_CYAN = BIT_GREEN | BIT_BLUE;

static const uint8_t MODE_COLOURS[MODE_COUNT] = { COLOUR_CYAN, COLOUR_GREEN, COLOUR_RED };

/* From lesson 06. Switch the three channels from the three bits. LOW is ON. */
static void setColour(uint8_t colour) {
    digitalWrite(LED_RED_PIN, ((colour & BIT_RED) != 0U) ? LOW : HIGH);
    digitalWrite(LED_GREEN_PIN, ((colour & BIT_GREEN) != 0U) ? LOW : HIGH);
    digitalWrite(LED_BLUE_PIN, ((colour & BIT_BLUE) != 0U) ? LOW : HIGH);
}

// ---------------------------------------------------------------------------
// TABLE CALCULATION AND PRINTING FUNCTIONS
// ---------------------------------------------------------------------------

static void calculate_table(uint16_t destination_table[][SIZE]) {
    for (uint8_t i = 1; i <= SIZE; i++) {
        for (uint8_t j = 1; j <= SIZE; j++) {
            destination_table[i - 1][j - 1] = (uint16_t)i * j;
        }
    }
}

/*
 * I admit, AI wrote this one
 */
static void format_row(char *buffer, size_t size, uint8_t row, uint8_t mode_number) {
    // Write the row label at the start; pos becomes how many characters it took.
    size_t pos = snprintf(buffer, size, MODES[mode_number], row);
    // Append the separator: buffer + pos is the first free slot, size - pos the room left.
    pos += snprintf(buffer + pos, size - pos, " |");

    for (uint8_t column = 0U; column < SIZE; column++) {
        // Append one cell and step pos past it, so the next cell lands after this one.
        pos += snprintf(buffer + pos, size - pos, MODES[mode_number], table[row - 1U][column]);
    }
}

static void print_table(uint8_t mode_number) {
    Serial.printf("\nMode: %s %s\n      ", MODE_NAMES[mode_number], MODE_DOTS[mode_number]);
    for (uint8_t i = 1; i <= SIZE; i++) {
        Serial.printf(MODES[mode_number], i);
    }
    Serial.printf("\n      ----------------------------------------------------------------\n");
    char line[LINE_SIZE];
    for (uint8_t i = 1; i <= SIZE; i++) {
        format_row(line, sizeof(line), i, mode_number);
        Serial.println(line);
    }
}

// ---------------------------------------------------------------------------
// BUTTON FUNCTIONS
// ---------------------------------------------------------------------------

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



// ---------------------------------------------------------------------------
// ENTRY POINTS
// ---------------------------------------------------------------------------

void setup(void) {
    Serial.begin(115200);
    delay(1500U);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);


    Serial.println();
    Serial.println("Multiplication table in OCT, DEC and HEX modes.");
    Serial.println("Press the button to change the mode.");

    calculate_table(table);
    print_table(mode);
    setColour(MODE_COLOURS[mode]);

}

void loop(void) {
    if (!isButtonPressed()) {
        return;
    }

    mode = (uint8_t)((mode + 1U) % MODE_COUNT);
    print_table(mode);
    setColour(MODE_COLOURS[mode]);

}
