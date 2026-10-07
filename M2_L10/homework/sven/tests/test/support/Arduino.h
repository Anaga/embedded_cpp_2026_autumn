#pragma once

#ifdef ARDUINO
#error "This Arduino mock is for native tests only."
#endif

#include <array>
#include <cinttypes>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

constexpr uint8_t LOW = 0U;
constexpr uint8_t HIGH = 1U;
constexpr uint8_t INPUT_PULLUP = 2U;

namespace fake_arduino {
inline uint32_t now_ms = 0U;
inline std::array<uint8_t, 256> inputs;
inline std::array<int, 256> pin_modes;
inline std::array<int, 256> attached_channels;
inline std::array<uint32_t, 3> frequencies;
inline std::array<uint8_t, 3> resolutions;
inline std::array<uint32_t, 3> duties;
inline std::vector<uint8_t> reads;
inline std::vector<std::pair<uint8_t, uint32_t>> writes;
inline std::vector<uint32_t> delays;
inline std::vector<std::pair<long, long>> random_ranges;
inline long random_value = 2000L;

inline void reset() {
    now_ms = 0U;
    inputs.fill(HIGH);
    pin_modes.fill(-1);
    attached_channels.fill(-1);
    frequencies.fill(0U);
    resolutions.fill(0U);
    duties.fill(0U);
    reads.clear();
    writes.clear();
    delays.clear();
    random_ranges.clear();
    random_value = 2000L;
}
}  // namespace fake_arduino

struct FakeSerial {
    uint32_t baud = 0U;
    std::string output;

    void begin(uint32_t value) { baud = value; }
    void println(const char *text = "") { output += std::string(text) + '\n'; }

    template <typename... Args>
    void printf(const char *format, Args... args) {
        // ESP32 uint32_t is unsigned long; native uint32_t may be unsigned int.
        std::string native_format(format);
        for (std::size_t pos = 0; (pos = native_format.find("%lu", pos)) != std::string::npos;) {
            native_format.replace(pos, 3, "%" PRIu32);
            pos += std::string("%" PRIu32).size();
        }
        const int size = std::snprintf(nullptr, 0, native_format.c_str(), args...);
        if (size < 0) {
            output += "<format error>";
            return;
        }
        std::vector<char> buffer(static_cast<std::size_t>(size) + 1U);
        std::snprintf(buffer.data(), buffer.size(), native_format.c_str(), args...);
        output.append(buffer.data(), static_cast<std::size_t>(size));
    }
};

inline FakeSerial Serial;

inline uint32_t millis() { return fake_arduino::now_ms; }
inline void delay(uint32_t duration) {
    fake_arduino::delays.push_back(duration);
    fake_arduino::now_ms += duration;
}
inline void pinMode(uint8_t pin, uint8_t mode) { fake_arduino::pin_modes.at(pin) = mode; }
inline int digitalRead(uint8_t pin) {
    fake_arduino::reads.push_back(pin);
    return fake_arduino::inputs.at(pin);
}
inline double ledcSetup(uint8_t channel, uint32_t frequency, uint8_t bits) {
    fake_arduino::frequencies.at(channel) = frequency;
    fake_arduino::resolutions.at(channel) = bits;
    return frequency;
}
inline void ledcAttachPin(uint8_t pin, uint8_t channel) {
    fake_arduino::attached_channels.at(pin) = channel;
}
inline void ledcWrite(uint8_t channel, uint32_t duty) {
    fake_arduino::duties.at(channel) = duty;
    fake_arduino::writes.emplace_back(channel, duty);
}
inline long random(long minimum, long maximum) {
    fake_arduino::random_ranges.emplace_back(minimum, maximum);
    return fake_arduino::random_value;
}
