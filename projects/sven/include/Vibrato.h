#pragma once

#include <cmath>
#include <cstdint>

namespace sounder {

// Shared by the existing demo pattern and the live hand-controlled tone.
class Vibrato {
public:
    static constexpr float kRateHz = 5.0f;
    static constexpr float kDepthCents = 50.0f;

    static float frequency(float centerHz, uint32_t elapsedMs) {
        constexpr float tau = 6.28318530718f;
        const float seconds = elapsedMs / 1000.0f;
        return centerHz * std::pow(2.0f, kDepthCents * std::sin(tau * kRateHz * seconds) / 1200.0f);
    }
};

} // namespace sounder
