#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

class AntiMurzikStateMachine {
public:
    enum class AntiMurzikState : uint8_t {
        IDLE = 0,
        MANUAL_CONTROL = 1,
        AUTONOMOUS_PATROL = 2,
        SAFETY_STOP = 3,
        WATERING = 4
    };

    AntiMurzikState getState() const { return currentState_; }
    void setState(AntiMurzikState newState);

    // Transition logic
    AntiMurzikState transition(AntiMurzikState from, AntiMurzikState to);

    // Human-readable state name
    const char* getStateName() const;

private:
    AntiMurzikState currentState_ = AntiMurzikState::IDLE;
};

#endif // STATE_MACHINE_H
