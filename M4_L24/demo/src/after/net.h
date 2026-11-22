// WiFi and the Telegram bot, in their own FreeRTOS task.
//
// Both have to wait: WiFiMulti scans and connects for seconds, every
// getUpdates() is an HTTPS round trip of 0.3-2 s. Neither can be cut into
// small pieces from outside the library, so they get a task of their own.
// While this task waits for the network, loop() keeps running.
#pragma once

namespace net {

// From setup(), after shared::begin(). Loads the saved networks from NVS
// and starts the task. Networks added later are used after a reboot.
void begin();

}  // namespace net
