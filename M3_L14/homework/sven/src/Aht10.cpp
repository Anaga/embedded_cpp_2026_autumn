#include "Aht10.h"

#include <Wire.h>

static const uint8_t READINGS_PER_AVERAGE = 5U;

void Aht10::begin(void) {
    Wire.begin();
    m_initialized = m_sensor.begin();
}

void Aht10::read(void) {
    m_valid = false;
    m_historyUpdated = false;

    if (!m_initialized) {
        m_initialized = m_sensor.begin();
    }
    if (!m_initialized) {
        return;
    }

    sensors_event_t humidity = {};
    sensors_event_t temperature = {};
    if (!m_sensor.getEvent(&humidity, &temperature)) {
        m_initialized = false;
        return;
    }

    m_temperature = temperature.temperature;
    m_humidity = humidity.relative_humidity;
    m_valid = true;

    m_temperatureSum += m_temperature;
    ++m_readingCount;
    if (m_readingCount == READINGS_PER_AVERAGE) {
        m_history.push(m_temperatureSum / static_cast<float>(READINGS_PER_AVERAGE));
        m_temperatureSum = 0.0F;
        m_readingCount = 0U;
        m_historyUpdated = true;
    }
}
