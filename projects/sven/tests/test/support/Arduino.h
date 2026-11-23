#pragma once
#ifdef ARDUINO
#error "Native test mocks must never be included in firmware builds."
#endif

#include <array>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>

namespace fake_arduino {
inline uint32_t now = 0;
inline uint32_t microsecondRemainder = 0;
inline std::vector<uint32_t> delays;
struct Interrupt { void (*handler)(void*) = nullptr; void* argument = nullptr; };
struct Write { uint8_t pin; uint8_t value; uint32_t atUs; };
inline std::array<uint8_t, 40> levels{};
inline std::array<uint8_t, 40> modes{};
inline std::array<Interrupt, 40> interrupts{};
inline std::vector<Write> writes;
inline void resetGpio() {
    microsecondRemainder = 0;
    levels = {};
    modes = {};
    interrupts = {};
    writes.clear();
}
inline void edge(uint8_t pin, uint8_t value) {
    const bool changed = levels.at(pin) != value;
    levels.at(pin) = value;
    const auto irq = interrupts.at(pin);
    if (changed && irq.handler != nullptr) irq.handler(irq.argument);
}
}

#define ARDUINO_ISR_ATTR
constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;
constexpr uint8_t INPUT = 1;
constexpr uint8_t OUTPUT = 3;
constexpr int CHANGE = 3;
inline uint32_t millis() { return fake_arduino::now; }
inline uint32_t micros() { return fake_arduino::now * 1000U + fake_arduino::microsecondRemainder; }
inline void delayMicroseconds(uint32_t us) {
    fake_arduino::microsecondRemainder += us;
    fake_arduino::now += fake_arduino::microsecondRemainder / 1000;
    fake_arduino::microsecondRemainder %= 1000;
}
inline void pinMode(uint8_t pin, uint8_t mode) { fake_arduino::modes.at(pin) = mode; }
inline int digitalRead(uint8_t pin) { return fake_arduino::levels.at(pin); }
inline void digitalWrite(uint8_t pin, uint8_t value) {
    fake_arduino::levels.at(pin) = value;
    fake_arduino::writes.push_back({pin, value, micros()});
}
inline void attachInterruptArg(uint8_t pin, void (*handler)(void*), void* argument, int) {
    fake_arduino::interrupts.at(pin) = {handler, argument};
}
inline void detachInterrupt(uint8_t pin) { fake_arduino::interrupts.at(pin) = {}; }

struct FakeSerial {
    bool connected = false;
    unsigned baud = 0;
    unsigned beginCalls = 0;
    std::deque<unsigned char> input;
    std::string output;
    explicit operator bool() const { return connected; }
    void begin(unsigned value) { baud = value; ++beginCalls; }
    int available() const { return static_cast<int>(input.size()); }
    int read() {
        if (input.empty()) return -1;
        const auto value = input.front();
        input.pop_front();
        return value;
    }
    void feed(const std::string& text) { input.insert(input.end(), text.begin(), text.end()); }
    void println(const char* text) { output += text; output += '\n'; }
    template <class... Args>
    void printf(const char* format, Args... args) {
        const int length = std::snprintf(nullptr, 0, format, args...);
        if (length < 0) return;
        std::vector<char> buffer(static_cast<size_t>(length) + 1);
        std::snprintf(buffer.data(), buffer.size(), format, args...);
        output.append(buffer.data(), static_cast<size_t>(length));
    }
};
inline FakeSerial Serial;
inline void delay(uint32_t ms) { fake_arduino::now += ms; fake_arduino::delays.push_back(ms); }
