#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

// ==================== WiFi Configuration ====================
// AP fallback credentials (used when no saved networks found)
const char* WIFI_AP_SSID = "AntiMurzik_AP";
const char* WIFI_AP_PASSWORD = "anti1234";
const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

// Web server port
const uint16_t WEB_SERVER_PORT = 80;

// WiFi TX power (15 dBm is optimal for ESP32-C3 Super Mini)
constexpr wifi_power_t WIFI_TX_POWER = WIFI_POWER_15dBm;
constexpr bool LIMIT_TX_POWER = true;

// ==================== Motor Pins (DRV8833 on ESP32-C3 Super Mini) ====================
// SAFE pins (no strapping pins: GPIO 2, 8, 9 are strapping)
constexpr int PIN_DRIVE_FWD = 10;  // IN1 (DRV8833: forward)
constexpr int PIN_DRIVE_REV = 9;   // IN2 (DRV8833: backward)
constexpr int PIN_STEER_LEFT = 3;  // IN3 (DRV8833: steer left)
constexpr int PIN_STEER_RIGHT = 4; // IN4 (DRV8833: steer right)

constexpr int PIN_HEADLIGHTS = 6;  // Headlights (reserved)
constexpr int PIN_PUMP = 5;        // Water pump (reserved)

constexpr int PWM_FREQ = 1000;
constexpr int PWM_RES = 8;

// ==================== Water Pump ====================
constexpr uint32_t PUMP_SPRAY_DURATION_MS = 600;
constexpr uint32_t PUMP_COOLDOWN_MS = 3000;

// ==================== BLE (Nordic UART Service) ====================
#define BLE_SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_UUID_RX        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_UUID_TX        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// ==================== Timing ====================
constexpr uint32_t HEARTBEAT_INTERVAL_MS = 1000;
constexpr uint32_t SNAPSHOT_PERIOD_MS = 100;
constexpr uint32_t LOOP_STATS_PERIOD_MS = 5000;

// ==================== Fixed-width types ====================
using uint8 = uint8_t;
using int16 = int16_t;
using uint32 = uint32_t;
using float32 = float;

// ==================== MDNS ====================
constexpr char MDNS_HOST[] = "antimurzik";

#endif // CONFIG_H