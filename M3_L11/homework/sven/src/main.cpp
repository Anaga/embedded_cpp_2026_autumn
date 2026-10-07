// Blink the onboard LED and report uptime without blocking.

#include <Arduino.h>

static const uint8_t LED_PIN = LED_BUILTIN; // HIGH = ON
static const uint32_t SERIAL_BAUD = 115200U;
static const uint32_t BLINK_INTERVAL_MS = 500U;
static const uint32_t REPORT_INTERVAL_MS = 1000U;
static const uint32_t MS_PER_SECOND = 1000U;

static uint32_t lastBlinkMs = 0U;
static uint32_t lastReportMs = 0U;
static uint32_t startupCpuFrequencyHz = 0U;
static bool ledOn = true;
static bool cpuFrequencyPrinted = false;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1500U);
    startupCpuFrequencyHz = rp2040.f_cpu();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    lastBlinkMs = millis();
    lastReportMs = lastBlinkMs;
}

void loop() {
    const uint32_t nowMs = millis();

    // Unsigned subtraction keeps the timers working across millis() rollover.
    if (nowMs - lastBlinkMs >= BLINK_INTERVAL_MS) {
        lastBlinkMs = nowMs;
        ledOn = !ledOn;
        digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    }

    // Print startup information once USB serial connects, without waiting for it.
    if (!cpuFrequencyPrinted && Serial) {
        Serial.print("CPU clock: ");
        Serial.print(startupCpuFrequencyHz);
        Serial.println(" Hz");
        cpuFrequencyPrinted = true;
    }

    if (nowMs - lastReportMs >= REPORT_INTERVAL_MS) {
        lastReportMs = nowMs;
        Serial.print("Pico alive, uptime ");
        Serial.print(nowMs / MS_PER_SECOND);
        Serial.println(" s");
    }
}
