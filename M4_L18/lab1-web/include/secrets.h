// Copy this file to secrets.h (same folder) and fill it in.
// secrets.h is listed in .gitignore and must never be committed.
#pragma once
#include <cstdint>

// true  -> the board is its own access point (no router needed):
//          join the WiFi network "esp-xxxx" with AP_PASS,
//          then open http://192.168.4.1/
// false -> the board joins WIFI_SSID as a station
constexpr bool USE_SOFTAP = false;

constexpr const char *WIFI_SSID = "HUAWEICF10";
constexpr const char *WIFI_PASS = "41256011";
constexpr const char *AP_PASS   = "embedded2026";   // at least 8 characters

// MQTT_HOST empty -> MQTT is off.
// Public brokers: anyone can read and write your topics.
constexpr const char *MQTT_HOST   = "broker.hivemq.com";
constexpr uint16_t    MQTT_PORT   = 1883;
constexpr const char *MQTT_USER   = "";              // empty -> anonymous
constexpr const char *MQTT_PASS   = "";
constexpr const char *MQTT_PREFIX = "gamma-ed/2026"; // topics: <prefix>/<id>/...
