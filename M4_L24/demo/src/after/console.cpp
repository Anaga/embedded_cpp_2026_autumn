#include "console.h"

#include <WiFi.h>
#include <stdlib.h>
#include <string.h>

#include "../common/wifi_store.h"

void Console::begin() {
  Serial.println("Type 'help' for commands.");
}

void Console::finishLine() {
  if (tooLong_) {
    Serial.println("line too long - ignored");
  } else if (len_ > 0U) {
    line_[len_] = '\0';
    execute(line_);
  }
  len_     = 0U;
  tooLong_ = false;
}

void Console::update() {
  // Only the bytes that have already arrived - no readStringUntil(),
  // which would wait for the rest of the line.
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());
    lastByteMs_ = millis();
    // Terminals end a line with CR LF, LF or CR alone: any of them ends it.
    // The empty "line" between CR and LF is ignored in finishLine().
    if ((c == '\r') || (c == '\n')) {
      finishLine();
      continue;
    }
    if ((c == '\b') || (c == 0x7F)) {   // backspace / DEL: a typo corrected
      if (len_ > 0U) {
        len_--;
      }
      continue;
    }
    if ((c < ' ') || (c > '~')) {
      continue;                      // other control or non-ASCII bytes
    }
    if (len_ < MAX_LINE_LEN) {
      line_[len_++] = c;
    } else {
      tooLong_ = true;
    }
  }

  // Some terminals send no line ending at all (VS Code Serial Monitor with
  // line ending "None"). They send the whole line at once, so a short quiet
  // time after the last byte also ends the line.
  if (((len_ > 0U) || tooLong_) && (millis() - lastByteMs_ >= LINE_IDLE_MS)) {
    finishLine();
  }
}

// Splits in place on spaces. "Home Net" in quotes stays one argument.
uint8_t Console::split(char *line, char *argv[], uint8_t max) {
  uint8_t argc = 0U;
  char *p = line;
  while ((*p != '\0') && (argc < max)) {
    while (*p == ' ') {
      p++;
    }
    if (*p == '\0') {
      break;
    }
    if (*p == '"') {
      p++;
      argv[argc++] = p;
      while ((*p != '\0') && (*p != '"')) {
        p++;
      }
    } else {
      argv[argc++] = p;
      while ((*p != '\0') && (*p != ' ')) {
        p++;
      }
    }
    if (*p != '\0') {
      *p++ = '\0';
    }
  }
  return argc;
}

void Console::execute(char *line) {
  char *argv[MAX_ARGS] = {};
  uint8_t argc = split(line, argv, MAX_ARGS);
  if (argc == 0U) {
    return;
  }

  if (strcmp(argv[0], "help") == 0) {
    printHelp();
  } else if (strcmp(argv[0], "status") == 0) {
    status_();
  } else if (strcmp(argv[0], "wifi") == 0) {
    wifiCommand(argc, argv);
  } else if (strcmp(argv[0], "reboot") == 0) {
    Serial.println("rebooting...");
    Serial.flush();
    ESP.restart();
  } else {
    Serial.printf("unknown command '%s' - type 'help'\n", argv[0]);
  }
}

void Console::wifiCommand(uint8_t argc, char *argv[]) {
  const char *sub = (argc >= 2U) ? argv[1] : "list";

  if (strcmp(sub, "list") == 0) {
    wifiList();
  } else if ((strcmp(sub, "add") == 0) && (argc >= 3U)) {
    const char *pass = (argc >= 4U) ? argv[3] : "";
    wifi_store::Result r = wifi_store::add(argv[2], pass);
    Serial.printf("wifi add \"%s\": %s\n", argv[2], wifi_store::resultText(r));
    if ((r == wifi_store::Result::ADDED) || (r == wifi_store::Result::REPLACED)) {
      Serial.println("type 'reboot' to use it now");
    }
  } else if ((strcmp(sub, "del") == 0) && (argc >= 3U)) {
    long n = strtol(argv[2], nullptr, 10);
    bool ok = (n >= 1L) && (n <= static_cast<long>(wifi_store::MAX_NETWORKS)) &&
              wifi_store::remove(static_cast<uint8_t>(n - 1L));
    Serial.println(ok ? "deleted" : "no such network - see 'wifi list'");
  } else if (strcmp(sub, "clear") == 0) {
    wifi_store::clear();
    Serial.println("all networks deleted");
  } else {
    Serial.println("usage: wifi list | wifi add \"<ssid>\" [password] | wifi del <n> | wifi clear");
  }
}

void Console::wifiList() {
  wifi_store::Network nets[wifi_store::MAX_NETWORKS];
  uint8_t count = wifi_store::load(nets, wifi_store::MAX_NETWORKS);
  if (count == 0U) {
    Serial.println("no networks saved - wifi add \"<ssid>\" <password>");
    return;
  }
  bool connected = WiFi.isConnected();
  String now = connected ? WiFi.SSID() : String("");
  for (uint8_t i = 0U; i < count; i++) {
    bool active = connected && (now == nets[i].ssid);
    // Never print the password: the monitor may be on the projector.
    Serial.printf("%u: \"%s\"%s%s\n", static_cast<unsigned>(i + 1U), nets[i].ssid,
                  (nets[i].pass[0] == '\0') ? " (open)" : "",
                  active ? "  <- connected" : "");
  }
}

void Console::printHelp() {
  Serial.println("help                          this list");
  Serial.println("status                        board id, sensor, LED, WiFi, loop");
  Serial.println("wifi list                     saved networks");
  Serial.println("wifi add \"<ssid>\" <password>  save a network");
  Serial.println("wifi del <n>                  delete network n");
  Serial.println("wifi clear                    delete all networks");
  Serial.println("reboot                        restart, use new networks");
}
