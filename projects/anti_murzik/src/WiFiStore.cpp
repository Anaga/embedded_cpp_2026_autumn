#include "WiFiStore.h"
#include <Arduino.h>

namespace wifi_store {

bool WiFiStore::begin(const char* nvs_namespace) {
    if (initialized_) return true;
    if (!prefs_.begin(nvs_namespace, false)) {
        Serial.println("[WiFiStore] Failed to open NVS namespace");
        return false;
    }
    initialized_ = true;
    network_count_ = prefs_.getUInt(COUNT_KEY, 0);
    if (network_count_ > MAX_NETWORKS) network_count_ = MAX_NETWORKS;
    return true;
}

uint8_t WiFiStore::load(Network* out, uint8_t max_count) {
    if (!initialized_) return 0;
    uint8_t count = 0;
    for (uint8_t i = 0; i < network_count_ && count < max_count; i++) {
        char key[16];
        snprintf(key, sizeof(key), "net_%u", i);
        String data = prefs_.getString(key, "");
        if (data.length() == 0) continue;

        // Format: ssid|pass|priority
        int sep1 = data.indexOf('|');
        int sep2 = data.indexOf('|', sep1 + 1);
        if (sep1 < 0 || sep2 < 0) continue;

        String ssid = data.substring(0, sep1);
        String pass = data.substring(sep1 + 1, sep2);
        String prio = data.substring(sep2 + 1);

        if (ssid.length() >= MAX_SSID_LENGTH) continue;

        strncpy(out[count].ssid, ssid.c_str(), MAX_SSID_LENGTH - 1);
        out[count].ssid[MAX_SSID_LENGTH - 1] = '\0';

        if (pass.length() >= MAX_PASS_LENGTH) pass = pass.substring(0, MAX_PASS_LENGTH - 1);
        strncpy(out[count].pass, pass.c_str(), MAX_PASS_LENGTH - 1);
        out[count].pass[MAX_PASS_LENGTH - 1] = '\0';

        out[count].priority = prio.toInt();
        count++;
    }
    return count;
}

bool WiFiStore::save(const char* ssid, const char* pass, int32_t priority) {
    if (!initialized_) return false;
    if (!ssid || strlen(ssid) == 0 || strlen(ssid) >= MAX_SSID_LENGTH) return false;
    if (pass && strlen(pass) >= MAX_PASS_LENGTH) return false;

    // Check if network already exists
    Network nets[MAX_NETWORKS];
    uint8_t count = load(nets, MAX_NETWORKS);
    int existing_idx = -1;
    for (uint8_t i = 0; i < count; i++) {
        if (strcmp(nets[i].ssid, ssid) == 0) {
            existing_idx = i;
            break;
        }
    }

    // If exists, update; else add at end (or replace lowest priority if full)
    uint8_t target_idx;
    if (existing_idx >= 0) {
        target_idx = existing_idx;
    } else if (count < MAX_NETWORKS) {
        target_idx = count;
        network_count_ = count + 1;
    } else {
        // Replace lowest priority
        int min_prio = INT32_MAX;
        target_idx = 0;
        for (uint8_t i = 0; i < MAX_NETWORKS; i++) {
            if (nets[i].priority < min_prio) {
                min_prio = nets[i].priority;
                target_idx = i;
            }
        }
    }

    char key[16];
    snprintf(key, sizeof(key), "net_%u", target_idx);

    String data = String(ssid) + "|" + String(pass ? pass : "") + "|" + String(priority);
    bool ok = prefs_.putString(key, data);

    if (ok && existing_idx < 0) {
        prefs_.putUInt(COUNT_KEY, network_count_);
    }
    return ok;
}

bool WiFiStore::remove(uint8_t index) {
    if (!initialized_ || index >= network_count_) return false;

    // Shift all higher indices down
    for (uint8_t i = index; i < network_count_ - 1; i++) {
        char key_src[16], key_dst[16];
        snprintf(key_src, sizeof(key_src), "net_%u", i + 1);
        snprintf(key_dst, sizeof(key_dst), "net_%u", i);
        String data = prefs_.getString(key_src, "");
        if (data.length() > 0) {
            prefs_.putString(key_dst, data);
        } else {
            prefs_.remove(key_dst);
        }
    }

    // Remove last
    char key_last[16];
    snprintf(key_last, sizeof(key_last), "net_%u", network_count_ - 1);
    prefs_.remove(key_last);

    network_count_--;
    prefs_.putUInt(COUNT_KEY, network_count_);
    return true;
}

bool WiFiStore::clear() {
    if (!initialized_) return false;
    for (uint8_t i = 0; i < network_count_; i++) {
        char key[16];
        snprintf(key, sizeof(key), "net_%u", i);
        prefs_.remove(key);
    }
    network_count_ = 0;
    prefs_.putUInt(COUNT_KEY, 0);
    return true;
}

void WiFiStore::list(Network* out, uint8_t max_count) const {
    // This is const but we need to load from prefs - load is non-const
    // For simplicity, we'll just return the count; actual loading is done via load()
    (void)out;
    (void)max_count;
}

}  // namespace wifi_store