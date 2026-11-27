#pragma once

#include "UltrasonicSensor.h"
#include "HandDistanceFilter.h"

#include <cstring>

namespace sounder {

// Owns the measurement schedule and serial diagnostics, independent of audio.
// Pin defaults match docs/ultrasonic-sensors.md. No GPIO is touched until start.
class UltrasonicDemo {
public:
    enum class Selection { Both, Pitch, Volume };
    enum class Timing { Diagnostic, Live };
    static constexpr uint32_t kSlotMs = 100; // Each sensor at most once per 200 ms.
    static constexpr uint32_t kLiveSlotMs = 35; // Alternating pings: 70 ms per sensor.
    static constexpr uint32_t kStartupMs = 200;
    static constexpr uint32_t kReportIntervalMs = 1000;
    static_assert(kLiveSlotMs * 1000 > UltrasonicSensor::kTimeoutUs,
                  "Live slots must leave time for an echo timeout before the next ping.");

    struct Statistics {
        uint32_t attempts = 0;
        uint32_t valid = 0;
        float minCm = 0.0f;
        float maxCm = 0.0f;
    };

    // GPIO34/35 are input-only; GPIO32/33 supply the trigger pulses.
    UltrasonicDemo(uint8_t pitchTrigger = 32, uint8_t pitchEcho = 34,
                   uint8_t volumeTrigger = 33, uint8_t volumeEcho = 35)
        : pitch_(pitchTrigger, pitchEcho), volume_(volumeTrigger, volumeEcho) {}

    void start(Selection selection = Selection::Both, Timing timing = Timing::Diagnostic) {
        stop();
        selection_ = selection;
        timing_ = timing;
        slotMs_ = timing == Timing::Live ? kLiveSlotMs : kSlotMs;
        resetStatistics();
        pitchFilter_.reset();
        volumeFilter_.reset();
        pitchHand_ = {};
        volumeHand_ = {};
        pitch_.begin();
        volume_.begin();
        active_ = true;
        nextPitch_ = selection != Selection::Volume;
        lastSlotMs_ = millis(); // Also allows newly powered modules to settle.
        slotDelayMs_ = kStartupMs; // Preserve recovery time across restarts.
        Serial.println("Sensors started; logging one combined line per second. Type sensors off to stop.");
    }

    void stop() {
        active_ = false;
        pitch_.end();
        volume_.end();
    }

    void update() {
        if (!active_) return;
        if (pitch_.poll()) record(pitch_.reading(), pitchStats_, pitchFilter_);
        if (volume_.poll()) record(volume_.reading(), volumeStats_, volumeFilter_);
        const uint32_t nowMs = millis();
        if (timing_ == Timing::Live) {
            pitchHand_ = pitchFilter_.sample(nowMs);
            volumeHand_ = volumeFilter_.sample(nowMs);
        }
        report(nowMs);
        if (pitch_.busy() || volume_.busy() || nowMs - lastSlotMs_ < slotDelayMs_) return;
        // No catch-up bursts after a delayed loop, and never simultaneous pings.
        lastSlotMs_ = nowMs;
        slotDelayMs_ = slotMs_;
        if (selection_ == Selection::Both) {
            (nextPitch_ ? pitch_ : volume_).trigger();
            nextPitch_ = !nextPitch_;
        } else {
            // Preserve the per-sensor recovery time when testing only one.
            if (nextPitch_ == (selection_ == Selection::Pitch)) {
                (selection_ == Selection::Pitch ? pitch_ : volume_).trigger();
            }
            nextPitch_ = !nextPitch_;
        }
    }

    // The firmware supplies already-tokenized, lower-case command arguments.
    bool handleCommand(const char* name, const char* argument, const char* extra) {
        if (std::strcmp(name, "sensors") != 0) return false;
        if (extra != nullptr) {
            showUsage();
        } else if (argument == nullptr || std::strcmp(argument, "status") == 0) {
            Serial.printf("Sensors %s | selection %s | %.1f C | %lu ms/sensor | logging 1000 ms/line | 30 ms timeout | 2..450 cm filter\n",
                          active_ ? "on" : "off", selectionName(), pitch_.temperatureC(),
                          static_cast<unsigned long>(measurementIntervalMs()));
        } else if (std::strcmp(argument, "on") == 0 || std::strcmp(argument, "demo") == 0) {
            start();
        } else if (std::strcmp(argument, "pitch") == 0) {
            start(Selection::Pitch);
        } else if (std::strcmp(argument, "volume") == 0) {
            start(Selection::Volume);
        } else if (std::strcmp(argument, "off") == 0) {
            stop();
            Serial.println("Sensors stopped.");
        } else if (std::strcmp(argument, "reset") == 0) {
            resetStatistics();
            Serial.println("Sensor statistics reset.");
        } else {
            showUsage();
        }
        return true;
    }

