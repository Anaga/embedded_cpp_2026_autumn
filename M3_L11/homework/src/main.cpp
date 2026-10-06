#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pins
// ---------------------------------------------------------------------------

//`LED_BUILTIN`, GP25 on the original Pico
static const uint8_t PIN_LED_GREEN = 25U;

static const uint32_t SERIAL_BAUD      = 115200U;
static const uint32_t USB_SETTLE_MS    = 2000U;  // give the USB serial port time to open
static const uint32_t TICKS_PER_TOGGLE = 500U;   // 500 ticks of 1 ms -> LED toggles twice a second
static const uint32_t REPORT_PERIOD_MS = 1000U;

static uint32_t s_lastReportMs = 0U;
static uint32_t s_lastTickMs = 0U;

static void reportOncePerSecond(uint32_t uptime) {
    Serial.printf("Pico alive, uptime %d s \n", uptime);
}

static void togglePin(uint8_t pin_id){
    static bool s_ledOn = false;
    if (s_ledOn) {Serial.println("Tick");}
    else {Serial.println("Tack");}

    digitalWrite(pin_id, s_ledOn);
    s_ledOn = !s_ledOn;
}



void setup() {
    pinMode(PIN_LED_GREEN, OUTPUT); 
    Serial.begin(SERIAL_BAUD);
    delay(USB_SETTLE_MS);
    Serial.println("Lesson 11 - Pico home_task demo");
    s_lastReportMs = static_cast<uint32_t>(millis());   // first report one second after the timer starts
}

void loop() {
    const uint32_t nowMs = static_cast<uint32_t>(millis());
    if (nowMs - s_lastTickMs >= TICKS_PER_TOGGLE) {
        s_lastTickMs = nowMs;
        togglePin(PIN_LED_GREEN);
    }

    if (nowMs - s_lastReportMs >= REPORT_PERIOD_MS) {
        s_lastReportMs = nowMs;
        uint32_t uptime_sec = (uint32_t)(nowMs / 1000U);
        reportOncePerSecond(uptime_sec);
    }
}