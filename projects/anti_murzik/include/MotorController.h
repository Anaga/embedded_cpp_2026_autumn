#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

class MotorController {
public:
    enum class MotorState : uint8 {
        STOP = 0,
        FORWARD = 1,
        BACKWARD = 2,
        TURN_LEFT = 3,
        TURN_RIGHT = 4
    };

    MotorController(int enL, int inL1, int inL2, int enR, int inR1, int inR2);
    void init();
    void setMotor(int motor, int speed); // speed: -255..255
    void forward(uint8_t speed = 150);   // Forward at given PWM duty
    void backward(uint8_t speed = 150);
    void turnLeft(uint8_t speed = 150);
    void turnRight(uint8_t speed = 150);
    void stop();
    void setAll(int speed, int direction);
    
    // Non-blocking state tracking
    MotorState getMotorState() const { return state_; }
    uint8_t getSpeed() const { return speed_; }

private:
    int enL_, inL1_, inL2_;
    int enR_, inR1_, inR2_;
    MotorState state_ = MotorState::STOP;
    uint8_t speed_ = 150;
    bool initialized_ = false;
};

#endif // MOTOR_CONTROLLER_H
