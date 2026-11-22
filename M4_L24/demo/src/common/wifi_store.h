// A short list of WiFi networks, saved in NVS (Preferences).
//
// NVS is a small key-value store in a flash partition. It survives power
// off AND re-flashing (an upload does not erase it; only "Erase Flash" does).
// It is not encrypted: whoever has the board can read the passwords out.
//
// Layout in the "wifi" namespace:  n = count,  s0..s4 = SSID,  p0..p4 = password
//
// Copy wifi_store.h / .cpp into your project as they are.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace wifi_store {

constexpr uint8_t MAX_NETWORKS = 5U;
constexpr size_t  SSID_MAX     = 31U;   // WiFiMulti::addAP() refuses longer
constexpr size_t  PASS_MAX     = 63U;   // WPA2 passphrase limit
constexpr size_t  PASS_MIN     = 8U;    // WPA2; 0 means an open network

struct Network {
  char ssid[SSID_MAX + 1U];
  char pass[PASS_MAX + 1U];
};

enum class Result : uint8_t { ADDED, REPLACED, FULL, BAD_SSID, BAD_PASS, NVS_ERROR };

// Fills out[0..max-1], returns how many networks are saved.
uint8_t load(Network *out, uint8_t max);

// Adds a network, or replaces the password if the SSID is already saved.
Result add(const char *ssid, const char *pass);

// index is 0-based. Returns false if there is no such network.
bool remove(uint8_t index);

void clear();

const char *resultText(Result r);

}  // namespace wifi_store
