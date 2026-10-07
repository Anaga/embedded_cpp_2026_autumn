/*
 * Taimer - timed watering and grow light
 * Board: ESP32-C3 Super Mini
 *
 * Wiring:
 *   PWM A (pump)  -> GPIO 6 and GND
 *   PWM B (light) -> GPIO 7 and GND
 *   potentiometer middle pin -> GPIO 4;
 *     outer pins -> 3V3 (through 3.3k), GND (through 100 ohm)
 *     measured on GPIO 4: 0.028 V .. 2.50 V
 *   Red LED, common anode -> 3V3; red GPIO 21 through 100 ohm (LOW = on)
 *
 * Serial monitor: 115200
 * Commands: w = water now, p = stop pump, s = status, h = help
 */

#include <Arduino.h>
#include <stdint.h>


// ---------------------------------------------------------------------------
// CONFIGURATION
// ---------------------------------------------------------------------------

// Pins.
// GPIO 8 and 9 are strapping pins, so the loads stay off them.
static const uint8_t PIN_PUMP  = 6U;   // PWM A
static const uint8_t PIN_LIGHT = 7U;   // PWM B
static const uint8_t PIN_POT   = 4U;   // ADC1 channel 4
static const uint8_t PIN_RED   = 21U;  // UART0 TX - flickers during the boot log

// The RGB LED is common anode: LOW turns the red segment on.
static const uint8_t RED_ON  = LOW;
static const uint8_t RED_OFF = HIGH;

// Time base. A hardware timer interrupt fires every TICK_MS and only counts.
// Everything else counts ticks. The potentiometer is sampled once per tick.
static const uint32_t TICK_MS    = 20U;
static const uint8_t  TICK_TIMER = 0U;  // general purpose timer 0 of 2

// Watering.
static const uint32_t WATER_INTERVAL_MIN = 60U;    // start to start
static const bool     WATER_ON_BOOT      = true;   // first watering right after power-on
static const uint32_t WATER_SEC          = 300U;   // fixed length for now (5 min)
static const uint32_t WATER_MIN_SEC      = 60U;    // knob at the low end
static const uint32_t WATER_MAX_SEC      = 360U;   // knob at the high end
static const uint32_t WATER_STEP_SEC     = 10U;    // knob resolution

// Pump.
static const uint32_t PUMP_DUTY_PERCENT   = 100U;   // after the soft start
static const uint32_t PUMP_RAMP_MS        = 1000U;  // soft start 0 -> PUMP_DUTY_PERCENT
static const uint32_t PUMP_HARD_LIMIT_SEC = 420U;   // on longer -> fault, off until reset

// Light. The cycle starts with a sunrise at power-on.
static const uint32_t LIGHT_ON_HOURS    = 14U;
static const uint32_t LIGHT_OFF_HOURS   = 10U;
static const uint32_t LIGHT_FADE_MIN    = 15U;   // sunrise and sunset, each
static const uint32_t LIGHT_MAX_PERCENT = 100U;  // brightness at full day

// PWM. LEDC channels 0 and 1 share one timer; 0 and 2 keep them independent.
static const uint8_t  CH_LIGHT     = 0U;
static const uint8_t  CH_PUMP      = 2U;
static const uint32_t LIGHT_PWM_HZ = 1000U;
static const uint32_t PUMP_PWM_HZ  = 1000U;  // keep <= 1 kHz for opto-isolated modules
static const uint8_t  PWM_BITS     = 12U;
static const uint32_t PWM_MAX      = (1UL << PWM_BITS) - 1UL;

// Potentiometer range. The ADC reads linearly up to about 2500 mV at 12 dB,
// and the 3.3k / 10k / 100R divider spans 28..2500 mV, so the whole knob
// travel is usable. Measured: 0 mV fully down, 2490..2510 mV fully up.
// The defaults sit inside both ends with margin.
// Calibrate: turn the knob to each end, press 's', copy the mV values here.
static const uint32_t POT_MV_LOW   = 100U;
static const uint32_t POT_MV_HIGH  = 2400U;
static const bool     POT_REVERSED = false;  // true if clockwise should mean shorter

// Potentiometer smoothing.
static const uint8_t  SMOOTH_SAMPLES      = 16U;    // moving average, one sample per tick
static const uint32_t POT_PRINT_PERIOD_MS = 1000U;  // knob changes printed at most this often

// Status LED.
static const uint32_t HEARTBEAT_PERIOD_MS = 5000U;
static const uint32_t HEARTBEAT_ON_MS     = 100U;
static const uint32_t FAULT_BLINK_MS      = 200U;

// Serial.
static const uint32_t SERIAL_BAUD    = 115200U;
static const uint32_t SERIAL_WAIT_MS = 1500U;  // USB CDC needs a moment after reset


// ---------------------------------------------------------------------------
// CONFIGURATION CHECKS - a bad value stops the build, not the pump
// ---------------------------------------------------------------------------

#if ESP_ARDUINO_VERSION_MAJOR != 2
#error "Written for Arduino core 2.x (ledcSetup, timerBegin with a divider)."
#endif

// lolin_c3_mini sets this. Without it Serial is UART0, whose TX is GPIO 21.
#if !ARDUINO_USB_CDC_ON_BOOT
#error "Serial must run over USB CDC: UART0 TX is GPIO 21, the red LED."
#endif

