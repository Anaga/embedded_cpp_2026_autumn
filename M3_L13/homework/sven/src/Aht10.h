#pragma once

#include <stdint.h>

class Aht10 {
public:
    enum class Status { Idle, Busy, Ready, Missing };

    void begin(); // Set up Wire; the first start() also initializes the sensor.

    void start(); // Ignored while a previous operation is pending.
    Status poll(); // Ready and Missing are returned once per operation.
    float temperature() const { return temperature_; }
    float humidity() const { return humidity_; }

private:
    enum class State { Idle, PoweringUp, Resetting, Calibrating, Measuring, Missing };

    void waitFor(State state);
    Status missing();

    State state_ = State::Idle;
    bool initialized_ = false;
    uint32_t startedMs_ = 0;
    uint32_t lastPollMs_ = 0;
    float temperature_ = 0;
    float humidity_ = 0;
};
