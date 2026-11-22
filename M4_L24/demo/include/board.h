// Pins and radio settings for the ESP32-C3 Super Mini.
// Same wiring as sessions 18-23 plus button A from K3: nothing to re-plug.
#pragma once
#include <Arduino.h>
#include <WiFi.h>

#if !defined(CONFIG_IDF_TARGET_ESP32C3)
#error "This project is written for the ESP32-C3 Super Mini"
#endif

// I2C SDA GPIO 8, SCL GPIO 9 (the Wire defaults of lolin_c3_mini).
// GPIO 8 and GPIO 9 are strapping pins. The AHT10 module pull-ups keep both
// HIGH during reset, which is the normal boot from flash.
//   - GPIO 9 is also the BOOT button: pressing it spoils a reading.
//   - Anything holding SDA or SCL LOW during reset can stop the boot.
//   - The on-board blue LED on GPIO 8 flickers with every I2C transfer.
constexpr int PIN_SDA = 8;
constexpr int PIN_SCL = 9;


// RGB LED, common anode to 3V3, LOW = on (as in K3).
//   green = the "LED" you switch with button A or from Telegram
//   blue  = heartbeat: blinks 4 times a second while loop() is alive
constexpr int PIN_LED_R = 5;
constexpr int PIN_LED_G = 6;
constexpr int PIN_LED_B = 7;


// Button A, INPUT_PULLUP, button to GND, active low (as in K3).
constexpr int PIN_BUTTON_A = 0;

// Many Super Mini boards connect only with reduced TX power (session 18).
constexpr bool         LIMIT_TX_POWER = true;
constexpr wifi_power_t TX_POWER       = WIFI_POWER_8_5dBm;
