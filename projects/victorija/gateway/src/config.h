// Every number the gateway is tuned with, in one place.
// DEMO values are marked: they are short so the audience sees things happen.
// The real values (for the bike in winter) are in the comment next to them.
#pragma once
#include <stdint.h>

// ---- Pins ---------------------------------------------------------------------
constexpr uint8_t PIN_RGB = 38U;  // the S3 DevKit's on-board WS2812 (LoRa pins: ../common/radio.h)

// ---- The node -----------------------------------------------------------------
constexpr uint32_t NODE_INTERVAL_S = 20U;  // DEMO: the node reports every 20 s (real: 600)
constexpr uint32_t NODE_LOST_MS = 3U * NODE_INTERVAL_S * 1000U;  // 3 missed reports in a row = "no contact"

// ---- Alarm limits -------------------------------------------------------------
constexpr float BATT_LOW_V = 12.2f;        // a lead-acid battery at ~12.2 V is about half empty
constexpr float BATT_CRIT_V = 11.8f;       // ~11.8 V: soon too flat to start the bike
constexpr float BATT_HYST_V = 0.1f;        // clears only 0.1 V ABOVE the limit: no flicker at 12.19 / 12.21
constexpr float BATT_SENSE_MIN_V = 3.0f;   // below: the divider isn't connected (no 12 V battery reads that)
constexpr float BATT_SENSE_MAX_V = 16.0f;  // above: a wiring fault too (a 12 V battery charges at 14.8 V at most)
constexpr float BATT_CHARGING_V = 13.2f;   // above this the battery is on a charger, not resting
constexpr float DAMP_ON_RH = 85.0f;        // condensation risk above this...
constexpr float DAMP_OFF_RH = 80.0f;       // ...and it clears only below this (hysteresis)
constexpr float T_MIN_C = -40.0f;          // outside -40...+60 C it isn't a reading, it's a fault: an AHT10
constexpr float T_MAX_C = 60.0f;           // without VIN reported 67-79 C. Under a bike seat in winter: never > 60
constexpr float T_JUMP_MAX_C = 10.0f;      // a jump bigger than this between two packets is not real
// DEMO: the movement alarm clears after 10 s of quiet (real: 120 s).
// Keep it longer than the node's TAMPER_COOLDOWN_S.
constexpr uint32_t TAMPER_QUIET_MS = 10000U;
constexpr uint32_t TIME_ALARM_CHECK_MS = 1000U;  // how often the time-based alarms are checked

// ---- LED ----------------------------------------------------------------------
constexpr uint8_t LED_FLASH_LEVEL = 20U;  // brightness of the short flash per packet (0-255)
constexpr uint32_t LED_FLASH_MS = 150U;   // how long that flash lasts
constexpr uint32_t LED_BLINK_MS = 300U;   // blinking colours: 0.3 s on, 0.3 s the other colour
constexpr uint32_t LED_REFRESH_MS = 1000U;  // rewrite a steady colour this often, in case a write got lost

// ---- Telegram and WiFi ------------------------------------------------------
constexpr uint32_t BOT_POLL_MS = 2000U;       // ask Telegram for new messages every 2 s
constexpr uint32_t NET_RETRY_MS = 5000U;      // while offline: try again every 5 s
constexpr uint32_t WIFI_CONNECT_MS = 15000U;  // one WiFiMulti scan + join may take this long
constexpr uint32_t NTP_WAIT_MS = 10000U;      // wait this long for the first NTP answer
constexpr uint32_t MUTE_MS = 3600000U;        // the Mute button: 1 hour without movement alerts
constexpr uint32_t NET_STACK_BYTES = 12288U;  // the net task's stack: TLS (HTTPS) needs a lot
constexpr uint8_t ALERT_QUEUE_LEN = 16U;      // alerts held while offline (16 x 320 B = 5 KB)
constexpr const char* TIME_ZONE = "EET-2EEST,M3.5.0/3,M10.5.0/4";  // Estonian time, with summer time

// ---- History ------------------------------------------------------------------
// 2000 entries x 12 bytes = 24 KB of the S3's 320 KB RAM. At the demo
// interval (20 s) that's about 11 hours; at the real 10 min, about 2 weeks.
constexpr uint16_t HIST_SIZE = 2000U;
constexpr uint8_t HIST_DEFAULT_H = 3U;  // /history with no number: the last 3 hours
constexpr uint8_t HIST_MAX_H = 48U;
constexpr uint8_t HIST_ROWS = 12U;      // aim for about 12 rows in the table
constexpr uint8_t HIST_MAX_ROWS = 25U;

// Any clock time before this (Nov 2023) means NTP hasn't set the clock yet.
constexpr uint32_t CLOCK_VALID_AFTER = 1700000000UL;
