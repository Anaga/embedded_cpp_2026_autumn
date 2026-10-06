/*
 * Lesson 11 - homework
 * Pico blink + uptime print, no delay()
 */

#include <Arduino.h>
#include <stdint.h>

static const uint32_t SERIAL_BAUD = 115200U;
static const uint32_t SERIAL_WAIT_MS = 3000U;
static const uint32_t BLINK_INTERVAL_MS = 500U;
static const uint32_t PRINT_INTERVAL_MS = 1000U;
static const uint32_t MS_PER_S = 1000U;
static const uint32_t HZ_PER_MHZ = 1000000U;

static uint32_t last_blink_ms = 0U;
static uint32_t last_print_ms = 0U;
static bool led_on = false;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(SERIAL_BAUD);
    // USB serial: wait a bit for the monitor so the first line is not lost
    while (!Serial && millis() < SERIAL_WAIT_MS)
    {
    }

    // bonus
    Serial.printf("CPU clock: %lu MHz\n", rp2040.f_cpu() / HZ_PER_MHZ);
}

void loop()
{
    uint32_t now = millis();

    if (now - last_blink_ms >= BLINK_INTERVAL_MS)
    {
        last_blink_ms += BLINK_INTERVAL_MS;
        led_on = !led_on;
        digitalWrite(LED_BUILTIN, led_on ? HIGH : LOW);
    }

    if (now - last_print_ms >= PRINT_INTERVAL_MS)
    {
        last_print_ms += PRINT_INTERVAL_MS;
        Serial.printf("Pico alive, uptime %lu s\n", now / MS_PER_S);
    }
}
