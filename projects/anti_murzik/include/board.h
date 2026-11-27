// Pins and radio settings for the ESP32-C3 Super Mini (LOLIN C3 Mini)
// Anti-Murzik robot with DRV8833 motor driver
#pragma once
#include <Arduino.h>
#include <WiFi.h>

#if !defined(CONFIG_IDF_TARGET_ESP32C3)
#error "This project is written for the ESP32-C3 Super Mini"
#endif

// ============================================================================
// I2C (AHT10 sensor) - GPIO 8 (SDA), GPIO 9 (SCL) are strapping pins
// The AHT10 module pull-ups keep both HIGH during reset (normal boot)
// GPIO 9 is also the BOOT button: pressing it spoils a reading
// The on-board blue LED on GPIO 8 flickers with every I2C transfer
// ============================================================================
constexpr int PIN_SDA = 8;
constexpr int PIN_SCL = 9;

// ============================================================================
// RGB LED, common anode to 3V3, LOW = on (as in K3)
//   green = status LED (controlled by button/console/bot)
//   blue  = heartbeat: blinks 4 times a second while loop() is alive
//   red   = error indicator
// ============================================================================
constexpr int PIN_LED_R = 5;
constexpr int PIN_LED_G = 6;
constexpr int PIN_LED_B = 7;

// ============================================================================
// Button A, INPUT_PULLUP, button to GND, active low (as in K3)
// GPIO 0 is also a strapping pin (must be HIGH at boot)
// ============================================================================
constexpr int PIN_BUTTON_A = 0;

// ============================================================================
// DRV8833 Motor Driver (H-bridge for 2 DC motors)
// Motor A (drive): IN1 = forward, IN2 = backward
// Motor B (steer): IN3 = left, IN4 = right
// ============================================================================
constexpr int PIN_DRIVE_FWD = 10;   // IN1 (DRV8833: forward)
constexpr int PIN_DRIVE_REV = 9;    // IN2 (DRV8833: backward)
constexpr int PIN_STEER_LEFT = 3;   // IN3 (DRV8833: steer left)
constexpr int PIN_STEER_RIGHT = 4;  // IN4 (DRV8833: steer right)

// ============================================================================
// Headlights and Water Pump (reserved pins)
// ============================================================================
constexpr int PIN_HEADLIGHTS = 6;  // Headlights (reserved, conflicts with LED_G)
constexpr int PIN_PUMP = 5;        // Water pump (reserved, conflicts with LED_R)

// ============================================================================
// Distance Sensor (HC-SR04)
// ============================================================================
constexpr int PIN_DIST_TRIG = 18;  // TRIG pin
constexpr int PIN_DIST_ECHO = 19;  // ECHO pin

// ============================================================================
// PWM Settings
// ============================================================================
constexpr int PWM_FREQ = 1000;
constexpr int PWM_RES = 8;

// ============================================================================
// WiFi Settings
// ============================================================================
constexpr bool         LIMIT_TX_POWER = true;
constexpr wifi_power_t TX_POWER       = WIFI_POWER_15dBm;

// ============================================================================
// MDNS hostname
// ============================================================================
constexpr char MDNS_HOST[] = "antimurzik";

// ============================================================================
// Timing Constants
// ============================================================================
constexpr uint32_t HEARTBEAT_INTERVAL_MS = 1000;
constexpr uint32_t SNAPSHOT_PERIOD_MS = 100;
constexpr uint32_t LOOP_STATS_PERIOD_MS = 5000;
constexpr uint32_t PUMP_SPRAY_DURATION_MS = 600;
constexpr uint32_t PUMP_COOLDOWN_MS = 3000;
constexpr uint32_t HEARTBEAT_HALF_MS = 125;

// ============================================================================
// Fixed-width types
// ============================================================================
using uint8 = uint8_t;
using int8 = int8_t;
using uint16 = uint16_t;
using int16 = int16_t;
using uint32 = uint32_t;
using int32 = int32_t;
using float32 = float;