#pragma once
#include <Arduino.h>
#include <stdint.h>

#if ESP_ARDUINO_VERSION_MAJOR != 2
#error "PISI demo requires Arduino-ESP32 core 2.x (LEDC API)."
#endif
#if !ARDUINO_USB_CDC_ON_BOOT
#error "Enable USB CDC on boot. GPIO20 is reserved for optional digital light sensor."
#endif

namespace cfg {
constexpr uint8_t PIN_PUMP = 3U;
constexpr uint8_t PIN_LIGHT = 10U;
constexpr uint8_t PIN_POT = 4U;
constexpr uint8_t PIN_RGB_RED = 5U;
constexpr uint8_t PIN_RGB_GREEN = 6U;
constexpr uint8_t PIN_RGB_BLUE = 7U;
constexpr uint8_t PIN_AHT_SDA = 0U;
constexpr uint8_t PIN_AHT_SCL = 1U;
constexpr uint8_t PIN_LIGHT_SENSOR_DO = 20U;
constexpr bool LIGHT_SENSOR_INSTALLED = false;

constexpr uint8_t CH_LIGHT = 0U;
constexpr uint8_t CH_PUMP = 2U;
constexpr uint32_t PWM_HZ = 1000UL;
constexpr uint8_t PWM_BITS = 12U;
constexpr uint32_t PWM_MAX = (1UL << PWM_BITS) - 1UL;

// DEMONSTRATION MODE. BOTH schedules begin at startup.
constexpr uint32_t PUMP_ON_MS = 30UL * 1000UL;
constexpr uint32_t PUMP_OFF_MS = 90UL * 1000UL;
constexpr uint32_t PUMP_CYCLE_MS = PUMP_ON_MS + PUMP_OFF_MS;
constexpr uint32_t PUMP_HARD_LIMIT_MS = 40UL * 1000UL;
constexpr bool PUMP_PWM_RAMP = true; // User confirmed it works on their pump.
constexpr uint32_t PUMP_RAMP_MS = 1000UL;
constexpr uint32_t PUMP_START_DUTY_PERCENT = 65UL;

constexpr uint32_t LIGHT_ON_MS = 120UL * 1000UL;
constexpr uint32_t LIGHT_OFF_MS = 60UL * 1000UL;
constexpr uint32_t LIGHT_CYCLE_MS = LIGHT_ON_MS + LIGHT_OFF_MS;
constexpr uint32_t LIGHT_FADE_MS = 10UL * 1000UL;

constexpr float TEMP_LIGHTS_OFF_C = 28.0f;
constexpr float TEMP_LIGHTS_ON_C = 26.0f;
constexpr uint32_t TEMP_MIN_OFF_MS = 300UL * 1000UL;
constexpr uint32_t TEMP_SAMPLE_MS = 2000UL;
constexpr uint32_t SENSOR_RETRY_MS = 5000UL;

constexpr uint32_t HEARTBEAT_PERIOD_MS = 1000UL;
constexpr uint32_t HEARTBEAT_ON_MS = 100UL;
constexpr uint32_t RGB_BOTH_SEGMENT_MS = 200UL; // blue on 200, off 200, green on 200, off 200
constexpr uint32_t FAULT_BLINK_MS = 200UL; // red toggles every 200 ms
constexpr uint32_t SERIAL_BAUD = 115200UL;
constexpr uint32_t STATUS_PERIOD_MS = 5000UL;
constexpr uint32_t STATS_PERIOD_MS = 5000UL;

constexpr uint32_t POT_MV_LOW = 100UL;
constexpr uint32_t POT_MV_HIGH = 2400UL;
constexpr uint32_t POT_MIN_SEC = 60UL;
constexpr uint32_t POT_MAX_SEC = 360UL;
constexpr uint32_t POT_STEP_SEC = 10UL;

static_assert(PUMP_ON_MS < PUMP_HARD_LIMIT_MS, "Pump cutoff must be later than normal stop");
static_assert(PUMP_HARD_LIMIT_MS < PUMP_CYCLE_MS, "Cutoff must be inside pump cycle");
static_assert(2UL * LIGHT_FADE_MS <= LIGHT_ON_MS, "Light fades overlap");
static_assert(PUMP_START_DUTY_PERCENT <= 100UL, "Invalid pump PWM start duty");
}
