#include "Aht10.h"

#include <Arduino.h>
#include <Wire.h>

// Follow Adafruit_AHTX0::begin() and getEvent() at the sensor level:
// https://github.com/adafruit/Adafruit_AHTX0/blob/master/Adafruit_AHTX0.cpp
// Their delays and busy loops become states advanced by poll().
namespace {
    constexpr uint8_t ADDRESS = 0x38;
    constexpr uint8_t CMD_SOFTRESET = 0xBA;
    constexpr uint8_t CMD_CALIBRATE = 0xE1;
    constexpr uint8_t CMD_TRIGGER = 0xAC;
    constexpr uint8_t BUSY_BIT = 0x80;
    constexpr uint8_t CALIBRATED_BIT = 0x08;
    constexpr uint32_t POWER_RESET_WAIT_MS = 20;
    constexpr uint32_t POLL_INTERVAL_MS = 10;
    constexpr uint32_t TIMEOUT_MS = 200;

    bool sendCommand(uint8_t command, uint8_t parameter = 0) {
        Wire.beginTransmission(ADDRESS);
        Wire.write(command);
        if (command != CMD_SOFTRESET) {
            Wire.write(parameter);
            Wire.write(0x00);
        }
        return Wire.endTransmission() == 0;
    }
} // namespace

void Aht10::begin() {
    Wire.setSDA(4);
    Wire.setSCL(5);
    Wire.begin();
    Wire.setTimeout(5, true); // Bound each transfer; reset I2C on timeout.
    state_ = State::Idle;
    initialized_ = false;
}

void Aht10::start() {
    if (state_ != State::Idle) {
        return;
    }

    if (!initialized_) {
        waitFor(State::PoweringUp);
        return;
    }

    // getEvent(): trigger a new measurement, then check busy in poll().
    waitFor(State::Measuring);
    if (!sendCommand(CMD_TRIGGER, 0x33)) {
        state_ = State::Missing;
    }
}

Aht10::Status Aht10::poll() {
    if (state_ == State::Missing) {
        return missing();
    }
    if (state_ == State::Idle) {
        return Status::Idle;
    }

    const uint32_t now = millis();
    const uint32_t elapsed = now - startedMs_;
    const uint32_t waitMs = (state_ == State::PoweringUp || state_ == State::Resetting)
                               ? POWER_RESET_WAIT_MS : POLL_INTERVAL_MS;
    if (elapsed < waitMs || now - lastPollMs_ < POLL_INTERVAL_MS) {
        return Status::Busy;
    }
    lastPollMs_ = now;

    if (state_ == State::PoweringUp) {
        // begin(): wait 20 ms, check the address, soft reset, wait 20 ms.
        Wire.beginTransmission(ADDRESS);
        if (Wire.endTransmission() != 0 || !sendCommand(CMD_SOFTRESET)) {
            return missing();
        }
        waitFor(State::Resetting);
        return Status::Busy;
    }

    // getStatus(): read only the status byte while waiting for busy to clear.
    if (Wire.requestFrom(ADDRESS, size_t(1)) != 1) {
        return missing();
    }
    const uint8_t status = Wire.read();
    if (status & BUSY_BIT) {
        return elapsed >= TIMEOUT_MS ? missing() : Status::Busy;
    }

    if (state_ == State::Resetting) {
        if (!sendCommand(CMD_CALIBRATE, 0x08)) {
            return missing();
        }
        waitFor(State::Calibrating);
        return Status::Busy;
    }
    if (!(status & CALIBRATED_BIT)) {
        return missing();
    }

    if (state_ == State::Calibrating) {
        initialized_ = true;
        state_ = State::Idle;
        start(); // Initialization finished; now trigger the measurement.
        return Status::Busy;
    }

    // getEvent(): busy cleared, so read the six-byte result and convert it.
    uint8_t data[6];
    if (Wire.requestFrom(ADDRESS, sizeof(data)) != sizeof(data)) {
        return missing();
    }
    for (uint8_t &byte : data) {
        byte = Wire.read();
    }
    if (data[0] & BUSY_BIT) {
        return elapsed >= TIMEOUT_MS ? missing() : Status::Busy;
    }
    if (!(data[0] & CALIBRATED_BIT)) {
        return missing();
    }

    const uint32_t rawHumidity = (uint32_t(data[1]) << 12)
                                 | (uint32_t(data[2]) << 4) | (data[3] >> 4);
    const uint32_t rawTemperature = (uint32_t(data[3] & 0x0F) << 16)
                                    | (uint32_t(data[4]) << 8) | data[5];
    humidity_ = rawHumidity * 100.0f / 0x100000;
    temperature_ = rawTemperature * 200.0f / 0x100000 - 50.0f;
    state_ = State::Idle;
    return Status::Ready;
}

void Aht10::waitFor(State state) {
    state_ = state;
    startedMs_ = lastPollMs_ = millis();
}

Aht10::Status Aht10::missing() {
    initialized_ = false;
    state_ = State::Idle;
    return Status::Missing;
}
