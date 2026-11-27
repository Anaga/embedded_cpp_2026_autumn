#include "WaterPump.h"

WaterPump::WaterPump()
    : state_(PumpState::OFF), sprayStart_(0), initialized_(false) {}

void WaterPump::begin() {
    pinMode(PIN_PUMP, OUTPUT);
    deactivate();
    initialized_ = true;
}

void WaterPump::update(uint32_t now) {
    if (state_ == PumpState::SPRAYING && (now - sprayStart_ >= PUMP_SPRAY_DURATION_MS)) {
        deactivate();
    }
}

void WaterPump::activate() {
    state_ = PumpState::SPRAYING;
    sprayStart_ = millis();
    digitalWrite(PIN_PUMP, HIGH);
}

void WaterPump::deactivate() {
    state_ = PumpState::OFF;
    digitalWrite(PIN_PUMP, LOW);
}

WaterPump::PumpState WaterPump::getState() const { return state_; }
bool WaterPump::isSpraying() const { return state_ == PumpState::SPRAYING; }
bool WaterPump::isReady() const { return initialized_; }