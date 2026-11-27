#ifndef NET_H
#define NET_H

// WiFi + Web Server + Telegram Bot + BLE in FreeRTOS task
// - WiFiMulti with NVS storage (wifi add, wifi list, wifi del, reboot)
// - Web server for web UI control
// - Telegram bot for remote control
// - BLE (Nordic UART) for mobile app control

namespace net {

// Initialize from setup(), after shared::begin()
// Loads saved networks from NVS and starts the task
void begin();

// For console commands (called from loop task)
bool handleConsoleCommand(const char* line);

}  // namespace net

#endif // NET_H