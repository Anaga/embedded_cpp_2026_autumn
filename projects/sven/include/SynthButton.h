#pragma once

#include <Arduino.h>

#include "Synthesizer.h"

namespace sounder {

// Normally open button to GND. Short presses select on release; a long hold
// emits once at the threshold and suppresses the short-press event.
class SynthButton {
public:
    static constexpr uint32_t kDebounceMs = 30;
    static constexpr uint32_t kLongPressMs = 3000;
    enum class Event { None, ShortPress, LongPress };

    explicit SynthButton(uint8_t pin = 25) : pin_(pin) {}

    void begin() {
        if (begun_) return;
        pinMode(pin_, INPUT_PULLUP);
        begun_ = true;
    }

    Event poll() {
        if (!begun_) return Event::None;
        const uint32_t nowMs = millis();
        const bool pressed = digitalRead(pin_) == LOW;
        if (!observed_) {
            // Establish the baseline after initialization, so a held boot
            // button needs release before it can produce either event.
            stablePressed_ = rawPressed_ = pressed;
            changedAtMs_ = nowMs;
            observed_ = true;
            return Event::None;
        }
        if (pressed != rawPressed_) {
            rawPressed_ = pressed;
            changedAtMs_ = nowMs;
        }
        if (rawPressed_ != stablePressed_ && nowMs - changedAtMs_ >= kDebounceMs) {
            stablePressed_ = rawPressed_;
            if (stablePressed_) {
                pressedAtMs_ = changedAtMs_;
                activePress_ = true;
                longReported_ = false;
            } else if (activePress_) {
                activePress_ = false;
                if (!longReported_) {
                    // Use the release edge, excluding its debounce time. A late
                    // poll still recognizes a completed long hold exactly once.
                    return changedAtMs_ - pressedAtMs_ >= kLongPressMs
                        ? Event::LongPress : Event::ShortPress;
                }
            }
        }
        if (activePress_ && rawPressed_ && !longReported_ && nowMs - pressedAtMs_ >= kLongPressMs) {
            longReported_ = true;
            return Event::LongPress;
        }
        return Event::None;
    }

    static constexpr SynthVoice nextVoice(SynthVoice current) {
        for (size_t i = 0; i < kSynthVoiceCount; ++i) {
            if (kSynthOptions[i].voice == current) return kSynthOptions[(i + 1) % kSynthVoiceCount].voice;
        }
        return kSynthOptions[0].voice;
    }

private:
    uint8_t pin_;
    uint32_t changedAtMs_ = 0;
    uint32_t pressedAtMs_ = 0;
    bool begun_ = false;
    bool observed_ = false;
    bool stablePressed_ = false;
    bool rawPressed_ = false;
    bool activePress_ = false;
    bool longReported_ = false;
};

} // namespace sounder
