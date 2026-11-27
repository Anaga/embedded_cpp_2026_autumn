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
    static constexpr uint32_t kTrackingTimeoutMs = 250; // Freeze after a gap; never expires a held value.
    static constexpr float kJumpCm = 4.0f;
    static constexpr float kConfirmCm = 2.0f;
    static constexpr uint32_t kConfirmMs = 200;
    static constexpr float kDeadbandCm = 0.15f;
    static constexpr float kSmoothingMs = 50.0f;
    static constexpr float kMaxSpeedCmPerSecond = 30.0f;

    struct Reading {
        bool valid = false; // At least one accepted position exists, including a held position.
        float distanceCm = 0.0f;
        uint32_t acceptedAtMs = 0;
        uint32_t heldJumps = 0;
        bool holding = false; // Output is frozen while waiting for the hand to return.
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
            tracking_ = false;
            return;
        }
        if (!initialized_) {
            initialize(distanceCm, measuredAtMs);
            return;
        }
        if (!tracking_ || std::fabs(distanceCm - anchorCm_) > kJumpCm) {
            if (!candidate_ || measuredAtMs - candidateAtMs_ > kConfirmMs ||
                std::fabs(distanceCm - candidateCm_) > kConfirmCm) {
                candidate_ = true;
                candidateCm_ = distanceCm;
                candidateAtMs_ = measuredAtMs;
                if (tracking_) ++heldJumps_;
                return;
            }
            // Returning hands establish a new absolute position; no need to
            // move back to the old value or slew across the entire gap.
            if (!tracking_) {
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
        return {initialized_, currentCm_, acceptedAtMs_, heldJumps_, initialized_ && !tracking_};
    }

private:
    void initialize(float distanceCm, uint32_t measuredAtMs) {
        initialized_ = true;
        tracking_ = true;
        candidate_ = false;
        currentCm_ = targetCm_ = anchorCm_ = distanceCm;
        acceptedAtMs_ = advancedAtMs_ = measuredAtMs;
    }

    void advance(uint32_t nowMs) {
        const uint32_t elapsed = nowMs - advancedAtMs_;
        if (elapsed == 0 || elapsed > UINT32_MAX / 2) return;
        advancedAtMs_ = nowMs;
        if (!initialized_ || !tracking_) return;
        if (nowMs - acceptedAtMs_ >= kTrackingTimeoutMs) {
            tracking_ = false;
            return; // Freeze the last output when usable observations cease.
        }
        const float mix = 1.0f - std::exp(-static_cast<float>(elapsed) / kSmoothingMs);
        const float limit = kMaxSpeedCmPerSecond * elapsed / 1000.0f;
        currentCm_ += std::clamp((targetCm_ - currentCm_) * mix, -limit, limit);
    }

    bool initialized_ = false;
    bool tracking_ = false;
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
