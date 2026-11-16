#include "DistanceSensor.h"

DistanceSensor::DistanceSensor(int trigPin, int echoPin)
    : trigPin_(trigPin), echoPin_(echoPin) {}

void DistanceSensor::init() {
    pinMode(trigPin_, OUTPUT);
    pinMode(echoPin_, INPUT);
    digitalWrite(trigPin_, LOW);
    state_ = DistanceState::UNKNOWN;
}

float DistanceSensor::readDistanceCm() {
    // Ensure a minimum interval between readings to avoid ultrasonic interference
    unsigned long now = millis();
    if (now - lastMeasureMs_ < 60) {
        return distanceCm_;
    }
    lastMeasureMs_ = now;
    
    // Trigger pulse 10us
    digitalWrite(trigPin_, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_, LOW);
    
    // Read echo duration (timeout 30ms = ~5m max)
    long duration = pulseIn(echoPin_, HIGH, 30000);
    
    if (duration == 0) {
        // No echo -> out of range
        state_ = DistanceState::UNKNOWN;
        distanceCm_ = -1.0;
        return distanceCm_;
    }
    
    // Speed of sound: 343 m/s -> 0.0343 cm/us
    distanceCm_ = duration * 0.0343 / 2.0;
    
    if (distanceCm_ <= 30.0) {
        state_ = DistanceState::OBSTACLE_DETECTED;
    } else {
        state_ = DistanceState::CLEAR;
    }
    return distanceCm_;
}
