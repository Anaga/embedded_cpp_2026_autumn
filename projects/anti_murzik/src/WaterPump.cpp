#include "WaterPump.h"

WaterPump::WaterPump(int pumpPin) : pumpPin_(pumpPin) {}

void WaterPump::init() {
    pinMode(pumpPin_, OUTPUT);
    digitalWrite(pumpPin_, LOW);
    state_ = PumpState::OFF;
}

void WaterPump::activate() {
    digitalWrite(pumpPin_, HIGH);
    state_ = PumpState::SPRAYING;
}

void WaterPump::deactivate() {
    digitalWrite(pumpPin_, LOW);
    state_ = PumpState::OFF;
}

void WaterPump::setMode(PumpState mode) {
    if (mode == PumpState::SPRAYING) {
        activate();
    } else {
        deactivate();
    }
}
