#include "console.h"
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include "WiFiStore.h"
#include "shared.h"

namespace {

void printIp(const char *label, IPAddress ip) {
  Serial.printf("%s: %u.%u.%u.%u\n", label, ip[0], ip[1], ip[2], ip[3]);
}

}  // namespace

void Console::begin() {
  len_ = 0U;
  tooLong_ = false;
  lastByteMs_ = 0U;
  Serial.println(F("Console ready. Type 'help' for commands."));
}

void Console::update() {
  while (Serial.available()) {
    char c = Serial.read();
    lastByteMs_ = millis();

    if (c == '\r' || c == '\n') {
      finishLine();
    } else if (len_ < MAX_LINE_LEN) {
      line_[len_++] = c;
      tooLong_ = false;
    } else {
      tooLong_ = true;
    }
  }

  // No line ending received for a while -> treat as end of line
  if (len_ > 0 && millis() - lastByteMs_ >= LINE_IDLE_MS) {
    finishLine();
  }
}

void Console::finishLine() {
  if (tooLong_) {
    Serial.println(F("Line too long, ignored"));
  } else {
    line_[len_] = '\0';
    if (len_ > 0) {
      execute(line_);
    }
  }
  len_ = 0U;
  tooLong_ = false;
}

void Console::execute(char *line) {
  char *argv[MAX_ARGS];
  uint8_t argc = split(line, argv, MAX_ARGS);
  if (argc == 0) return;

  if (strcmp(argv[0], "help") == 0) {
    printHelp();
  } else if (strcmp(argv[0], "status") == 0) {
    if (status_) status_();
  } else if (strcmp(argv[0], "wifi") == 0) {
    wifiCommand(argc, argv);
  } else if (strcmp(argv[0], "reboot") == 0) {
    Serial.println(F("Rebooting..."));
    ESP.restart();
  } else {
    Serial.printf("Unknown command: %s\n", argv[0]);
    printHelp();
  }
}

void Console::wifiCommand(uint8_t argc, char *argv[]) {
  if (argc < 2) {
    printHelp();
    return;
  }

  wifi_store::WiFiStore store;
  store.begin();

  if (strcmp(argv[1], "list") == 0) {
    wifiList();
  } else if (strcmp(argv[1], "add") == 0) {
    if (argc < 3) {
      Serial.println(F("Usage: wifi add \"<ssid>\" [password]"));
      return;
    }
    const char *ssid = argv[2];
    const char *pass = (argc >= 4) ? argv[3] : "";
    if (store.save(ssid, pass)) {
      Serial.printf("Saved: %s\n", ssid);
    } else {
      Serial.println(F("Failed to save"));
    }
  } else if (strcmp(argv[1], "del") == 0) {
    if (argc < 3) {
      Serial.println(F("Usage: wifi del <index>"));
      return;
    }
    uint8_t idx = atoi(argv[2]);
    if (store.remove(idx)) {
      Serial.printf("Deleted index %u\n", idx);
    } else {
      Serial.println(F("Invalid index"));
    }
  } else if (strcmp(argv[1], "clear") == 0) {
    if (store.clear()) {
      Serial.println(F("All networks cleared"));
    } else {
      Serial.println(F("Failed to clear"));
    }
  } else {
    Serial.printf("Unknown wifi subcommand: %s\n", argv[1]);
    printHelp();
  }
}

void Console::wifiList() {
  wifi_store::WiFiStore store;
  store.begin();
  
  wifi_store::Network nets[wifi_store::MAX_NETWORKS];
  uint8_t count = store.load(nets, wifi_store::MAX_NETWORKS);
  if (count == 0) {
    Serial.println(F("No saved networks"));
    return;
  }
  Serial.println(F("Saved networks:"));
  for (uint8_t i = 0; i < count; i++) {
    Serial.printf("  %u: %s (prio %ld)\n", i, nets[i].ssid, nets[i].priority);
  }
}

void Console::printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  help                          this list"));
  Serial.println(F("  status                        board id, sensors, LED, WiFi, longest loop pass"));
  Serial.println(F("  wifi list                     saved networks"));
  Serial.println(F("  wifi add \"<ssid>\" <password>  save a network (quotes if the name has spaces)"));
  Serial.println(F("  wifi add \"<ssid>\"             an open network, no password"));
  Serial.println(F("  wifi del <n>                  delete network number n (see wifi list)"));
  Serial.println(F("  wifi clear                    delete all networks"));
  Serial.println(F("  reboot                        restart the board (new networks are used then)"));
}

uint8_t Console::split(char *line, char *argv[], uint8_t max) {
  uint8_t argc = 0;
  char *p = line;
  while (*p && argc < max) {
    while (*p == ' ' || *p == '\t') p++;
    if (!*p) break;
    if (*p == '"') {
      p++;
      argv[argc++] = p;
      while (*p && *p != '"') p++;
      if (*p) *p++ = '\0';
    } else {
      argv[argc++] = p;
      while (*p && *p != ' ' && *p != '\t') p++;
      if (*p) *p++ = '\0';
    }
  }
  return argc;
}