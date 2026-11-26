#ifndef WATER_PUMP_H
#define WATER_PUMP_H

#include <Arduino.h>
#include "board.h"

class WaterPump {
 public:
  enum class PumpState : uint8_t {
    OFF = 0,
    SPRAYING = 1,
    STANDBY = 2
  };

  WaterPump();
  void begin();
  void update(uint32_t now);

  void activate();
  void deactivate();

  PumpState getState() const;
  bool isSpraying() const;
  bool isReady() const;

 private:
  PumpState state_;
  uint32_t sprayStart_;
  bool initialized_;
};

#endif // WATER_PUMP_H