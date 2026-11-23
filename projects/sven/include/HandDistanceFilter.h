#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace sounder {

// One independent hand tracker. Feed each new range once, then sample its
// continuous output from the main loop. No Arduino or audio dependencies.
class HandDistanceFilter {
public:
    static constexpr float kMinCm = 2.0f;
    static constexpr float kMaxCm = 50.0f;
    static constexpr uint32_t kHoldMs = 250;
    static constexpr float kJumpCm = 4.0f;
    static constexpr float kConfirmCm = 2.0f;
    static constexpr uint32_t kConfirmMs = 200;
    static constexpr float kDeadbandCm = 0.15f;
    static constexpr float kSmoothingMs = 50.0f;
    static constexpr float kMaxSpeedCmPerSecond = 30.0f;

    struct Reading {
        bool valid = false;
        float distanceCm = 0.0f;
        uint32_t acceptedAtMs = 0;
        uint32_t heldJumps = 0;
    };

    void reset() { *this = {}; }
    void resetStatistics() { heldJumps_ = 0; }

    void observe(bool valid, float distanceCm, uint32_t measuredAtMs) {
        const uint32_t elapsed = measuredAtMs - observedAtMs_;
        // A cached or out-of-order reading must not count as confirmation.
        if (observed_ && (elapsed == 0 || elapsed > UINT32_MAX / 2)) return;
        observed_ = true;
        observedAtMs_ = measuredAtMs;
        advance(measuredAtMs);
        if (!valid || !std::isfinite(distanceCm) || distanceCm < kMinCm || distanceCm > kMaxCm) {
            candidate_ = false;
            return;
        }
        if (!initialized_) {
            initialize(distanceCm, measuredAtMs);
            return;
        }
        const bool expired = measuredAtMs - acceptedAtMs_ >= kHoldMs;
        if (expired || std::fabs(distanceCm - anchorCm_) > kJumpCm) {
            if (!candidate_ || measuredAtMs - candidateAtMs_ > kConfirmMs ||
                std::fabs(distanceCm - candidateCm_) > kConfirmCm) {
                candidate_ = true;
                candidateCm_ = distanceCm;
                candidateAtMs_ = measuredAtMs;
                if (!expired) ++heldJumps_;
                return;
            }
            // Reacquire after silence only when the new hand position repeats.
            if (expired) {
                initialize(distanceCm, measuredAtMs);
                return;
            }
        }
        candidate_ = false;
        anchorCm_ = distanceCm;
        acceptedAtMs_ = measuredAtMs;
        if (std::fabs(distanceCm - targetCm_) > kDeadbandCm) targetCm_ = distanceCm;
    }

    Reading sample(uint32_t nowMs) {
        advance(nowMs);
        return {initialized_ && nowMs - acceptedAtMs_ < kHoldMs, currentCm_, acceptedAtMs_, heldJumps_};
    }

private:
    void initialize(float distanceCm, uint32_t measuredAtMs) {
        initialized_ = true;
        candidate_ = false;
        currentCm_ = targetCm_ = anchorCm_ = distanceCm;
        acceptedAtMs_ = advancedAtMs_ = measuredAtMs;
    }

    void advance(uint32_t nowMs) {
        const uint32_t elapsed = nowMs - advancedAtMs_;
        if (elapsed == 0 || elapsed > UINT32_MAX / 2) return;
        advancedAtMs_ = nowMs;
        if (!initialized_) return;
        const float mix = 1.0f - std::exp(-static_cast<float>(elapsed) / kSmoothingMs);
        const float limit = kMaxSpeedCmPerSecond * elapsed / 1000.0f;
        currentCm_ += std::clamp((targetCm_ - currentCm_) * mix, -limit, limit);
    }

    bool initialized_ = false;
    bool observed_ = false;
    bool candidate_ = false;
    float currentCm_ = 0.0f;
    float targetCm_ = 0.0f;
    float anchorCm_ = 0.0f;
    float candidateCm_ = 0.0f;
    uint32_t observedAtMs_ = 0;
    uint32_t acceptedAtMs_ = 0;
    uint32_t advancedAtMs_ = 0;
    uint32_t candidateAtMs_ = 0;
    uint32_t heldJumps_ = 0;
};

} // namespace sounder