    bool active() const { return active_; }
    uint32_t measurementIntervalMs() const { return 2 * slotMs_; }
    const UltrasonicSensor::Reading& pitchReading() const { return pitch_.reading(); }
    const UltrasonicSensor::Reading& volumeReading() const { return volume_.reading(); }
    const HandDistanceFilter::Reading& pitchHandReading() const { return pitchHand_; }
    const HandDistanceFilter::Reading& volumeHandReading() const { return volumeHand_; }
    const Statistics& pitchStatistics() const { return pitchStats_; }
    const Statistics& volumeStatistics() const { return volumeStats_; }

private:
    struct ReportTimer {
        uint32_t lastMs = 0;
        bool sent = false;
    };

    void resetStatistics() {
        pitchStats_ = {};
        volumeStats_ = {};
        pitchFilter_.resetStatistics();
        volumeFilter_.resetStatistics();
        reportTimer_ = {};
    }

    const char* selectionName() const {
        return selection_ == Selection::Both ? "both" : selection_ == Selection::Pitch ? "pitch" : "volume";
    }

    static void showUsage() {
        Serial.println("Use sensors [on|demo|pitch|volume|off|status|reset].");
    }

    void record(const UltrasonicSensor::Reading& reading, Statistics& stats, HandDistanceFilter& filter) {
        ++stats.attempts;
        if (timing_ == Timing::Live) {
            filter.observe(reading.status == UltrasonicSensor::Status::Valid, reading.distanceCm, reading.completedAtMs);
        }
        if (reading.status == UltrasonicSensor::Status::Valid) {
            if (stats.valid == 0 || reading.distanceCm < stats.minCm) stats.minCm = reading.distanceCm;
            if (stats.valid == 0 || reading.distanceCm > stats.maxCm) stats.maxCm = reading.distanceCm;
            ++stats.valid;
        }
    }

    void report(uint32_t nowMs) {
        const bool showPitch = selection_ != Selection::Volume;
        const bool showVolume = selection_ != Selection::Pitch;
        // Wait for each selected sensor's first result, including failures.
        if ((showPitch && pitchStats_.attempts == 0) || (showVolume && volumeStats_.attempts == 0) ||
            !Serial || (reportTimer_.sent && nowMs - reportTimer_.lastMs < kReportIntervalMs)) return;
        reportTimer_ = {nowMs, true};
        Serial.printf("[%lu ms] ", static_cast<unsigned long>(nowMs));
        if (showPitch) reportReading("pitch", pitch_.reading(), pitchStats_, pitchHand_);
        if (showPitch && showVolume) Serial.printf("%s", " || ");
        if (showVolume) reportReading("volume", volume_.reading(), volumeStats_, volumeHand_);
        Serial.println("");
    }

    void reportReading(const char* name, const UltrasonicSensor::Reading& reading, const Statistics& stats,
                       const HandDistanceFilter::Reading& hand) const {
        Serial.printf("%s %s | echo %lu us | ", name,
                      UltrasonicSensor::statusName(reading.status), static_cast<unsigned long>(reading.pulseUs));
        if (reading.status == UltrasonicSensor::Status::Valid || reading.status == UltrasonicSensor::Status::OutOfRange) {
            Serial.printf("raw %.2f cm | ", reading.distanceCm);
        } else {
            Serial.printf("%s", "distance n/a | ");
        }
        if (timing_ == Timing::Live) {
            if (hand.valid) Serial.printf("hand %.2f cm%s | ", hand.distanceCm, hand.holding ? " (hold)" : "");
            else Serial.printf("%s", "hand n/a | ");
            Serial.printf("held %lu | ", static_cast<unsigned long>(hand.heldJumps));
        }
        Serial.printf("valid %lu/%lu | ", static_cast<unsigned long>(stats.valid),
                      static_cast<unsigned long>(stats.attempts));
        if (stats.valid != 0) Serial.printf("min %.2f max %.2f cm", stats.minCm, stats.maxCm);
        else Serial.printf("%s", "min/max n/a");
    }

    UltrasonicSensor pitch_;
    UltrasonicSensor volume_;
    HandDistanceFilter pitchFilter_;
    HandDistanceFilter volumeFilter_;
    HandDistanceFilter::Reading pitchHand_;
    HandDistanceFilter::Reading volumeHand_;
    Statistics pitchStats_;
    Statistics volumeStats_;
    ReportTimer reportTimer_;
    Selection selection_ = Selection::Both;
    Timing timing_ = Timing::Diagnostic;
    bool active_ = false;
    bool nextPitch_ = true;
    uint32_t lastSlotMs_ = 0;
    uint32_t slotMs_ = kSlotMs;
    uint32_t slotDelayMs_ = kStartupMs;
};

} // namespace sounder
