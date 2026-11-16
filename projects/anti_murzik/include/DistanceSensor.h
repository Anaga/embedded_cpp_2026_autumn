#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include <Arduino.h>

class DistanceSensor {
public:
    enum class DistanceState : uint8_t {
        UNKNOWN = 0,
        MEASURING = 1,
        OBSTACLE_DETECTED = 2,
        CLEAR = 3
    };

    DistanceSensor(int trigPin, int echoPin);
    void init();
    float readDistanceCm();
    DistanceState getState() const { return state_; }
    float getDistanceCm() const { return distanceCm_; }

private:
    int trigPin_, echoPin_;
    DistanceState state_ = DistanceState::UNKNOWN;
    float distanceCm_ = 0.0;

    unsigned long lastMeasureMs_ = 0;
};

#endif // DISTANCE_SENSOR_H
