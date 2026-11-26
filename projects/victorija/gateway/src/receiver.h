// The LoRa receiver: one GuardianMsg packet at a time.
//
// For every packet: check it's really ours (size, magic, version), count
// lost packets from gaps in seq, print it, publish it for the bot, then
// hand it to the alarms, the history and the LED.
//
// update(now) only asks the radio "anything there?" and returns at once.
#pragma once
#include <Arduino.h>

#include "alarms.h"
#include "history.h"
#include "shared.h"
#include "status_led.h"

class Receiver {
 public:
  Receiver(Alarms& alarms, History& history, StatusLed& led) : alarms_(alarms), history_(history), led_(led) {}
  bool begin();  // false if the Ra-01 doesn't answer
  void update(uint32_t now);
  void printStatus() const;  // for the console's "status" command

 private:
  void handle(int packetSize, uint32_t now);

  Alarms& alarms_;
  History& history_;
  StatusLed& led_;
  shared::Latest current_;  // loop()'s own copy; the bot gets a copy of it
  bool haveSeq_ = false;
  uint16_t lastSeq_ = 0U;
};

const char* wakeName(uint8_t wakeCause);
void copyName(const GuardianMsg& m, char* out);  // out: sizeof(m.name) + 1 bytes, always ends in 0
