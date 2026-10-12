/*
 * Lesson 12 - homework
 * One Pico and two UARTs - pot and led
 */

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "LineReader.h"

static const uint32_t SERIAL_BAUD = 115200U;
static const uint32_t SERIAL_WAIT_MS = 5000U;

static const uint8_t PIN_POT = 26U;
static const uint16_t MAX_POT_READING = 1023U;
static const uint32_t POT_REQUEST_INTERVAL_MS = 100U;
static const uint32_t TIMEOUT_MS = 200U;

static const uint32_t PRINT_INTERVAL_MS = 1000U;
static const uint32_t MS_PER_S = 1000U;
static const uint32_t HZ_PER_MHZ = 1000000U;

static const uint32_t HALF_MIN_MS = 50U;
static const uint32_t HALF_MAX_MS = 500U;

static uint32_t last_print_ms = 0U;
static uint32_t last_blink_ms = 0U;
static bool led_on = false;

static LineReader pot_reader;
static LineReader led_reader;

enum class MasterState : uint8_t {
    Idle,
    Waiting
};

static MasterState g_master_state = MasterState::Idle;
static uint32_t g_pot_value = 0U;
static uint32_t last_pot_request_ms = 0U;
static bool g_link_ok = true;

static void handlePotSide(void) {
    if (pot_reader.readLine(Serial1) && strcmp(pot_reader.line(), "POT?") == 0) {
        Serial1.printf("POT %u\n", (unsigned)analogRead(PIN_POT));
    }
}

static void handleLedSide(void) {
    uint32_t now = millis();
    switch (g_master_state) {
        case MasterState::Idle:
            while (led_reader.readLine(Serial2)) {}
            if (now - last_pot_request_ms >= POT_REQUEST_INTERVAL_MS) {
                last_pot_request_ms = now;
                Serial2.println("POT?");
                g_master_state = MasterState::Waiting;
            }
            break;
        case MasterState::Waiting:
            if (led_reader.readLine(Serial2) && strncmp(led_reader.line(), "POT ", 4U) == 0) {
                char* end = nullptr;
                const char* number = led_reader.line() + 4U;
                const uint32_t value = strtoul(number, &end, 10);
                if (end != number && *end == '\0' && value <= MAX_POT_READING) {
                    g_pot_value = value;
                    g_link_ok = true;
                    g_master_state = MasterState::Idle;
                }
            } else if (now - last_pot_request_ms >= TIMEOUT_MS) {
                g_link_ok = false;
                g_master_state = MasterState::Idle;
            }

            break;
    }
}


void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(SERIAL_BAUD);
    Serial1.begin(SERIAL_BAUD);
    Serial2.begin(SERIAL_BAUD);
}

void loop(void) {
    uint32_t now = millis();
    uint32_t blink_half_interval = HALF_MIN_MS + g_pot_value * (HALF_MAX_MS - HALF_MIN_MS) / MAX_POT_READING;

    if (last_blink_ms == 0U) {
        last_blink_ms = now;
    } else if (now - last_blink_ms >= blink_half_interval && g_link_ok)
    {
        last_blink_ms = now;
        led_on = !led_on;
        digitalWrite(LED_BUILTIN, led_on ? HIGH : LOW);
    } else if (!g_link_ok) {
        led_on = false;
        digitalWrite(LED_BUILTIN, led_on ? HIGH : LOW);
    }

    if (last_print_ms == 0U) {
        last_print_ms = now;
    } else if (now - last_print_ms >= PRINT_INTERVAL_MS)
    {
        last_print_ms += PRINT_INTERVAL_MS;
        // Serial.printf("Blink interval: %4lu ms  Value: %4u     State is %s, link %s\n", blink_half_interval, (unsigned) g_pot_value, g_master_state == MasterState::Idle ? "Idle" : "Waiting", g_link_ok ? "OK" : "Not OK");
        Serial.printf("Blink interval: %4lu ms  Value: %4u     link %s\n", blink_half_interval, (unsigned) g_pot_value, g_link_ok ? "OK" : "Not OK");
    }

    handlePotSide();
    handleLedSide();


}