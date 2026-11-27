#pragma once

#include "DemoEngine.h"
#include "LiveTone.h"
#include "Synthesizer.h"

namespace sounder {

constexpr uint32_t kSampleRate = 44100;
constexpr uint32_t kControlMs = 5;
constexpr float kDefaultLevel = 35.0f;

struct PlaybackStatus {
    DemoEngine::Mode mode = DemoEngine::Mode::Idle;
    size_t pattern = 0;
    float hz = 0.0f;
    float drive = 0.0f;
    SynthVoice voice = SynthVoice::Sine;
    size_t tourIndex = 0;
    bool live = false;
};

// The audio callback alone owns this object and its synthesizer. The template
// keeps sequencing testable without Bluetooth, Arduino, or a second clock.
template <class Synth>
class Playback {
public:
    explicit Playback(Synth& synth, uint32_t initialMs = 0)
        : synth_(synth), nowMs_(initialMs), lastControlMs_(initialMs) {}

    void startTone(float hz, uint32_t durationMs = 0) {
        live_ = false;
        engine_.startTone(nowMs_, hz, durationMs);
        refresh_ = true;
    }

    void startPattern(size_t pattern) {
        live_ = false;
        engine_.startPattern(nowMs_, pattern);
        refresh_ = true;
    }

    void startTour() {
        live_ = false;
        engine_.startTour(nowMs_);
        refresh_ = true;
    }

    void startShowcase() {
        live_ = false;
        engine_.startTour(nowMs_, kSynthVoiceCount);
        refresh_ = true;
    }

    void stop() {
        live_ = false;
        liveTone_.reset();
        engine_.stop();
        frame_ = {0.0f, 0.0f};
        refresh_ = true;
    }

    void setLiveTarget(Frame target) {
        if (!live_) {
            engine_.stop();
            liveTone_.reset();
            live_ = true;
            liveStartedAtMs_ = nowMs_;
        }
        liveTone_.setTarget(target, nowMs_);
    }

    void setLevel(float level) {
        if (std::isfinite(level) && level >= 0.0f && level <= 100.0f && level != level_) {
            level_ = level;
            refresh_ = true;
        }
    }

    void setVoice(SynthVoice voice) {
        voice_ = voice;
        synth_.select(activeVoice());
    }

    // Keep oscillators, sequencing, and live controls running while silencing
    // the output. Unmuting resumes the current sound without resetting it.
    void setMuted(bool muted) { muted_ = muted; }

    int16_t nextSample() {
        if (refresh_ || nowMs_ - lastControlMs_ >= kControlMs) {
            lastControlMs_ = nowMs_;
            refresh_ = false;
            frame_ = live_ ? liveTone_.sample(nowMs_) : engine_.sample(nowMs_);
            if (live_) frame_.hz = Vibrato::frequency(frame_.hz, nowMs_ - liveStartedAtMs_);
            synth_.select(activeVoice());
            if (frame_.hz > 0.0f && frame_.hz != lastHz_) {
                synth_.setFrequency(frame_.hz);
                lastHz_ = frame_.hz;
            }
            synth_.setAmplitude(32767.0f * frame_.drive * level_ / 100.0f);
        }
        const int16_t sample = synth_.readSample();
        // One tick per stereo frame, not per channel. The remainder avoids
        // drift from rounding 44.1 samples/ms; unsigned time also wraps safely.
        fraction_ += 1000;
        if (fraction_ >= kSampleRate) {
            fraction_ -= kSampleRate;
            ++nowMs_;
        }
        return muted_ ? 0 : sample;
    }

    PlaybackStatus status() const {
        return {live_ ? DemoEngine::Mode::Tone : engine_.mode(), engine_.pattern(), frame_.hz, frame_.drive,
                activeVoice(), engine_.tourIndex(), live_};
    }

    uint32_t timeMs() const { return nowMs_; }

private:
    SynthVoice activeVoice() const {
        return engine_.mode() == DemoEngine::Mode::Showcase
            ? kSynthOptions[engine_.tourIndex()].voice : voice_;
    }

    Synth& synth_;
    DemoEngine engine_;
    LiveTone liveTone_;
    bool live_ = false;
    uint32_t liveStartedAtMs_ = 0;
    Frame frame_{0.0f, 0.0f};
    float level_ = kDefaultLevel;
    float lastHz_ = 0.0f;
    SynthVoice voice_ = SynthVoice::Sine;
    uint32_t nowMs_ = 0;
    uint32_t fraction_ = 0;
    uint32_t lastControlMs_ = 0;
    bool refresh_ = true;
    bool muted_ = false;
};

} // namespace sounder
