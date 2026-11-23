#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#include <cmath>
#include <cstdint>

namespace sounder {

// HC-SR04-compatible Trig/Echo driver for ESP32. Call the public API from one
// task; the interrupt only captures edges. Instances must stay at one address.
// The caller schedules trigger recovery time: this project's HC-SR04 live mode
// uses 70 ms per sensor; diagnostics use a conservative 200 ms.
class UltrasonicSensor {
public:
    static constexpr uint32_t kTimeoutUs = 30000;
    static constexpr float kMinCm = 2.0f;
    static constexpr float kMaxCm = 450.0f;

    enum class Status { Idle, Valid, Timeout, OutOfRange, EchoHigh };

    struct Reading {
        Status status = Status::Idle;
        uint32_t pulseUs = 0;
        float distanceCm = 0.0f;
        uint32_t completedAtMs = 0;
    };

    UltrasonicSensor(uint8_t triggerPin, uint8_t echoPin)
        : triggerPin_(triggerPin), echoPin_(echoPin) {}
    UltrasonicSensor(const UltrasonicSensor&) = delete;
    UltrasonicSensor& operator=(const UltrasonicSensor&) = delete;
    ~UltrasonicSensor() { end(); }

    void begin() {
        if (begun_) return;
        reading_ = {};
        pinMode(triggerPin_, OUTPUT);
        digitalWrite(triggerPin_, LOW);
        pinMode(echoPin_, INPUT); // External divider provides the pull-down.
        attachInterruptArg(echoPin_, onEcho, this, CHANGE);
        begun_ = true;
    }

    void end() {
        if (!begun_) return;
        portENTER_CRITICAL(&mux_);
        capturing_ = false;
        portEXIT_CRITICAL(&mux_);
        detachInterrupt(echoPin_);
        digitalWrite(triggerPin_, LOW);
        pending_ = false;
        begun_ = false;
    }

    // Returns false if unavailable/busy. A stuck-high Echo publishes an
    // immediate failure without emitting a trigger. Poll to consume it.
    bool trigger() {
        if (!begun_ || pending_) return false;
        pending_ = true;
        echoHigh_ = digitalRead(echoPin_) == HIGH;
        portENTER_CRITICAL(&mux_);
        startedUs_ = micros();
        rose_ = false;
        complete_ = false;
        pulseUs_ = 0;
        capturing_ = !echoHigh_;
        portEXIT_CRITICAL(&mux_);
        if (!echoHigh_) {
            digitalWrite(triggerPin_, LOW);
            delayMicroseconds(2);
            digitalWrite(triggerPin_, HIGH);
            delayMicroseconds(10);
            digitalWrite(triggerPin_, LOW);
        }
        return true;
    }

    // True exactly once per completed attempt. Never waits for an echo.
    bool poll() {
        if (!pending_) return false;
        portENTER_CRITICAL(&mux_);
        const bool complete = complete_;
        const uint32_t pulse = pulseUs_;
        const bool timedOut = static_cast<uint32_t>(micros() - startedUs_) >= kTimeoutUs;
        if (complete || timedOut || echoHigh_) capturing_ = false;
        portEXIT_CRITICAL(&mux_);
        if (!complete && !timedOut && !echoHigh_) return false;

        pending_ = false;
        reading_ = {};
        reading_.completedAtMs = millis();
        if (echoHigh_) {
            reading_.status = Status::EchoHigh;
        } else if (!complete) {
            reading_.status = Status::Timeout;
        } else {
            reading_.pulseUs = pulse;
            reading_.distanceCm = pulse * (331.3f + 0.606f * temperatureC_) / 20000.0f;
            reading_.status = reading_.distanceCm >= kMinCm && reading_.distanceCm <= kMaxCm
                ? Status::Valid : Status::OutOfRange;
        }
        return true;
    }

    bool busy() const { return pending_; }
    const Reading& reading() const { return reading_; }

    bool setTemperatureC(float temperatureC) {
        if (!std::isfinite(temperatureC) || temperatureC < -10.0f || temperatureC > 60.0f) return false;
        temperatureC_ = temperatureC;
        return true;
    }
    float temperatureC() const { return temperatureC_; }

    static const char* statusName(Status status) {
        switch (status) {
            case Status::Idle: return "idle";
            case Status::Valid: return "ok";
            case Status::Timeout: return "timeout";
            case Status::OutOfRange: return "out-of-range";
            case Status::EchoHigh: return "echo-high";
        }
        return "unknown";
    }

private:
    static void ARDUINO_ISR_ATTR onEcho(void* context) {
        auto& self = *static_cast<UltrasonicSensor*>(context);
        portENTER_CRITICAL_ISR(&self.mux_);
        const uint32_t nowUs = micros();
        if (self.capturing_ && static_cast<uint32_t>(nowUs - self.startedUs_) < kTimeoutUs) {
            if (digitalRead(self.echoPin_) == HIGH) {
                if (!self.rose_) {
                    self.riseUs_ = nowUs;
                    self.rose_ = true;
                }
            } else if (self.rose_) {
                self.pulseUs_ = nowUs - self.riseUs_;
                self.complete_ = true;
                self.capturing_ = false;
            }
        }
        portEXIT_CRITICAL_ISR(&self.mux_);
    }

    const uint8_t triggerPin_;
    const uint8_t echoPin_;
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
    volatile bool capturing_ = false;
    volatile bool rose_ = false;
    volatile bool complete_ = false;
    volatile uint32_t startedUs_ = 0;
    volatile uint32_t riseUs_ = 0;
    volatile uint32_t pulseUs_ = 0;
    bool begun_ = false;
    bool pending_ = false;
    bool echoHigh_ = false;
    float temperatureC_ = 20.0f;
    Reading reading_;
};

} // namespace sounder
