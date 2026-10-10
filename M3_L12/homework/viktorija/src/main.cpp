/*
 * Lesson 12 - homework
 * One Pico and two UARTs - pot and led
 */

#include <Arduino.h>
#include <stdint.h>

#include "RingBuffer.h"

static const uint32_t SERIAL_BAUD = 115200U;
static const uint32_t SERIAL_WAIT_MS = 5000U;

static const uint8_t PIN_POT = 26U;
static const uint16_t MAX_POT_READING = 1023U;
// Potentiometer smoothing.
// static const uint8_t SMOOTH_SAMPLES = 16U;
// static const uint32_t SAMPLE_PERIOD_MS = 20U;
// static const uint32_t POT_PRINT_PERIOD_MS = 1000U;

static const uint32_t PRINT_INTERVAL_MS = 1000U;
static const uint32_t MS_PER_S = 1000U;
static const uint32_t HZ_PER_MHZ = 1000000U;

static const uint32_t HALF_MIN_MS = 50U;
static const uint32_t HALF_MAX_MS = 500U;

static uint32_t last_print_ms = 0U;
static uint32_t last_blink_ms = 0U;
static bool led_on = false;

// The last 16 potentiometer readings: 32 bytes of data, fixed at compile time.
// static RingBuffer<uint16_t, SMOOTH_SAMPLES> g_pot_samples;

// Smooth potentiometer function. Not using it yet. Or maybe at all.
// static void runPotentiometer(void) {
//     static uint32_t last_sample_ms = 0U;
//     static uint32_t last_print_ms = 0U;
//     static uint16_t last_raw = 0U;

//     const uint32_t now = millis();

//     // One new reading per period, not sixteen in a row.
//     if ((now - last_sample_ms) >= SAMPLE_PERIOD_MS) {
//         last_sample_ms = now;
//         last_raw = (uint16_t)analogRead(PIN_POT);
//         g_pot_samples.push(last_raw);
//     }

//     if ((now - last_print_ms) < POT_PRINT_PERIOD_MS) {
//         return;
//     }
//     last_print_ms = now;

//     uint32_t sum = 0U;
//     uint16_t lowest = 0xFFFFU;
//     uint16_t highest = 0U;
//     for (uint8_t i = 0U; i < g_pot_samples.count(); i++) {
//         const uint16_t v = g_pot_samples.at(i);
//         sum += v;
//         lowest = (v < lowest) ? v : lowest;
//         highest = (v > highest) ? v : highest;
//     }
//     const uint16_t smooth = (uint16_t)(sum / g_pot_samples.count());

//     Serial.printf("[pot]   raw %4u   smooth %4u   last 16 spread over %u counts\n",
//                   (unsigned)last_raw, (unsigned)smooth, (unsigned)(highest - lowest));
// }


void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(SERIAL_BAUD);
    Serial1.begin(SERIAL_BAUD);
    Serial2.begin(SERIAL_BAUD);
    // USB serial: wait a bit for the monitor so the first line is not lost
    while (!Serial && millis() < SERIAL_WAIT_MS)
    {
    }

    // Fill the smoothing buffer once so the first average is not over one sample.
    // for (uint8_t i = 0U; i < SMOOTH_SAMPLES; i++) {
    //     g_pot_samples.push((uint16_t)analogRead(PIN_POT));
    // }

    // this line did not print...
    Serial.printf("CPU clock: %lu MHz\n", rp2040.f_cpu() / HZ_PER_MHZ);
}

void loop(void) {
    uint32_t now = millis();
    uint16_t pot_reading = (uint16_t)analogRead(PIN_POT);
    uint32_t blink_half_interval = HALF_MIN_MS + pot_reading * (HALF_MAX_MS - HALF_MIN_MS) / MAX_POT_READING;

    if (last_blink_ms == 0U) {
        last_blink_ms = now;
    } else if (now - last_blink_ms >= blink_half_interval)
    {
        last_blink_ms = now;
        led_on = !led_on;
        digitalWrite(LED_BUILTIN, led_on ? HIGH : LOW);
    }

    if (now - last_print_ms >= PRINT_INTERVAL_MS)
    {
        last_print_ms += PRINT_INTERVAL_MS;
        Serial.printf("Pico alive, uptime %lu sec, pot: %u, blink half interval: %lu ms\n", now / MS_PER_S, (unsigned)pot_reading, blink_half_interval);
        Serial1.println("POT?");
    }

    // while (Serial1.available() > 0) {
    //     Serial.println(Serial1.read());
    // }
    while (Serial2.available() > 0) {
        Serial.println(Serial2.read());
    }
}