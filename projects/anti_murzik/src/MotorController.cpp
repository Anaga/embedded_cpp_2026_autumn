#include "MotorController.h"

MotorController::MotorController()
    : currentGear_(2), state_(MotorState::STOP), initialized_(false) {}

void MotorController::begin() {
    // Setup PWM channels for DRV8833
    // Channel 0: Drive Forward
    ledcSetup(0, PWM_FREQ, PWM_RES);
    ledcAttachPin(PIN_DRIVE_FWD, 0);
    
    // Channel 1: Drive Reverse
    ledcSetup(1, PWM_FREQ, PWM_RES);
    ledcAttachPin(PIN_DRIVE_REV, 1);
    
    // Channel 2: Steer Left
    ledcSetup(2, PWM_FREQ, PWM_RES);
    ledcAttachPin(PIN_STEER_LEFT, 2);
    
    // Channel 3: Steer Right
    ledcSetup(3, PWM_FREQ, PWM_RES);
    ledcAttachPin(PIN_STEER_RIGHT, 3);
    
    stop();
    initialized_ = true;
}

void MotorController::update(uint32_t now) {
    (void)now;
    // Motor commands are handled via shared::receiveCommand()
    // This component is passive - commands come from net task
}

bool MotorController::setGear(int gear) {
    if (gear < 1 || gear > 3) return false;
    currentGear_ = gear;
    return true;
}

int MotorController::getGear() const { return currentGear_; }

void MotorController::forward() {
    uint8_t pwm = GEAR_PWM[currentGear_];
    ledcWrite(1, 0);
    ledcWrite(0, pwm);
    state_ = MotorState::FORWARD;
}

void MotorController::backward() {
    uint8_t pwm = GEAR_PWM[currentGear_];
    ledcWrite(0, 0);
    ledcWrite(1, pwm);
    state_ = MotorState::BACKWARD;
}

void MotorController::turnLeft() {
    uint8_t pwm = GEAR_PWM[currentGear_];
    ledcWrite(3, 0);
    ledcWrite(2, pwm);
    state_ = MotorState::TURN_LEFT;
}

void MotorController::turnRight() {
    uint8_t pwm = GEAR_PWM[currentGear_];
    ledcWrite(2, 0);
    ledcWrite(3, pwm);
    state_ = MotorState::TURN_RIGHT;
}

void MotorController::stop() {
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWrite(3, 0);
    state_ = MotorState::STOP;
}

MotorController::MotorState MotorController::getState() const { return state_; }
uint8_t MotorController::getSpeed() const { return GEAR_PWM[currentGear_]; }
bool MotorController::isReady() const { return initialized_; }