#include "MotorController.h"

MotorController::MotorController(int enL, int inL1, int inL2, int enR, int inR1, int inR2)
    : enL_(enL), inL1_(inL1), inL2_(inL2), enR_(enR), inR1_(inR1), inR2_(inR2) {}

void MotorController::init() {
    pinMode(enL_, OUTPUT);
    pinMode(inL1_, OUTPUT);
    pinMode(inL2_, OUTPUT);
    pinMode(enR_, OUTPUT);
    pinMode(inR1_, OUTPUT);
    pinMode(inR2_, OUTPUT);
    
    // Set initial state: stopped, low outputs
    digitalWrite(inL1_, LOW);
    digitalWrite(inL2_, LOW);
    digitalWrite(inR1_, LOW);
    digitalWrite(inR2_, LOW);
    digitalWrite(enL_, LOW);
    digitalWrite(enR_, LOW);
    
    initialized_ = true;
    state_ = MotorState::STOP;
}

void MotorController::setMotor(int motor, int speed) {
    if (!initialized_) return;
    
    // motor: 0 = left, 1 = right
    int in1, in2, en;
    if (motor == 0) {
        in1 = inL1_; in2 = inL2_; en = enL_;
    } else {
        in1 = inR1_; in2 = inR2_; en = enR_;
    }
    
    // Clamp speed
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;
    
    if (speed > 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(en, speed);
    } else if (speed < 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(en, -speed);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(en, 0);
    }
}

void MotorController::forward(uint8_t speed) {
    state_ = MotorState::FORWARD;
    speed_ = speed;
    digitalWrite(inL1_, HIGH);
    digitalWrite(inL2_, LOW);
    digitalWrite(inR1_, HIGH);
    digitalWrite(inR2_, LOW);
    analogWrite(enL_, speed);
    analogWrite(enR_, speed);
}

void MotorController::backward(uint8_t speed) {
    state_ = MotorState::BACKWARD;
    speed_ = speed;
    digitalWrite(inL1_, LOW);
    digitalWrite(inL2_, HIGH);
    digitalWrite(inR1_, LOW);
    digitalWrite(inR2_, HIGH);
    analogWrite(enL_, speed);
    analogWrite(enR_, speed);
}

void MotorController::turnLeft(uint8_t speed) {
    state_ = MotorState::TURN_LEFT;
    speed_ = speed;
    // Reverse left, forward right
    digitalWrite(inL1_, LOW);
    digitalWrite(inL2_, HIGH);
    digitalWrite(inR1_, HIGH);
    digitalWrite(inR2_, LOW);
    analogWrite(enL_, speed);
    analogWrite(enR_, speed);
}

void MotorController::turnRight(uint8_t speed) {
    state_ = MotorState::TURN_RIGHT;
    speed_ = speed;
    // Forward left, reverse right
    digitalWrite(inL1_, HIGH);
    digitalWrite(inL2_, LOW);
    digitalWrite(inR1_, LOW);
    digitalWrite(inR2_, HIGH);
    analogWrite(enL_, speed);
    analogWrite(enR_, speed);
}

void MotorController::stop() {
    state_ = MotorState::STOP;
    speed_ = 0;
    digitalWrite(inL1_, LOW);
    digitalWrite(inL2_, LOW);
    digitalWrite(inR1_, LOW);
    digitalWrite(inR2_, LOW);
    analogWrite(enL_, 0);
    analogWrite(enR_, 0);
}

void MotorController::setAll(int speed, int direction) {
    // direction: 1 = forward, -1 = backward
    if (direction > 0) forward(speed);
    else if (direction < 0) backward(speed);
    else stop();
}
