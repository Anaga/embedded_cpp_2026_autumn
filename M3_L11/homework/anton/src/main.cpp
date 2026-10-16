// Lesson 11 - first Pico build: blink and report uptime, no delay() in loop.

#include <Arduino.h>
#include <stdint.h>

static const uint8_t LED_PIN = LED_BUILTIN;  // GP25 on the original Pico
static const uint32_t SERIAL_BAUD = 115200U;
static const uint32_t BLINK_HALF_PERIOD_MS = 500U;  // 500 ms on, 500 ms off
static const uint32_t REPORT_PERIOD_MS = 1000U;     // one line per second
static const uint32_t MS_PER_SECOND = 1000U;

static uint32_t last_blink_ms = 0U;
static uint32_t last_report_ms = 0U;
static bool led_on = false;

void setup() {
  Serial.begin(SERIAL_BAUD);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Bonus: print the CPU clock frequency once at start-up
  Serial.printf("CPU clock: %lu Hz\n",
                (unsigned long)rp2040.f_cpu());

  last_blink_ms = millis();
  last_report_ms = millis();
}

void loop() {
  const uint32_t now_ms = millis();

  // Unsigned subtraction keeps the timers working across millis() rollover
  if (now_ms - last_blink_ms >= BLINK_HALF_PERIOD_MS) {
    last_blink_ms = now_ms;
    led_on = !led_on;
    digitalWrite(LED_PIN, led_on ? HIGH : LOW);
  }

  if (now_ms - last_report_ms >= REPORT_PERIOD_MS) {
    last_report_ms = now_ms;
    Serial.printf("Pico alive, uptime %lu s\n",
                  (unsigned long)(now_ms / MS_PER_SECOND));
  }
}
