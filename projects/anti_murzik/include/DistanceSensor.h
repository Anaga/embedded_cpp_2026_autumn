#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include <Arduino.h>

// HC-SR04 ultrasonic distance sensor - non-blocking
class DistanceSensor {
 public:
  enum class DistanceState : uint8_t {
    UNKNOWN = 0,
    MEASURING = 1,
    OBSTACLE_DETECTED = 2,
    CLEAR = 3
  };

  DistanceSensor(int trigPin, int echoPin);
  void begin();
  void update(uint32_t now);
  float readDistanceCm();

  DistanceState getState() const;
  float getDistanceCm() const;

 private:
  int trigPin_, echoPin_;
  DistanceState state_;
  float distanceCm_;
  uint32_t lastMeasure_ = 0;
};

#endif // DISTANCE_SENSOR_H