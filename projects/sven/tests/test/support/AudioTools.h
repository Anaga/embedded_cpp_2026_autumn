#pragma once
#include <Arduino.h>
#include <cmath>
#include <functional>

namespace fake_audio {
inline bool beginFails = false;
inline std::function<void()> afterSample;
}

namespace audio_tools {
struct AudioInfo {
    uint32_t sample_rate;
    int channels;
    int bits_per_sample;
    AudioInfo(uint32_t rate, int count, int bits)
        : sample_rate(rate), channels(count), bits_per_sample(bits) {}
};

// The vendor oscillator is compiled in the board build. This native stand-in
// exposes clock/gain changes so tests check how the real controller uses it.
template <class T>
class FastIntSineGenerator {
public:
    AudioInfo info{44100, 1, 16};
    float hz = 0;
    float amplitude = 0;
    double phase = 0;
    unsigned beginCalls = 0;
    unsigned samples = 0;
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
    bool begin(AudioInfo value, float frequency) {
        ++beginCalls;
        info = value;
        hz = frequency;
        return !fake_audio::beginFails;
    }
    void setFrequency(float value) { hz = value; frequencies.push_back(value); }
    void setAmplitude(float value) { amplitude = value; amplitudes.push_back(value); }
    T readSample() {
        const T result = static_cast<T>(amplitude * std::sin(phase * 6.283185307179586));
        phase = std::fmod(phase + hz / info.sample_rate, 1.0);
        ++samples;
        if (fake_audio::afterSample) {
            auto callback = std::move(fake_audio::afterSample);
            fake_audio::afterSample = nullptr;
            callback();
        }
        return result;
    }
};
}
