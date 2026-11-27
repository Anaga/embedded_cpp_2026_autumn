#ifndef WIFI_STORE_H
#define WIFI_STORE_H

#include <stdint.h>
#include <Preferences.h>

namespace wifi_store {

constexpr uint8_t MAX_NETWORKS = 8;
constexpr size_t MAX_SSID_LENGTH = 32;
constexpr size_t MAX_PASS_LENGTH = 64;

struct Network {
    char ssid[MAX_SSID_LENGTH];
    char pass[MAX_PASS_LENGTH];
    int32_t priority;  // higher = preferred
};

class WiFiStore {
public:
    WiFiStore() : prefs_(), initialized_(false), network_count_(0) {}

    // Initialize NVS namespace
    bool begin(const char* nvs_namespace = "wifi_cfg");

    // Load all networks into array, returns count
    uint8_t load(Network* out, uint8_t max_count);

    // Add or update a network
    bool save(const char* ssid, const char* pass = "", int32_t priority = 0);

    // Delete network by index (0-based in stored order)
    bool remove(uint8_t index);

    // Clear all networks
    bool clear();

    // Get count of stored networks
    uint8_t count() const { return network_count_; }

    // List networks (for console output)
    void list(Network* out, uint8_t max_count) const;

private:
    Preferences prefs_;
    bool initialized_;
    uint8_t network_count_ = 0;

    // Internal key format: "net_0", "net_1", etc.
    static constexpr const char* NS = "wifi_cfg";
    static constexpr const char* COUNT_KEY = "count";
};

}  // namespace wifi_store

#endif // WIFI_STORE_H