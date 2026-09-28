#pragma once

#ifdef ARDUINO
#error "This Arduino mock is for native tests only."
#endif

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;
constexpr uint8_t OUTPUT = 3;
constexpr uint8_t ADC_11db = 3;

namespace fake_arduino {
using PinSetting = std::pair<uint8_t, uint8_t>;
using PinWrite = std::tuple<uint8_t, uint8_t, uint64_t>;

inline uint64_t time_us = 0;
inline std::vector<uint16_t> analog_inputs;
inline std::vector<uint8_t> analog_read_pins;
inline std::vector<uint64_t> analog_read_times;
inline std::vector<uint32_t> millisecond_delays;
inline std::vector<uint32_t> microsecond_delays;
inline std::vector<PinSetting> pin_modes;
inline std::array<int, 8> pin_levels;
inline std::vector<PinWrite> pin_writes;
inline std::vector<uint8_t> adc_resolutions;
inline std::vector<PinSetting> adc_attenuations;

inline void reset() {
    time_us = 0;
    analog_inputs.clear();
    analog_read_pins.clear();
    analog_read_times.clear();
    millisecond_delays.clear();
    microsecond_delays.clear();
    pin_modes.clear();
    pin_levels.fill(-1);
    pin_writes.clear();
    adc_resolutions.clear();
    adc_attenuations.clear();
}
}  // namespace fake_arduino

struct FakeSerial {
    std::string output;
    std::vector<std::pair<uint32_t, uint64_t>> begin_calls;

    void begin(uint32_t baud) {
        begin_calls.emplace_back(baud, fake_arduino::time_us);
    }

    void print(unsigned int value) { output += std::to_string(value); }
    void println() { output += "\r\n"; }
    void println(const char* value) {
        output += value;
        println();
    }

    template <typename... Args>
    size_t printf(const char* format, Args... args) {
        const int length = std::snprintf(nullptr, 0, format, args...);
        if (length < 0) {
            return 0;
        }
        std::vector<char> buffer(static_cast<size_t>(length) + 1);
        std::snprintf(buffer.data(), buffer.size(), format, args...);
        output.append(buffer.data(), static_cast<size_t>(length));
        return static_cast<size_t>(length);
    }
};

inline FakeSerial Serial;

inline uint32_t millis() {
    // Match the ESP32's 32-bit millisecond rollover on a 64-bit host.
    return static_cast<uint32_t>(fake_arduino::time_us / 1000);
}

inline void delay(uint32_t milliseconds) {
    fake_arduino::millisecond_delays.push_back(milliseconds);
    fake_arduino::time_us += static_cast<uint64_t>(milliseconds) * 1000;
}

inline void delayMicroseconds(uint32_t microseconds) {
    fake_arduino::microsecond_delays.push_back(microseconds);
    fake_arduino::time_us += microseconds;
}

inline uint16_t analogRead(uint8_t pin) {
    const auto value = fake_arduino::analog_inputs.at(
        fake_arduino::analog_read_pins.size());
    fake_arduino::analog_read_pins.push_back(pin);
    fake_arduino::analog_read_times.push_back(fake_arduino::time_us);
    return value;
}

inline void pinMode(uint8_t pin, uint8_t mode) {
    fake_arduino::pin_modes.emplace_back(pin, mode);
}

inline void digitalWrite(uint8_t pin, uint8_t level) {
    fake_arduino::pin_levels.at(pin) = level;
    fake_arduino::pin_writes.emplace_back(pin, level, fake_arduino::time_us);
}

inline void analogReadResolution(uint8_t bits) {
    fake_arduino::adc_resolutions.push_back(bits);
}

inline void analogSetPinAttenuation(uint8_t pin, uint8_t attenuation) {
    fake_arduino::adc_attenuations.emplace_back(pin, attenuation);
}
