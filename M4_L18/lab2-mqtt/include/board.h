// Pins and radio settings per board.
#pragma once
#include <Arduino.h>
#include <WiFi.h>

#if defined(CONFIG_IDF_TARGET_ESP32C3)
// ESP32-C3 Super Mini - course pin map from session 18:
//   I2C  SDA GPIO 8, SCL GPIO 9 (the Wire defaults of lolin_c3_mini)
//   RGB  R 5, G 6, B 7 - common anode to 3V3, LOW = on (as in K3)
//
// GPIO 8 and GPIO 9 are strapping pins. The pull-ups on the AHT10 module
// keep both HIGH during reset, which is the normal boot from flash.
//   - GPIO 9 is also the BOOT button: pressing it during a reading pulls
//     SCL low and that reading fails (the code recovers).
//   - Anything that holds SDA or SCL LOW during reset can stop the board
//     from booting.
//   - The on-board blue LED sits on GPIO 8 (SDA): it flickers with every
//     I2C transfer - an I2C activity indicator. It cannot be used as a
//     normal LED while I2C runs.
constexpr const char *BOARD_NAME = "ESP32-C3 Super Mini";
constexpr int  PIN_SDA = 8;
constexpr int  PIN_SCL = 9;
constexpr bool HAS_RGB = true;
constexpr int  PIN_LED_R = 5;
constexpr int  PIN_LED_G = 6;
constexpr int  PIN_LED_B = 7;
constexpr int  PIN_LED = -1;          // no free single LED
constexpr bool LED_ACTIVE_LOW = true;
// Many Super Mini boards connect only with reduced TX power.
// If a board cannot connect, try false or another wifi_power_t value.
constexpr bool         LIMIT_TX_POWER = true;
constexpr wifi_power_t TX_POWER = WIFI_POWER_8_5dBm;

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
constexpr const char *BOARD_NAME = "Lolin S2 Mini";
constexpr int  PIN_SDA = 33;
constexpr int  PIN_SCL = 35;
constexpr bool HAS_RGB = false;
constexpr int  PIN_LED_R = -1;
constexpr int  PIN_LED_G = -1;
constexpr int  PIN_LED_B = -1;
constexpr int  PIN_LED = 15;          // on-board LED, active high
constexpr bool LED_ACTIVE_LOW = false;
constexpr bool         LIMIT_TX_POWER = false;
constexpr wifi_power_t TX_POWER = WIFI_POWER_19_5dBm;

#else
#error "This project is written for the ESP32-C3 and the ESP32-S2"
#endif
