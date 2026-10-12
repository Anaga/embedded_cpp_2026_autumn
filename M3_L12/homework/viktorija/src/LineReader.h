#pragma once

#include <Arduino.h>

#include <stdint.h>

class LineReader {
public:
    const char* line() const;

    bool readLine(Stream& in);

private:
    static const size_t LINE_MAX = 31U;
    char m_line[LINE_MAX + 1U] = {};
    size_t m_len = 0U;
};


