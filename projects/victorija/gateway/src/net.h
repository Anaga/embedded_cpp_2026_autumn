// WiFi and the Telegram bot: the parts that MUST wait.
//
// One HTTPS request to Telegram takes 0.3-2 s, and joining WiFi takes
// seconds. That can't be cut into small pieces, so it runs in its own
// FreeRTOS task on core 0, next to the WiFi driver. loop() keeps core 1
// and never waits for the network.
//
// The task reads the alarms and the history; it never touches the radio
// or the LED. It sends alerts that loop() queued (shared.h).
#pragma once
#include "alarms.h"
#include "history.h"

namespace net {

void begin(const Alarms& alarms, History& history);  // from setup(): loads the WiFi list, starts the task
void printStatus();                                   // for the console's "status" command

}  // namespace net
