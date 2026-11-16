#include <Arduino.h>
#include <stdint.h>

// ======== Lesson 21 - Final homework: non-blocking FSM for autonomous devices ======
// Brings together:
//   - non-blocking timing (millis)
//   - state machine with named transitions
//   - sensor -> actuator automation with fail-safe
//
// States: OFF -> IDLE -> MANUAL -> AUTONOMOUS -> SAFE

enum class DeviceState : uint8_t { OFF = 0, IDLE = 1, MANUAL = 2, AUTONOMOUS = 3, SAFE = 4 };

class StateMachine {
public:
    DeviceState state_ = DeviceState::OFF;

    void init() { state_ = DeviceState::IDLE; }

    DeviceState current() const { return state_; }

    void transition(DeviceState next) {
        // Safety rule: never leave SAFE until explicitly reset to IDLE
        if (state_ == DeviceState::SAFE) {
            if (next == DeviceState::IDLE) state_ = next;
            return;
        }
        switch (state_) {
            case DeviceState::OFF:
                if (next == DeviceState::IDLE || next == DeviceState::MANUAL) state_ = next;
                break;
            case DeviceState::IDLE:
                if (next == DeviceState::MANUAL || next == DeviceState::AUTONOMOUS) state_ = next;
                break;
            case DeviceState::MANUAL:
                if (next == DeviceState::IDLE || next == DeviceState::AUTONOMOUS) state_ = next;
                break;
            case DeviceState::AUTONOMOUS:
                if (next == DeviceState::IDLE || next == DeviceState::MANUAL || next == DeviceState::SAFE) state_ = next;
                break;
            case DeviceState::SAFE:
                break;
            default:
                break;
        }
    }
};

static StateMachine fsm;
static const uint32_t TICK_MS = 100;
static const uint32_t HEARTBEAT_MS = 1000;
static const uint32_t READ_MS = 200;
static const uint32_t SAFE_DEBOUNCE_MS = 1000;
static bool sensor_ok = true;

void setup() {
    Serial.begin(115200);
    fsm.init();
}

void loop() {
    static uint32_t last_tick = 0;
    static uint32_t last_read = 0;
    static uint32_t last_safe_check = 0;
    static uint32_t hb_last = 0;

    const uint32_t now = millis();

    // Non-blocking tick
    if (now - last_tick >= TICK_MS) {
        last_tick = now;
        if (fsm.current() == DeviceState::AUTONOMOUS) {
            // Autonomous routine runs here (sensor -> actuator)
        }
    }

    // Periodic sensor read
    if (now - last_read >= READ_MS) {
        last_read = now;
    }

    // Fail-safe check
    if (now - last_safe_check >= SAFE_DEBOUNCE_MS) {
        last_safe_check = now;
        if (!sensor_ok) {
            fsm.transition(DeviceState::SAFE);
        }
    }

    // Heartbeat: print state once per second
    if (now - hb_last >= HEARTBEAT_MS) {
        hb_last = now;
        Serial.printf("STATE %u\n", (unsigned)fsm.current());
    }
}
