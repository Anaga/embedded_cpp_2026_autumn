#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "board.h"

// DRV8833 dual H-bridge motor driver
// Motor A (drive): IN1 = forward, IN2 = backward
// Motor B (steer): IN3 = left, IN4 = right
class MotorController {
 public:
  enum class MotorState : uint8_t {
    STOP = 0,
    FORWARD = 1,
    BACKWARD = 2,
    TURN_LEFT = 3,
    TURN_RIGHT = 4
  };

  // Gears: 0=stop, 1=slow, 2=medium, 3=fast
  static constexpr uint8_t GEAR_PWM[4] = {0, 130, 190, 255};

  MotorController();
  void begin();
  void update(uint32_t now);

  bool setGear(int gear);
  int getGear() const;

  void forward();
  void backward();
  void turnLeft();
  void turnRight();
  void stop();

  MotorState getState() const;
  uint8_t getSpeed() const;
  bool isReady() const;

 private:
  int currentGear_;
  MotorState state_;
  bool initialized_;
};

#endif // MOTOR_CONTROLLER_H