#pragma once

#include "DemoEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace sounder {

// Audio-owned interpolation for live controls, preserving oscillator phase.
class LiveTone {
public:
    static constexpr uint32_t kGlideMs = 20;
    static constexpr uint32_t kFadeMs = 15;

    void reset() { from_ = target_ = current_ = {440.0f, 0.0f}; startedAtMs_ = 0; }

    void setTarget(Frame target, uint32_t nowMs) {
        if (!std::isfinite(target.hz) || target.hz < kMinHz || target.hz > kMaxHz ||
            !std::isfinite(target.drive) || target.drive < 0.0f || target.drive > 1.0f) {
            target = {440.0f, 0.0f};
        }
        if (target.hz == target_.hz && target.drive == target_.drive) return;
        from_ = sample(nowMs);
        target_ = target;
        startedAtMs_ = nowMs;
    }

    Frame sample(uint32_t nowMs) {
        const uint32_t elapsed = nowMs - startedAtMs_;
        const float pitchMix = std::min(elapsed / static_cast<float>(kGlideMs), 1.0f);
        const float gainMix = std::min(elapsed / static_cast<float>(kFadeMs), 1.0f);
        current_ = {from_.hz + (target_.hz - from_.hz) * pitchMix,
                    from_.drive + (target_.drive - from_.drive) * gainMix};
        return current_;
    }

private:
    Frame from_{440.0f, 0.0f};
    Frame target_{440.0f, 0.0f};
    Frame current_{440.0f, 0.0f};
    uint32_t startedAtMs_ = 0;
};

} // namespace sounder
