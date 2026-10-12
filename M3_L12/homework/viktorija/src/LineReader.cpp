#include <Arduino.h>

#include "LineReader.h"

bool LineReader::readLine(Stream& in) {
    while (in.available() > 0) {
        const char c = static_cast<char>(in.read());
        if (c == '\r') { continue; }
        if (c == '\n') {
            m_line[m_len] = '\0';
            m_len = 0U;
            return true;
        }
        if (m_len < LINE_MAX) {
            m_line[m_len++] = c;
        }
    }
    return false;
}

const char* LineReader::line() const {
    return m_line;
}

