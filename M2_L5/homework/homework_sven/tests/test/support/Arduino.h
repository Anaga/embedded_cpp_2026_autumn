#pragma once

// Only the Arduino API used by this assignment is implemented here.
// This header belongs exclusively to native test builds.
#ifdef ARDUINO
#error "The desktop Arduino substitute must not be used in firmware builds."
#endif

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace fake_arduino {
inline uint16_t analog_counts = 0U;
// Optional sequence for testing noisy input. Fall back to analog_counts once
// it is consumed, while keeping track of every ADC read.
inline std::vector<uint16_t> analog_samples;
inline size_t analog_sample_index = 0U;
inline uint32_t analog_read_count = 0U;
inline uint64_t elapsed_us = 0U;
inline std::string serial_output;

inline void reset() {
    analog_counts = 0U;
    analog_samples.clear();
    analog_sample_index = 0U;
    analog_read_count = 0U;
    elapsed_us = 0U;
    serial_output.clear();
}

inline void advanceMilliseconds(uint32_t milliseconds) {
    elapsed_us += static_cast<uint64_t>(milliseconds) * 1000U;
}
} // namespace fake_arduino

inline int analogRead(uint8_t) {
    ++fake_arduino::analog_read_count;
    if (fake_arduino::analog_sample_index < fake_arduino::analog_samples.size()) {
        return fake_arduino::analog_samples[fake_arduino::analog_sample_index++];
    }
    return fake_arduino::analog_counts;
}

inline uint32_t millis() {
    return static_cast<uint32_t>(fake_arduino::elapsed_us / 1000U);
}

inline void delay(uint32_t milliseconds) {
    fake_arduino::advanceMilliseconds(milliseconds);
}

inline void delayMicroseconds(uint32_t microseconds) {
    fake_arduino::elapsed_us += microseconds;
}

inline constexpr int ADC_11db = 3;
inline void analogReadResolution(uint8_t) {}
inline void analogSetPinAttenuation(uint8_t, int) {}

class TestSerial {
public:
    void begin(uint32_t) {}

    void println(const char* text = "") {
        write(std::string(text) + '\n');
    }

    template <typename... Args>
    int printf(const char* format, Args... args) {
        const int length = std::snprintf(nullptr, 0, format, args...);
        if (length < 0) {
            return length;
        }
        std::vector<char> buffer(static_cast<size_t>(length) + 1U);
        std::snprintf(buffer.data(), buffer.size(), format, args...);
        write(std::string(buffer.data(), static_cast<size_t>(length)));
        return length;
    }

private:
    static void write(const std::string& text) {
        fake_arduino::serial_output += text;
    }
};

inline TestSerial Serial;
