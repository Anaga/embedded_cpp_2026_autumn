#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace sounder {

enum class SynthVoice { Sine, Triangle, Pulse, Fm, Organ };

struct SynthOption {
    SynthVoice voice;
    const char* name;
};

inline constexpr SynthOption kSynthOptions[] = {
    {SynthVoice::Sine, "sine"},
    {SynthVoice::Triangle, "triangle"},
    {SynthVoice::Pulse, "pulse"},
    {SynthVoice::Fm, "fm"},
    {SynthVoice::Organ, "organ"},
};
constexpr size_t kSynthVoiceCount = sizeof(kSynthOptions) / sizeof(kSynthOptions[0]);

class PhaseSynth {
public:
    void setFrequency(float hz) { phaseStep_ = hz / sampleRate_; }
    void setAmplitude(float amplitude) { amplitude_ = amplitude; }

protected:
    static constexpr float kRadiansPerCycle = 6.28318530718f;

    explicit PhaseSynth(float sampleRate, float phase = 0.0f)
        : sampleRate_(sampleRate), phase_(phase) {}

    float nextPhase() {
        const float phase = phase_;
        phase_ += phaseStep_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        return phase;
    }

    int16_t scale(float wave) const {
        return static_cast<int16_t>(wave * amplitude_);
    }

    float phaseStep_ = 0.0f;

private:
    const float sampleRate_;
    float phase_;
    float amplitude_ = 0.0f;
};

class TriangleSynth : public PhaseSynth {
public:
    explicit TriangleSynth(float sampleRate) : PhaseSynth(sampleRate, 0.25f) {}

    int16_t readSample() {
        return scale(1.0f - 4.0f * std::fabs(nextPhase() - 0.5f));
    }
};

class PulseSynth : public PhaseSynth {
public:
    explicit PulseSynth(float sampleRate) : PhaseSynth(sampleRate) {}

    int16_t readSample() {
        constexpr float duty = 0.25f;
        const float phase = nextPhase();
        const bool high = phase < duty;
        const float fallingPhase = phase - duty + (high ? 1.0f : 0.0f);
        const float pulse = (high ? 1.0f : 0.0f)
                          + 0.5f * (polyBlep(phase) - polyBlep(fallingPhase));
        return scale((pulse - duty) / (1.0f - duty));
    }

private:
    // Smooth the step on either side of an edge to reduce aliasing.
    float polyBlep(float phase) const {
        if (phase < phaseStep_) {
            const float t = phase / phaseStep_ - 1.0f;
            return -t * t;
        }
        if (phase > 1.0f - phaseStep_) {
            const float t = (phase - 1.0f) / phaseStep_ + 1.0f;
            return t * t;
        }
        return 0.0f;
    }
};

class FmRobotSynth : public PhaseSynth {
public:
    explicit FmRobotSynth(float sampleRate) : PhaseSynth(sampleRate) {}

    void setFrequency(float hz) {
        PhaseSynth::setFrequency(hz);
        // Reduce modulation as the upper sidebands approach Nyquist.
        const float room = (0.5f - 3.0f * phaseStep_) / 0.2f;
        modulationIndex_ = 0.9f * std::clamp(room, 0.0f, 1.0f);
    }

    int16_t readSample() {
        const float angle = kRadiansPerCycle * nextPhase();
        return scale(std::sin(angle + modulationIndex_ * std::sin(2.0f * angle)));
    }

private:
    float modulationIndex_ = 0.0f;
};

class OrganSynth : public PhaseSynth {
public:
    explicit OrganSynth(float sampleRate) : PhaseSynth(sampleRate) {}

    void setFrequency(float hz) {
        PhaseSynth::setFrequency(hz);
        secondGain_ = 0.30f * harmonicGain(2.0f * phaseStep_);
        thirdGain_ = 0.15f * harmonicGain(3.0f * phaseStep_);
    }

    int16_t readSample() {
        const float angle = kRadiansPerCycle * nextPhase();
        return scale(0.55f * std::sin(angle)
                     + secondGain_ * std::sin(2.0f * angle)
                     + thirdGain_ * std::sin(3.0f * angle));
    }

private:
    static float harmonicGain(float phaseStep) {
        // Fade each upper harmonic between 80% of Nyquist and Nyquist.
        return std::clamp((0.5f - phaseStep) / 0.1f, 0.0f, 1.0f);
    }

    float secondGain_ = 0.0f;
    float thirdGain_ = 0.0f;
};

template <class Sine>
class Synthesizer {
public:
    Synthesizer(Sine& sine, float sampleRate)
        : sine_(sine), triangle_(sampleRate), pulse_(sampleRate), fm_(sampleRate), organ_(sampleRate) {}

    void select(SynthVoice voice) { voice_ = voice; }

    void setFrequency(float hz) {
        // Keep inactive voices up to date so selection can change during a note.
        sine_.setFrequency(hz);
        triangle_.setFrequency(hz);
        pulse_.setFrequency(hz);
        fm_.setFrequency(hz);
        organ_.setFrequency(hz);
    }

    void setAmplitude(float amplitude) {
        sine_.setAmplitude(amplitude);
        triangle_.setAmplitude(amplitude);
        pulse_.setAmplitude(amplitude);
        fm_.setAmplitude(amplitude);
        organ_.setAmplitude(amplitude);
    }

    int16_t readSample() {
        switch (voice_) {
            case SynthVoice::Sine: return sine_.readSample();
            case SynthVoice::Triangle: return triangle_.readSample();
            case SynthVoice::Pulse: return pulse_.readSample();
            case SynthVoice::Fm: return fm_.readSample();
            case SynthVoice::Organ: return organ_.readSample();
        }
        return 0;
    }

private:
    Sine& sine_;
    TriangleSynth triangle_;
    PulseSynth pulse_;
    FmRobotSynth fm_;
    OrganSynth organ_;
    SynthVoice voice_ = SynthVoice::Sine;
};

} // namespace sounder
