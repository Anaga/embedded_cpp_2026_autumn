#pragma once

#include "Vibrato.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

// No Arduino dependencies: timing and musical behavior can be checked on a host.
namespace sounder {

constexpr float kMinHz = 100.0f;
constexpr float kMaxHz = 8000.0f;
constexpr float kTau = 6.28318530718f;

struct Frame {
    float hz;
    float drive; // Amplitude scale from 0 to 1.
};

struct Pattern {
    const char* name;
    const char* description;
    uint32_t durationMs;
};

static const Pattern kPatterns[] = {
    {"scale", "C major: C4 to C5 and back", 6000},
    {"sweep", "100 to 8000 Hz and back", 6000},
    {"levels", "1000 Hz: amplitude steps 5, 10, 20, 35, 50, 75, 100%", 5600},
    {"vibrato", "440 Hz with 5 Hz pitch modulation (+/-50 cents)", 5000},
    {"tremolo", "440 Hz with 4 Hz amplitude modulation", 5000},
    {"envelope", "880 Hz: repeated attack, decay, sustain, release", 4800},
    {"arp", "Fast C major arpeggio; one note at a time", 4800},
    {"siren", "Continuous glide between 400 and 1600 Hz", 5000},
    {"echo", "440 Hz hit followed by four weaker repeats", 3000},
};
constexpr size_t kPatternCount = sizeof(kPatterns) / sizeof(kPatterns[0]);

inline float midiToHz(unsigned note) {
    return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
}

inline Frame renderPattern(size_t index, uint32_t elapsedMs) {
    if (index >= kPatternCount || elapsedMs >= kPatterns[index].durationMs) {
        return {0.0f, 0.0f};
    }
    const float seconds = elapsedMs / 1000.0f;
    switch (index) {
        case 0: {
            static const unsigned notes[] = {
                60, 62, 64, 65, 67, 69, 71, 72, 71, 69, 67, 65, 64, 62, 60
            };
            return {midiToHz(notes[elapsedMs / 400]),
                    elapsedMs % 400 < 330 ? 1.0f : 0.0f};
        }
        case 1: {
            const float progress = elapsedMs / 6000.0f;
            const float triangle = 1.0f - std::fabs(2.0f * progress - 1.0f);
            return {kMinHz * std::pow(kMaxHz / kMinHz, triangle), 1.0f};
        }
        case 2: {
            static const float levels[] = {0.05f, 0.10f, 0.20f, 0.35f, 0.50f, 0.75f, 1.0f};
            return {1000.0f, elapsedMs % 800 < 650 ? levels[elapsedMs / 800] : 0.0f};
        }
        case 3:
            return {Vibrato::frequency(440.0f, elapsedMs), 1.0f};
        case 4:
            return {440.0f, 0.55f + 0.45f * std::sin(kTau * 4.0f * seconds)};
        case 5: {
            const uint32_t phase = elapsedMs % 1200;
            float drive = 0.0f;
            if (phase < 100) {
                drive = phase / 100.0f;
            } else if (phase < 300) {
                drive = 1.0f - 0.6f * (phase - 100) / 200.0f;
            } else if (phase < 650) {
                drive = 0.4f;
            } else if (phase < 1050) {
                drive = 0.4f * (1050 - phase) / 400.0f;
            }
            return {880.0f, drive};
        }
        case 6: {
            static const unsigned notes[] = {60, 64, 67, 72, 67, 64};
            return {midiToHz(notes[(elapsedMs / 100) % 6]),
                    elapsedMs % 100 < 80 ? 1.0f : 0.0f};
        }
        case 7:
            return {400.0f * std::pow(4.0f, 0.5f + 0.5f * std::sin(kTau * 0.7f * seconds)), 1.0f};
        case 8: {
            static const float repeats[] = {1.0f, 0.55f, 0.30f, 0.16f, 0.08f};
            const size_t repeat = elapsedMs / 500;
            return {440.0f, repeat < 5 && elapsedMs % 500 < 200 ? repeats[repeat] : 0.0f};
        }
    }
    return {0.0f, 0.0f};
}

class DemoEngine {
public:
    enum class Mode { Idle, Tone, Pattern, Tour, Showcase };

    void stop() { mode_ = Mode::Idle; }

    void startTone(uint32_t now, float hz, uint32_t durationMs = 0) {
        if (!std::isfinite(hz) || hz < kMinHz || hz > kMaxHz) {
            stop();
            return;
        }
        mode_ = Mode::Tone;
        startedAt_ = now;
        toneHz_ = hz;
        toneDurationMs_ = durationMs;
    }

    void startPattern(uint32_t now, size_t index) {
        if (index >= kPatternCount) {
            stop();
            return;
        }
        mode_ = Mode::Pattern;
        startedAt_ = now;
        pattern_ = index;
    }

    void startTour(uint32_t now, size_t tourCount = 1) {
        mode_ = tourCount > 1 ? Mode::Showcase : Mode::Tour;
        startedAt_ = now;
        pattern_ = 0;
        tourIndex_ = 0;
        tourCount_ = tourCount;
    }

    Frame sample(uint32_t now) {
        uint32_t elapsed = now - startedAt_; // Unsigned subtraction handles millis() rollover.
        if (mode_ == Mode::Idle) {
            return {0.0f, 0.0f};
        }
        if (mode_ == Mode::Tone) {
            if (toneDurationMs_ != 0 && elapsed >= toneDurationMs_) {
                stop();
                return {0.0f, 0.0f};
            }
            return {toneHz_, 1.0f};
        }
        while (elapsed >= kPatterns[pattern_].durationMs) {
            const bool lastPattern = pattern_ + 1 == kPatternCount;
            if (mode_ == Mode::Pattern || (lastPattern && tourIndex_ + 1 >= tourCount_)) {
                stop();
                return {0.0f, 0.0f};
            }
            const uint32_t duration = kPatterns[pattern_].durationMs;
            startedAt_ += duration;
            elapsed -= duration;
            if (lastPattern) {
                pattern_ = 0;
                ++tourIndex_;
            } else {
                ++pattern_;
            }
        }
        return renderPattern(pattern_, elapsed);
    }

    Mode mode() const { return mode_; }
    size_t pattern() const { return pattern_; }
    size_t tourIndex() const { return tourIndex_; }

private:
    Mode mode_ = Mode::Idle;
    uint32_t startedAt_ = 0;
    uint32_t toneDurationMs_ = 0;
    float toneHz_ = 440.0f;
    size_t pattern_ = 0;
    size_t tourIndex_ = 0;
    size_t tourCount_ = 1;
};

} // namespace sounder
