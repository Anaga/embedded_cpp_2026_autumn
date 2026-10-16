// LineReader - one line buffer for one Stream (one UART).
//
// A Stream does not hand out lines, only bytes. Each side of the protocol
// needs its own buffer, so the class owns one: when a '\n' arrives the
// buffer holds the line and readLine() returns true.

#pragma once

#include <Arduino.h>
#include <stdint.h>

class LineReader {
public:
  static const uint8_t LINE_MAX = 31U;  // at most 31 characters per line

  // Pumps bytes from in into the internal buffer.
  // Returns true exactly when a complete line is ready; the line is kept in
  // the buffer and is valid until the next call to readLine().
  bool readLine(Stream &in);

  // The line, NUL terminated. Valid after readLine() returned true.
  const char *line() const {
    return m_buffer;
  }

private:
  char m_buffer[LINE_MAX + 1U] = {'\0'};
  uint8_t m_length = 0U;
};
