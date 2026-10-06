// Lesson 11 - Hardware timer demo
// ESP32-C3 Super Mini, Arduino-ESP32 core 2.x (the version PlatformIO ships)
//
// A hardware timer fires an interrupt every 1 ms. The interrupt does two
// small things:
//   1. it measures its own timing with micros(): count, shortest and
//      longest interval since the last report
//   2. it sets a flag
// loop() turns the flag into a 1 Hz heartbeat on the green LED, and once
// per second copies the statistics inside a critical section and prints
// them.
//
// Serial output, once per second:
//   n=<ticks> min=<us> us max=<us> us jitter=<us> us
//
// Look closely at the first line after a reset.

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Pins
// ---------------------------------------------------------------------------

// RGB LED is common anode: LOW turns a channel ON, HIGH turns it OFF.
static const uint8_t PIN_LED_GREEN = 6U;

// ---------------------------------------------------------------------------
// Timer configuration (see slide "The arithmetic")
// ---------------------------------------------------------------------------

static const uint8_t  TIMER_NUM   = 0U;      // timer 0 = TIMG0
static const uint16_t TIMER_DIV   = 80U;     // 80 MHz / 80 = 1 MHz -> 1 us per tick
static const uint64_t ALARM_TICKS = 1000U;   // 1000 ticks x 1 us = 1 ms -> 1 kHz

// ---------------------------------------------------------------------------
// Application timing
// ---------------------------------------------------------------------------

static const uint32_t SERIAL_BAUD      = 115200U;
static const uint32_t USB_SETTLE_MS    = 2000U;  // give the USB serial port time to open
static const uint32_t TICKS_PER_TOGGLE = 500U;   // 500 ticks of 1 ms -> LED toggles twice a second
static const uint32_t REPORT_PERIOD_MS = 1000U;

// ---------------------------------------------------------------------------
// Shared between the ISR and loop()
// ---------------------------------------------------------------------------

static hw_timer_t* s_timer = nullptr;
static uint32_t s_lastReportMs = 0U;

portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;

volatile bool     g_tickFlag = false;
volatile uint32_t g_lastUs   = 0U;
volatile uint32_t g_minUs    = UINT32_MAX;
volatile uint32_t g_maxUs    = 0U;
volatile uint32_t g_count    = 0U;

// ---------------------------------------------------------------------------
// Interrupt service routine - keep it short
// ---------------------------------------------------------------------------

void IRAM_ATTR onTick() {
    const uint32_t now = static_cast<uint32_t>(micros());

    portENTER_CRITICAL_ISR(&g_mux);
    const uint32_t dt = now - g_lastUs;   // unsigned: correct across the micros() wrap
    g_lastUs = now;
    if (dt < g_minUs) { g_minUs = dt; }
    if (dt > g_maxUs) { g_maxUs = dt; }
    g_count++;
    portEXIT_CRITICAL_ISR(&g_mux);

    g_tickFlag = true;   // signal only - loop() does the work
}

// ---------------------------------------------------------------------------
// Work done in loop() context
// ---------------------------------------------------------------------------

// Called for every flag loop() sees. If loop() is busy for longer than 1 ms,
// several ticks fold into one flag and the heartbeat runs slow.
static void heartbeat() {
    static uint32_t s_ticksSeen = 0U;
    static bool s_ledOn = false;

    s_ticksSeen++;
    if (s_ticksSeen >= TICKS_PER_TOGGLE) {
        s_ticksSeen = 0U;
        s_ledOn = !s_ledOn;
        digitalWrite(PIN_LED_GREEN, s_ledOn ? LOW : HIGH);
    }
}

// Copy and reset inside the critical section, print outside it.
static void reportOncePerSecond() {
    portENTER_CRITICAL(&g_mux);      // the ISR has to wait
    const uint32_t n  = g_count;
    const uint32_t lo = g_minUs;
    const uint32_t hi = g_maxUs;
    g_count = 0U;
    g_minUs = UINT32_MAX;
    g_maxUs = 0U;
    portEXIT_CRITICAL(&g_mux);       // the ISR may run again

    if (n == 0U) {
        Serial.println("n=0 - the timer is not running");
        return;
    }
    Serial.printf("n=%lu min=%lu us max=%lu us jitter=%lu us\n",
                  static_cast<unsigned long>(n),
                  static_cast<unsigned long>(lo),
                  static_cast<unsigned long>(hi),
                  static_cast<unsigned long>(hi - lo));
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(USB_SETTLE_MS);
    Serial.println("Lesson 11 - hardware timer demo");

    pinMode(PIN_LED_GREEN, OUTPUT);
    digitalWrite(PIN_LED_GREEN, HIGH);   // off

    s_timer = timerBegin(TIMER_NUM, TIMER_DIV, true);   // true: count up
    if (s_timer == nullptr) {
        Serial.println("timerBegin failed - check TIMER_NUM (0 or 1 on the C3)");
        while (true) {
            delay(REPORT_PERIOD_MS);
        }
    }
    timerAttachInterrupt(s_timer, &onTick, false);      // false: level interrupt
    timerAlarmWrite(s_timer, ALARM_TICKS, true);        // true: auto-reload, repeat
    timerAlarmEnable(s_timer);                          // arm: interrupts start now

    s_lastReportMs = static_cast<uint32_t>(millis());   // first report one second after the timer starts
}

void loop() {
    if (g_tickFlag) {
        g_tickFlag = false;
        heartbeat();
    }

    const uint32_t nowMs = static_cast<uint32_t>(millis());
    if (nowMs - s_lastReportMs >= REPORT_PERIOD_MS) {
        s_lastReportMs = nowMs;
        reportOncePerSecond();
    }
}
