#include "DistanceSensor.h"

DistanceSensor::DistanceSensor(int trigPin, int echoPin)
    : trigPin_(trigPin), echoPin_(echoPin),
      state_(DistanceState::UNKNOWN), distanceCm_(0.0f) {}

void DistanceSensor::begin() {
    pinMode(trigPin_, OUTPUT);
    pinMode(echoPin_, INPUT);
    digitalWrite(trigPin_, LOW);
    lastMeasure_ = 0;
}

void DistanceSensor::update(uint32_t now) {
    if (now - lastMeasure_ < 100U) return;  // Read every 100ms
    lastMeasure_ = now;
    readDistanceCm();
}

float DistanceSensor::readDistanceCm() {
    digitalWrite(trigPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_, LOW);

    uint32_t duration = pulseIn(echoPin_, HIGH, 30000);  // 30ms timeout
    if (duration == 0) {
        distanceCm_ = -1.0f;
        state_ = DistanceState::UNKNOWN;
        return distanceCm_;
    }

    distanceCm_ = duration * 0.0343f / 2.0f;  // cm

    if (distanceCm_ > 0 && distanceCm_ < 30.0f) {
        state_ = DistanceState::OBSTACLE_DETECTED;
    } else {
        state_ = DistanceState::CLEAR;
    }
    return distanceCm_;
}

DistanceSensor::DistanceState DistanceSensor::getState() const { return state_; }
float DistanceSensor::getDistanceCm() const { return distanceCm_; }