static_assert((1000U % TICK_MS) == 0U, "TICK_MS must divide 1000");
static_assert(PUMP_RAMP_MS >= TICK_MS, "soft start shorter than one tick");
static_assert(HEARTBEAT_ON_MS >= TICK_MS, "heartbeat flash shorter than one tick");
static_assert(FAULT_BLINK_MS >= TICK_MS, "fault blink shorter than one tick");
static_assert(POT_PRINT_PERIOD_MS >= TICK_MS, "print period shorter than one tick");
static_assert(SMOOTH_SAMPLES > 0U, "need at least one sample");

static_assert(WATER_MIN_SEC < WATER_MAX_SEC, "watering range is empty");
static_assert((WATER_MIN_SEC % WATER_STEP_SEC) == 0U, "min must be a multiple of the step");
static_assert((WATER_MAX_SEC % WATER_STEP_SEC) == 0U, "max must be a multiple of the step");
static_assert(PUMP_HARD_LIMIT_SEC > (WATER_MAX_SEC + (PUMP_RAMP_MS / 1000U) + 1U),
              "hard limit must exceed the longest normal watering");
static_assert((WATER_INTERVAL_MIN * 60U) > PUMP_HARD_LIMIT_SEC,
              "interval must be longer than any watering");
static_assert((WATER_SEC >= WATER_MIN_SEC) && (WATER_SEC <= WATER_MAX_SEC),
              "WATER_SEC outside the watering range");
static_assert(PUMP_DUTY_PERCENT <= 100U, "pump duty above 100 percent");

static_assert(LIGHT_MAX_PERCENT <= 100U, "brightness above 100 percent");
static_assert(LIGHT_FADE_MIN > 0U, "use 1 for a near-instant switch");
static_assert((2U * LIGHT_FADE_MIN) <= (LIGHT_ON_HOURS * 60U), "fades longer than the day");

static_assert((CH_LIGHT / 2U) != (CH_PUMP / 2U), "PWM channels would share an LEDC timer");
static_assert(POT_MV_LOW < POT_MV_HIGH, "pot range reversed - use POT_REVERSED instead");


// ---------------------------------------------------------------------------
// Step 3: the pump runs WATER_SEC every WATER_INTERVAL_MIN. The light stays off.
// Next steps: knob sets the length, light, status LED, commands.
// ---------------------------------------------------------------------------

static bool     pumpOn      = false;
static bool     pumpFault   = false;
static uint32_t pumpStartMs = 0U;   // when the current watering began
static uint32_t lastWaterMs = 0U;   // start of the last watering, for the schedule

static void setPwmPercent(uint8_t channel, uint32_t percent) {
    ledcWrite(channel, (percent * PWM_MAX + 50U) / 100U);
}

static void pumpStart(uint32_t now) {
    pumpOn = true;
    pumpStartMs = now;
    lastWaterMs = now;
    Serial.printf("[%lu s] pump on for %lu s\n",
                  (unsigned long)(now / 1000U), (unsigned long)WATER_SEC);
}

static void pumpStop(uint32_t now) {
    pumpOn = false;
    setPwmPercent(CH_PUMP, 0U);
    Serial.printf("[%lu s] pump off\n",(unsigned long)(now / 1000U));
}

void setup() {
    // Loads off before anything else, so nothing switches on during startup.
    pinMode(PIN_PUMP, OUTPUT);
    digitalWrite(PIN_PUMP, LOW);
    pinMode(PIN_LIGHT, OUTPUT);
    digitalWrite(PIN_LIGHT, LOW);
    pinMode(PIN_RED, OUTPUT);
    digitalWrite(PIN_RED, RED_OFF);

    Serial.begin(SERIAL_BAUD);
    delay(SERIAL_WAIT_MS);

    ledcSetup(CH_PUMP, PUMP_PWM_HZ, PWM_BITS);
    ledcAttachPin(PIN_PUMP, CH_PUMP);
    ledcSetup(CH_LIGHT, LIGHT_PWM_HZ, PWM_BITS);
    ledcAttachPin(PIN_LIGHT, CH_LIGHT);
    ledcWrite(CH_PUMP, 0U);
    ledcWrite(CH_LIGHT, 0U);

    Serial.println("Taimer - step 3: pump on a timer, light off");

    uint32_t now = millis();
    lastWaterMs = now;
    if (WATER_ON_BOOT) {
        pumpStart(now);
    }
}

void loop() {
    static uint32_t lastTick = 0U;

    uint32_t now = millis();
    if ((now - lastTick) < TICK_MS) {
        return;
    }
    lastTick = now;

    if (pumpFault) {
        return;  // output already off, stays off until reset
    }

    // Unsigned subtraction keeps this right across the 49-day millis() wrap.
    if (pumpOn) {
        uint32_t onMs = now - pumpStartMs;

        if (onMs >= (PUMP_HARD_LIMIT_SEC * 1000U)) {
            pumpFault = true;
            pumpStop(now);
            Serial.println("FAULT: pump on too long, off until reset");
            return;
        }
        if (onMs >= (WATER_SEC * 1000U)) {
            pumpStop(now);
            return;
        }
        // Soft start, then full duty.
        uint32_t duty = PUMP_DUTY_PERCENT;
        if (onMs < PUMP_RAMP_MS) {
            duty = (PUMP_DUTY_PERCENT * onMs) / PUMP_RAMP_MS;
        }
        setPwmPercent(CH_PUMP, duty);
    } else if ((now - lastWaterMs) >= (WATER_INTERVAL_MIN * 60000U)) {
        pumpStart(now);
    }
}
