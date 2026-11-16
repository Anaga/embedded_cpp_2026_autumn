#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi Configuration ====================
const char* WIFI_AP_SSID = "AntiMurzik_AP";
const char* WIFI_AP_PASSWORD = "anti1234";
const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

// Web server port
const uint16_t WEB_SERVER_PORT = 80;

// ==================== Motor Pins (L298N / MX1508) ====================
// Left motor
const int MOTOR_L_EN = 13;   // PWM enable left
const int MOTOR_L_IN1 = 14;  // Direction left 1
const int MOTOR_L_IN2 = 27;  // Direction left 2

// Right motor
const int MOTOR_R_EN = 12;   // PWM enable right
const int MOTOR_R_IN1 = 26;  // Direction right 1
const int MOTOR_R_IN2 = 25;  // Direction right 2

// PWM settings
const int PWM_FREQ = 1000;   // Hz
const int PWM_CHANNEL_L = 0; // LEDC channel 0
const int PWM_CHANNEL_R = 1; // LEDC channel 1
const int PWM_RESOLUTION = 8; // 8-bit (0-255)

// ==================== Water Pump ====================
const int PUMP_PIN = 33;     // MOSFET gate / relay control
const uint32_t PUMP_SPRAY_DURATION_MS = 800; // Spray duration when activated
const uint32_t PUMP_COOLDOWN_MS = 3000;      // Minimum time between sprays

// ==================== Distance Sensor (HC-SR04) ====================
const int TRIG_PIN = 32;
const int ECHO_PIN = 35;
const uint32_t DISTANCE_READ_INTERVAL_MS = 100; // Read every 100ms
const float DISTANCE_OBSTACLE_CM = 30.0;        // Obstacle threshold

// ==================== PIR Motion Sensor ====================
const int PIR_PIN = 34;
const uint32_t PIR_COOLDOWN_MS = 5000; // Minimum time between motion-triggered sprays

// ==================== Timing ====================
const uint32_t HEARTBEAT_INTERVAL_MS = 1000;   // Status heartbeat
const uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000; // WiFi reconnect attempt

// ==================== Fixed-width types ====================
using uint8 = uint8_t;
using int16 = int16_t;
using uint32 = uint32_t;
using float32 = float;

#endif // CONFIG_H
