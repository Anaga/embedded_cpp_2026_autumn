// Commands typed into the serial monitor, one line at a time, never waiting.
//
//   help                          this list
//   status                        WiFi and the latest packet
//   wifi list                     saved networks
//   wifi add "<ssid>" <password>  save a network (quotes if the name has spaces)
//   wifi add "<ssid>"             an open network, no password
//   wifi del <n>                  delete network number n (see wifi list)
//   wifi clear                    delete all networks
//   reboot                        restart the board (new networks are used then)
//
// platformio.ini has monitor_echo = yes and send_on_enter, so in the
// PlatformIO monitor you see what you type and the line is sent on Enter.
// Other terminals work too: CR, LF, CR LF or no line ending at all.
// Every command answers with at least one line - silence means the
// board never got the line.
#pragma once
#include <Arduino.h>

class Console {
 public:
  using StatusFn = void (*)();

  explicit Console(StatusFn status) : status_(status) {}

  void begin();
  void update();                     // reads what has arrived, returns at once

 private:
  static constexpr size_t MAX_LINE_LEN = 120U;
  static constexpr uint8_t MAX_ARGS = 4U;
  static constexpr uint32_t LINE_IDLE_MS = 300U;  // no line ending: quiet time ends the line

  void finishLine();
  void execute(char *line);
  void wifiCommand(uint8_t argc, char *argv[]);
  static uint8_t split(char *line, char *argv[], uint8_t max);
  static void printHelp();
  static void wifiList();

  StatusFn status_;
  char     line_[MAX_LINE_LEN + 1U] = {};
  size_t   len_      = 0U;
  bool     tooLong_  = false;
  uint32_t lastByteMs_ = 0U;
};
