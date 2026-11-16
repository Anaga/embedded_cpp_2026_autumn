#ifndef WATER_PUMP_H
#define WATER_PUMP_H

#include <Arduino.h>

class WaterPump {
public:
    enum class PumpState : uint8 {
        OFF = 0,
        SPRAYING = 1,
        STANDBY = 2
    };

    WaterPump(int pumpPin);
    void init();
    void activate(); // Start spraying
    void deactivate(); // Stop spraying
    void setMode(PumpState mode);
    PumpState getState() const { return state_; }
    
    // Is pump currently spraying?
    bool isSpraying() const { return state_ == PumpState::SPRAYING; }

private:
    int pumpPin_;
    PumpState state_ = PumpState::OFF;
};

#endif // WATER_PUMP_H
