#pragma once

#include <Arduino.h>

#include "Synthesizer.h"

namespace sounder {

// Two independent, active-high indicators. Call from the main task only;
// blink timing uses millis() without blocking audio or hand controls.
class StatusLeds {
public:
    static constexpr uint32_t kDisconnectedHalfPeriodMs = 500;

    explicit StatusLeds(uint8_t redPin = 27, uint8_t greenPin = 26)
        : redPin_(redPin), greenPin_(greenPin) {}
    StatusLeds(const StatusLeds&) = delete;
    StatusLeds& operator=(const StatusLeds&) = delete;
    ~StatusLeds() { end(); }

    void begin() {
        if (begun_) return;
        // Preload off before enabling the output drivers.
        digitalWrite(redPin_, LOW);
        digitalWrite(greenPin_, LOW);
        pinMode(redPin_, OUTPUT);
        pinMode(greenPin_, OUTPUT);
        redHigh_ = greenHigh_ = false;
        connectionKnown_ = voiceKnown_ = false;
        begun_ = true;
    }

    void end() {
        if (!begun_) return;
        write(redPin_, redHigh_, false);
        write(greenPin_, greenHigh_, false);
        pinMode(redPin_, INPUT);
        pinMode(greenPin_, INPUT);
        begun_ = false;
    }

    void update(SynthVoice voice, bool connected) {
        if (!begun_) return;
        const uint32_t nowMs = millis();
        if (!connectionKnown_ || connected != connected_) {
            connected_ = connected;
            connectionKnown_ = true;
            redBlink_.reset(nowMs);
        }
        if (!voiceKnown_ || voice != voice_) {
            voice_ = voice;
            voiceKnown_ = true;
            greenBlink_.reset(nowMs);
        }
        write(redPin_, redHigh_, !connected_ && redBlink_.update(nowMs, kDisconnectedHalfPeriodMs));
        write(greenPin_, greenHigh_, greenBlink_.update(nowMs, halfPeriodMs(voice_)));
    }

private:
    struct Blink {
        uint32_t phaseAtMs = 0;
        bool high = true;

        void reset(uint32_t nowMs) { phaseAtMs = nowMs; high = true; }

        bool update(uint32_t nowMs, uint32_t halfPeriodMs) {
            const uint32_t steps = (nowMs - phaseAtMs) / halfPeriodMs;
            if (steps != 0) {
                if (steps % 2 != 0) high = !high;
                phaseAtMs += steps * halfPeriodMs;
            }
            return high;
        }
    };

    static constexpr uint32_t halfPeriodMs(SynthVoice voice) {
        switch (voice) {
            case SynthVoice::Sine: return 1000;     // 0.5 Hz
            case SynthVoice::Triangle: return 500;  // 1 Hz
            case SynthVoice::Pulse: return 250;     // 2 Hz
            case SynthVoice::Fm: return 167;        // approximately 3 Hz
            case SynthVoice::Organ: return 125;     // 4 Hz
        }
        return 1000;
    }

    static void write(uint8_t pin, bool& current, bool high) {
        if (current != high) digitalWrite(pin, high ? HIGH : LOW);
        current = high;
    }

    uint8_t redPin_;
    uint8_t greenPin_;
    Blink redBlink_;
    Blink greenBlink_;
    SynthVoice voice_ = SynthVoice::Sine;
    bool begun_ = false;
    bool connectionKnown_ = false;
    bool voiceKnown_ = false;
    bool connected_ = false;
    bool redHigh_ = false;
    bool greenHigh_ = false;
};

} // namespace sounder
