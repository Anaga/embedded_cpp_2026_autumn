// Lesson 19 - Lab 3: the class setup. Both values are on the whiteboard.
//
// CLASS_SSID  - the router the gateway is joined to. The node never joins
//               it: it only scans for it to learn the WiFi channel.
//               No password is needed.
// GATEWAY_MAC - printed by the gateway at start, already in this form.
#pragma once
#include <stdint.h>

constexpr const char *CLASS_SSID = "Gamma Intelligence Training";
constexpr uint8_t GATEWAY_MAC[6] = {0xCC, 0x8D, 0xA2, 0x8C, 0x88, 0x78};
// 0xCC, 0x8D, 0xA2, 0x8C, 0x88, 0x78