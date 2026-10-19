#pragma once

#include <stdint.h>
#include <Adafruit_AHTX0.h>

#include "RingBuffer.h"

class Aht10 {
public:
    static constexpr uint8_t HISTORY_CAPACITY = 14U;
    using History = RingBuffer<float, HISTORY_CAPACITY>;

    void begin(void);
    // Reads and caches both values; each fifth successful read adds an average.
    void read(void);

    bool valid(void) const { return m_valid; }
    float temperature(void) const { return m_temperature; }
    float humidity(void) const { return m_humidity; }
    const History &history(void) const { return m_history; }
    // True if the most recent read() added a history entry.
    bool historyUpdated(void) const { return m_historyUpdated; }

private:
    Adafruit_AHTX0 m_sensor;
    bool m_initialized = false;
    bool m_valid = false;
    bool m_historyUpdated = false;
    float m_temperature = 0.0F;
    float m_humidity = 0.0F;
    float m_temperatureSum = 0.0F;
    uint8_t m_readingCount = 0U;
    History m_history;
};
