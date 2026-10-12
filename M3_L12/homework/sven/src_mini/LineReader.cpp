#include "LineReader.h"

bool LineReader::readLine(Stream& in) {
    while (in.available() > 0) {
        const char c = in.read();
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            if (m_length < LINE_CAPACITY) {
                m_line[m_length] = '\0';
                m_length = 0U;
                return true;
            }
            m_length = 0U;
        } else if (m_length < LINE_CAPACITY) {
            // A full buffer marks an overlong line; discard it at the newline.
            m_line[m_length++] = c;
        }
    }
    return false;
}

const char* LineReader::line(void) const {
    return m_line;
}
