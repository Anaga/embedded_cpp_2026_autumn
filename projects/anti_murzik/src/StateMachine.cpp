#include "StateMachine.h"

void AntiMurzikStateMachine::setState(AntiMurzikState newState) {
    currentState_ = transition(currentState_, newState);
}

AntiMurzikStateMachine::AntiMurzikState
AntiMurzikStateMachine::transition(AntiMurzikState from, AntiMurzikState to) {
    // Safety: any state can go to SAFETY_STOP, and SAFETY_STOP can go to IDLE
    if (to == AntiMurzikState::SAFETY_STOP) return to;
    if (from == AntiMurzikState::SAFETY_STOP && to != AntiMurzikState::IDLE) return from;
    
    // From IDLE: can go to MANUAL_CONTROL, AUTONOMOUS_PATROL
    if (from == AntiMurzikState::IDLE) {
        if (to == AntiMurzikState::MANUAL_CONTROL ||
            to == AntiMurzikState::AUTONOMOUS_PATROL) return to;
    }
    
    // From MANUAL_CONTROL: can go to IDLE, AUTONOMOUS_PATROL, WATERING
    if (from == AntiMurzikState::MANUAL_CONTROL) {
        if (to == AntiMurzikState::IDLE ||
            to == AntiMurzikState::AUTONOMOUS_PATROL ||
            to == AntiMurzikState::WATERING) return to;
    }
    
    // From AUTONOMOUS_PATROL: can go to IDLE, MANUAL_CONTROL, WATERING, SAFETY_STOP
    if (from == AntiMurzikState::AUTONOMOUS_PATROL) {
        if (to == AntiMurzikState::IDLE ||
            to == AntiMurzikState::MANUAL_CONTROL ||
            to == AntiMurzikState::WATERING ||
            to == AntiMurzikState::SAFETY_STOP) return to;
    }
    
    // From WATERING: can go to IDLE, MANUAL_CONTROL, AUTONOMOUS_PATROL
    if (from == AntiMurzikState::WATERING) {
        if (to == AntiMurzikState::IDLE ||
            to == AntiMurzikState::MANUAL_CONTROL ||
            to == AntiMurzikState::AUTONOMOUS_PATROL) return to;
    }
    
    // Default: stay in current state
    return from;
}

const char* AntiMurzikStateMachine::getStateName() const {
    switch (currentState_) {
        case AntiMurzikState::IDLE:              return "IDLE";
        case AntiMurzikState::MANUAL_CONTROL:   return "MANUAL_CONTROL";
        case AntiMurzikState::AUTONOMOUS_PATROL: return "AUTONOMOUS_PATROL";
        case AntiMurzikState::SAFETY_STOP:      return "SAFETY_STOP";
        case AntiMurzikState::WATERING:         return "WATERING";
        default:                                 return "UNKNOWN";
    }
}
