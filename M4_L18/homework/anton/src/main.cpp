/*
 * Homework 18 - Wireless communication: Wi-Fi access point on ESP32.
 *
 * The board becomes an AP. A phone can join it and reach the board at a
 * fixed IP. The board answers ping and prints the station list.
 *
 * Non-blocking: everything uses millis(); no delay() in loop().
 */

#include <Arduino.h>
#include <WiFi.h>
#include <stdint.h>

// ==================== Configuration ====================
static const char* AP_SSID = "AntiMurzik_AP";
static const char* AP_PASS = "***";       // WPA2, at least 8 chars
static const uint16_t AP_CHANNEL = 1;
static const uint8_t AP_MAX_STA = 4;
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_GW(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);

static const uint32_t PRINT_INTERVAL_MS = 5000;

// ==================== Setup ====================
void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("=== L18 Homework - Wi-Fi AP ===");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
    bool ok = WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, false, AP_MAX_STA);
    if (!ok) {
        Serial.println("AP start FAILED");
    } else {
        Serial.print("AP started: ");
        Serial.print(AP_SSID);
        Serial.print(" at ");
        Serial.println(WiFi.softAPIP());
    }
}

// ==================== Loop ====================
void loop() {
    static uint32_t last_print = 0;
    const uint32_t now = millis();

    if (now - last_print >= PRINT_INTERVAL_MS) {
        last_print = now;
        uint8_t n = WiFi.softAPgetStationNum();
        Serial.printf("[AP] stations=%u uptime=%lus\n",
                      n, (unsigned long)(now / 1000));
    }
}
