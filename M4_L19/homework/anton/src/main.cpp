/*
 * Homework 19 - Built-in web server on ESP32 for remote control.
 *
 * The board runs an AP. A phone browser opens http://192.168.4.1 and shows
 * a page with buttons to control two LEDs and read a "sensor" value.
 *
 * Non-blocking: WebServer handles each request in a callback; loop() calls
 * server.handleClient() and nothing blocks. No delay() in loop().
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <stdint.h>

// ==================== Configuration ====================
static const char* AP_SSID = "AntiMurzik_Web";
static const char* AP_PASS = "***";
static const uint16_t WEB_PORT = 80;

static const uint8_t PIN_LED1 = 2;
static const uint8_t PIN_LED2 = 4;

// ==================== Globals ====================
static WebServer server(WEB_PORT);
static bool led1_on = false;
static bool led2_on = false;

// ==================== Handlers ====================
static void handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<title>Anti-Murzik</title>"
        "<style>body{font-family:sans-serif;background:#1a1a2e;color:#e0e0e0;"
        "text-align:center;padding:20px}"
        ".btn{font-size:24px;padding:20px 30px;margin:10px;border:none;"
        "border-radius:12px;cursor:pointer}"
        ".on{background:#4caf50;color:#fff}.off{background:#f44336;color:#fff}"
        ".status{background:#16213e;padding:20px;border-radius:10px;margin-top:20px}"
        "</style></head><body>"
        "<h1>Anti-Murzik Control</h1>"
        "<p>LED1: <a href=\"/led1/on\"><button class=\"btn on\">ON</button></a> "
        "<a href=\"/led1/off\"><button class=\"btn off\">OFF</button></a></p>"
        "<p>LED2: <a href=\"/led2/on\"><button class=\"btn on\">ON</button></a> "
        "<a href=\"/led2/off\"><button class=\"btn off\">OFF</button></a></p>"
        "<p><a href=\"/sensor\"><button class=\"btn on\">READ SENSOR</button></a></p>"
        "<div class=\"status\">"
        "<p>LED1: " + String(led1_on ? "ON" : "OFF") + "</p>"
        "<p>LED2: " + String(led2_on ? "ON" : "OFF") + "</p>"
        "<p>Uptime: " + String(millis() / 1000) + " s</p>"
        "</div></body></html>";
    server.send(200, "text/html", html);
}

static void handleLed1On()  { led1_on = true;  digitalWrite(PIN_LED1, HIGH); server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); }
static void handleLed1Off() { led1_on = false; digitalWrite(PIN_LED1, LOW);  server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); }
static void handleLed2On()  { led2_on = true;  digitalWrite(PIN_LED2, HIGH); server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); }
static void handleLed2Off() { led2_on = false; digitalWrite(PIN_LED2, LOW);  server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); }

static void handleSensor() {
    // Simulated sensor: replace with real AHT10 read in the real project
    float t = 22.5f;
    float h = 45.0f;
    String json = "{\"t\":" + String(t, 1) + ",\"rh\":" + String(h, 1) + "}";
    server.send(200, "application/json", json);
}

static void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

// ==================== Setup ====================
void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    digitalWrite(PIN_LED1, LOW);
    digitalWrite(PIN_LED2, LOW);

    // AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // Routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/led1/on", HTTP_GET, handleLed1On);
    server.on("/led1/off", HTTP_GET, handleLed1Off);
    server.on("/led2/on", HTTP_GET, handleLed2On);
    server.on("/led2/off", HTTP_GET, handleLed2Off);
    server.on("/sensor", HTTP_GET, handleSensor);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Web server started");
}

// ==================== Loop (non-blocking) ====================
void loop() {
    server.handleClient();
}
