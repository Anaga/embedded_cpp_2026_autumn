#pragma once
#include <Arduino.h>

enum esp_a2d_connection_state_t {
    ESP_A2D_CONNECTION_STATE_DISCONNECTED,
    ESP_A2D_CONNECTION_STATE_CONNECTING,
    ESP_A2D_CONNECTION_STATE_CONNECTED,
    ESP_A2D_CONNECTION_STATE_DISCONNECTING,
};
enum esp_a2d_audio_state_t {
    ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND,
    ESP_A2D_AUDIO_STATE_STOPPED,
    ESP_A2D_AUDIO_STATE_STARTED,
};
enum esp_bt_gap_discovery_state_t { ESP_BT_GAP_DISCOVERY_STOPPED, ESP_BT_GAP_DISCOVERY_STARTED };
using esp_bd_addr_t = uint8_t[6];
enum esp_bt_pin_type_t { ESP_BT_PIN_TYPE_VARIABLE, ESP_BT_PIN_TYPE_FIXED };
struct Frame { int16_t channel1; int16_t channel2; };

class BluetoothA2DPSource {
public:
    unsigned starts = 0;
    bool ssp = false;
    bool reconnectEnabled = false;
    int retries = 0;
    std::string localName;
    std::string pin;
    esp_bt_pin_type_t pinType = ESP_BT_PIN_TYPE_FIXED;
    int32_t (*render)(Frame*, int32_t) = nullptr;
    void (*connection)(esp_a2d_connection_state_t, void*) = nullptr;
    void (*audio)(esp_a2d_audio_state_t, void*) = nullptr;
    void (*discovery)(esp_bt_gap_discovery_state_t) = nullptr;
    bool (*select)(const char*, esp_bd_addr_t, int) = nullptr;
    void (*beforeStart)() = nullptr;
    void set_auto_reconnect(bool enabled, int count = 0) { reconnectEnabled = enabled; retries = count; }
    void set_local_name(const char* text) { localName = text; }
    void set_data_callback_in_frames(int32_t (*cb)(Frame*, int32_t)) { render = cb; }
    void set_on_connection_state_changed(void (*cb)(esp_a2d_connection_state_t, void*)) { connection = cb; }
    void set_on_audio_state_changed(void (*cb)(esp_a2d_audio_state_t, void*)) { audio = cb; }
    void set_discovery_mode_callback(void (*cb)(esp_bt_gap_discovery_state_t)) { discovery = cb; }
    void set_ssid_callback(bool (*cb)(const char*, esp_bd_addr_t, int)) { select = cb; }
    void set_ssp_enabled(bool enabled) { ssp = enabled; }
    void set_pin_code(const char* text, esp_bt_pin_type_t type) { pin = text; pinType = type; }
    void start() {
        if (beforeStart != nullptr) beforeStart();
        ++starts;
    }
    bool reconnectAllowed() const { return is_autoreconnect_allowed; }
protected:
    bool is_autoreconnect_allowed = false;
};
