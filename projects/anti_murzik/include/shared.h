#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <cmath>

// ==================== Commands from net task to loop task ====================
namespace shared {

enum class LedCmd : uint8_t {
    NONE = 0,
    ON,
    OFF,
    TOGGLE
};

enum class MotorCmd : uint8_t {
    NONE = 0,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    STOP,
    GEAR_1,
    GEAR_2,
    GEAR_3
};

enum class PumpCmd : uint8_t {
    NONE = 0,
    FIRE
};

struct Command {
    enum class Type : uint8_t { NONE, LED, MOTOR, PUMP } type = Type::NONE;
    union {
        LedCmd led;
        MotorCmd motor;
        PumpCmd pump;
    } cmd;
    uint8_t param = 0;  // For motor: speed; for led: unused
};

// ==================== Snapshot from loop task to net task ====================
struct Snapshot {
    bool sensorValid = false;
    float tempC = NAN;
    float rhPct = NAN;
    float chipC = NAN;
    bool ledOn = false;
    int motorState = 0;  // 0=stop, 1=fwd, 2=rev, 3=left, 4=right
    uint8_t gear = 2;
    uint32_t longestPassMs = 0;
    uint32_t uptimeSec = 0;
    bool wifiConnected = false;
    int wifiRssi = 0;
    char wifiSsid[33] = "";
    char deviceId[16] = "esp-????";
};

// ==================== Public API ====================
// Initialize shared structures (call from setup(), before net::begin())
void begin();

// Get device ID string (e.g., "esp-a1b2")
const char* deviceId();

// ---- Commands: net task -> loop task ----
bool sendLedCmd(LedCmd cmd);
bool sendMotorCmd(MotorCmd cmd, uint8_t speed = 150);
bool sendPumpCmd(PumpCmd cmd);

// Loop task pulls commands
bool receiveCommand(Command& out);

// ---- Snapshot: loop task -> net task ----
void publish(const Snapshot& s);

// Net task reads snapshot
Snapshot read();

}  // namespace shared

#endif // SHARED_H