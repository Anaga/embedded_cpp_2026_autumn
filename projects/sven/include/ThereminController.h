#pragma once

#include "DemoEngine.h"
#include "HandDistanceFilter.h"

#include <algorithm>
#include <cmath>

namespace sounder {

// Maps accepted hand readings to musical targets and holds each axis until it
// receives a new position or the session stops. No GPIO, serial, or audio ownership.
class ThereminController {
public:
    enum class Mode { Off, Pitch, Volume, Both };
    static constexpr float kNearCm = 5.0f;
    static constexpr float kFarCm = HandDistanceFilter::kMaxCm;

    struct Input {
        bool valid = false;
        float distanceCm = 0.0f;
    };

    void start(Mode mode) { mode_ = mode; pitch_ = {}; volume_ = {}; }
    void stop() { start(Mode::Off); }
    Mode mode() const { return mode_; }

    Frame update(Input pitch, Input volume) {
        remember(pitch, pitch_);
        remember(volume, volume_);
        const bool needsPitch = mode_ == Mode::Pitch || mode_ == Mode::Both;
        const bool needsVolume = mode_ == Mode::Volume || mode_ == Mode::Both;
        const float hz = needsPitch && pitch_.seen
            ? 220.0f * std::pow(4.0f, 1.0f - position(pitch_.distanceCm)) : 440.0f;
        const bool ready = mode_ != Mode::Off &&
            (!needsPitch || pitch_.seen) && (!needsVolume || volume_.seen);
        return {hz, ready ? (needsVolume ? 1.0f - position(volume_.distanceCm) : 1.0f) : 0.0f};
    }

    static const char* modeName(Mode mode) {
        switch (mode) {
            case Mode::Off: return "off";
            case Mode::Pitch: return "pitch";
            case Mode::Volume: return "volume";
            case Mode::Both: return "both";
        }
        return "unknown";
    }

private:
    struct Axis {
        bool seen = false;
        float distanceCm = 0.0f;
    };

    static void remember(Input input, Axis& axis) {
        // Ignore distant furniture; 2..5 cm readings clamp to the near endpoint.
        if (input.valid && std::isfinite(input.distanceCm) &&
            input.distanceCm >= 2.0f && input.distanceCm <= kFarCm) {
            axis = {true, input.distanceCm};
        }
    }

    static float position(float distanceCm) {
        return std::clamp((distanceCm - kNearCm) / (kFarCm - kNearCm), 0.0f, 1.0f);
    }

    Mode mode_ = Mode::Off;
    Axis pitch_;
    Axis volume_;
};

} // namespace sounder
