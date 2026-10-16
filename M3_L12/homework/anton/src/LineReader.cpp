#include "LineReader.h"

bool LineReader::readLine(Stream &in) {
  while (in.available() > 0U) {
    const int raw = in.read();
    if (raw < 0) {
      break;
    }
    const char c = (char)raw;

    if (c == '\r') {
      // A '\r' is ignored: the line ends with '\n'
      continue;
    }

    if (c == '\n') {
      // End of line: terminate and hand the buffer out
      m_buffer[m_length] = '\0';
      return true;
    }

    if (m_length < LINE_MAX) {
      m_buffer[m_length++] = c;
    } else {
      // Line longer than LINE_MAX: discard it, start a fresh buffer
      m_length = 0U;
    }
  }

  return false;
}
