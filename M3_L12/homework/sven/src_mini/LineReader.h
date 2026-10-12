#pragma once

#include <Arduino.h>
#include <stdint.h>

class LineReader {
public:
    bool readLine(Stream& in);
    // Read the completed line after readLine() returns true.
    const char* line(void) const;

private:
    static const uint8_t LINE_CAPACITY = 32U;
    char m_line[LINE_CAPACITY] = {};
    uint8_t m_length = 0U;
};
