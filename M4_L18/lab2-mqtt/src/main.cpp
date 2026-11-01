// Lesson 18 - Lab 2: AHT10 + WiFi + a local web server + MQTT
//
// Serial: device id, WiFi scan, connection events with the reason,
// then one status line every 2 s.
// Browser: http://<ip>/ or http://esp-xxxx.local/  (page with LED buttons)
//          http://<ip>/status                        (JSON)
// curl:    curl -X POST -H 'Content-Type: application/json' \
//               -d '{"on":true}' http://<ip>/led
// MQTT:    publishes <prefix>/<id>/status every 5 s,
//          <prefix>/<id>/led with payload 1 or 0 switches the LED

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <esp_mac.h>
#include <Adafruit_AHTX0.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "board.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy include/secrets.example.h to include/secrets.h and fill it in"
#endif

// ---------------------------------------------------------------- timing
constexpr uint32_t SENSOR_PERIOD_MS = 2000;
constexpr uint32_t AHT_RETRY_MS     = 5000;
constexpr uint32_t PRINT_PERIOD_MS  = 2000;
constexpr uint32_t MQTT_RETRY_MS    = 5000;   // connect() blocks up to a few seconds
constexpr uint32_t MQTT_PERIOD_MS   = 5000;
constexpr uint8_t  AHT10_ADDRESS    = 0x38;

// ---------------------------------------------------------------- state
static char deviceId[16];               // "esp-xxxx" from the last two MAC bytes
static char mqttClientId[32];           // "gamma-ed-esp-xxxx": unique on a public broker
static bool ledOn = false;

struct Reading {
  bool  valid;
  float t;
  float rh;
};
static Reading reading = {false, NAN, NAN};

static Adafruit_AHTX0 aht;
static bool     ahtPresent = false;
static uint32_t lastAhtTry = 0;
static uint32_t lastSample = 0;

static WebServer server(80);
static bool mdnsStarted = false;

static WiFiClient   net;
static PubSubClient mqtt(net);
static char     topicStatus[64];
static char     topicLed[64];
static uint32_t lastMqttTry = 0;
static uint32_t lastMqttPublish = 0;

static uint32_t lastPrint = 0;

// ---------------------------------------------------------------- helpers
static bool mqttEnabled() {
  return !USE_SOFTAP && MQTT_HOST[0] != '\0';
}

static void ledBegin() {
  if (HAS_RGB) {
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
  } else {
    pinMode(PIN_LED, OUTPUT);
  }
}

// "LED on" = green on the RGB LED (C3), the on-board LED on the S2.
static void ledSet(bool on) {
  ledOn = on;
  if (HAS_RGB) {
    // common anode: LOW = on
    digitalWrite(PIN_LED_R, HIGH);
    digitalWrite(PIN_LED_G, on ? LOW : HIGH);
    digitalWrite(PIN_LED_B, HIGH);
  } else {
    digitalWrite(PIN_LED, (on != LED_ACTIVE_LOW) ? HIGH : LOW);
  }
}

static void makeDeviceId() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);   // works before WiFi is started
  snprintf(deviceId, sizeof(deviceId), "esp-%02x%02x", mac[4], mac[5]);
  Serial.printf("Board %s  MAC %02X:%02X:%02X:%02X:%02X:%02X  id %s\n",
                BOARD_NAME, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], deviceId);
}

static const char *wifiStatusText(wl_status_t s) {
  switch (s) {
    case WL_IDLE_STATUS:     return "idle";
    case WL_NO_SSID_AVAIL:   return "no SSID";
    case WL_CONNECTED:       return "connected";
    case WL_CONNECT_FAILED:  return "failed";
    case WL_CONNECTION_LOST: return "lost";
    case WL_DISCONNECTED:    return "disconnected";
    default:                 return "?";
  }
}

static const char *disconnectReasonText(uint8_t r) {
  switch (r) {
    case 2:   return "auth expired - weak signal?";
    case 15:  return "4-way handshake timeout - wrong password?";
    case 201: return "AP not found - wrong SSID, 5 GHz only, or out of range";
    case 202: return "auth failed - wrong password?";
    case 203: return "association failed";
    case 204: return "handshake timeout - wrong password?";
    default:  return "see wifi_err_reason_t";
  }
}

static const char *mqttStateText(int s) {
  switch (s) {
    case -4: return "connection timeout";
    case -3: return "connection lost";
    case -2: return "connect failed - no TCP connection to the broker";
    case -1: return "disconnected";
    case 0:  return "connected";
    case 1:  return "bad protocol";
    case 2:  return "client id rejected";
    case 3:  return "broker unavailable";
    case 4:  return "bad user name or password";
    case 5:  return "not authorized";
    default: return "?";
  }
}

// The same JSON for /status and for MQTT.
static size_t buildStatusJson(char *out, size_t size) {
  JsonDocument doc;
  doc["id"]        = deviceId;
  doc["board"]     = BOARD_NAME;
  doc["uptime_ms"] = millis();
  doc["led"]       = ledOn;
  if (reading.valid) {
    doc["t"]  = serialized(String(reading.t, 1));
    doc["rh"] = serialized(String(reading.rh, 1));
  } else {
    doc["t"]  = nullptr;
    doc["rh"] = nullptr;
  }
  if (!USE_SOFTAP) {
    doc["rssi"] = WiFi.RSSI();
  }
  return serializeJson(doc, out, size);
}

// ---------------------------------------------------------------- WiFi
// Runs in the Arduino event task, not in an ISR: Serial is allowed here,
// but keep it short.
static void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  static uint8_t lastReason = 0;
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("WiFi: associated, waiting for an IP address");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      lastReason = 0;
      Serial.printf("WiFi: got IP %s -> http://%s/\n",
                    WiFi.localIP().toString().c_str(),
                    WiFi.localIP().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: {
      uint8_t r = info.wifi_sta_disconnected.reason;
      if (r != lastReason) {               // print only when the reason changes
        lastReason = r;
        Serial.printf("WiFi: disconnected, reason %u (%s)\n", r, disconnectReasonText(r));
      }
      break;
    }
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("AP: a client joined");
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("AP: a client left");
      break;
    default:
      break;
  }
}

static void wifiScan() {
  Serial.println("Scanning...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.printf("No networks found (n = %d)\n", n);
  }
  for (int i = 0; i < n; i++) {
    Serial.printf("%2d  %-32s %4d dBm  ch %2d\n", i, WiFi.SSID(i).c_str(),
                  WiFi.RSSI(i), WiFi.channel(i));
  }
  WiFi.scanDelete();
}

static void wifiBegin() {
  WiFi.setHostname(deviceId);   // core 2.x: must come before WiFi.mode()
  WiFi.onEvent(onWifiEvent);

  if (USE_SOFTAP) {
    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(deviceId, AP_PASS)) {
      Serial.printf("AP: join \"%s\" (password from secrets.h), then open http://%s/\n",
                    deviceId, WiFi.softAPIP().toString().c_str());
    } else {
      Serial.println("AP: start failed - is AP_PASS at least 8 characters?");
    }
    return;
  }

  WiFi.mode(WIFI_STA);
  if (LIMIT_TX_POWER) {
    WiFi.setTxPower(TX_POWER);
  }
  wifiScan();                        // blocking 2-3 s, fine in setup()
  Serial.printf("WiFi: connecting to \"%s\"\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  // the driver retries by itself
}

static void mdnsService() {
  if (mdnsStarted) return;
  bool netUp = USE_SOFTAP || WiFi.isConnected();
  if (!netUp) return;
  mdnsStarted = true;                // one attempt only
  if (MDNS.begin(deviceId)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("mDNS: http://%s.local/\n", deviceId);
  } else {
    Serial.println("mDNS: start failed");
  }
}

// ---------------------------------------------------------------- AHT10
static bool ahtResponds() {
  // The library waits in a loop while the busy bit is set, and a missing
  // sensor reads as 0xFF (busy set) - the hang seen in session 15.
  // So check the address first.
  Wire.beginTransmission(AHT10_ADDRESS);
  return Wire.endTransmission() == 0;
}

static void ahtService(uint32_t now) {
  if (!ahtPresent) {
    if (now - lastAhtTry < AHT_RETRY_MS) return;
    lastAhtTry = now;
    if (ahtResponds() && aht.begin(&Wire)) {
      ahtPresent = true;
      Serial.println("AHT10: found");
    }
    return;
  }

  if (now - lastSample < SENSOR_PERIOD_MS) return;
  lastSample = now;

  sensors_event_t h, t;
  if (ahtResponds() && aht.getEvent(&h, &t)) {
    reading = {true, t.temperature, h.relative_humidity};
  } else {
    ahtPresent = false;
    lastAhtTry = now;
    reading = {false, NAN, NAN};
    Serial.println("AHT10: missing");
  }
}

// ---------------------------------------------------------------- web
static const char INDEX_HTML[] = R"html(<!DOCTYPE html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Lesson 18</title>
<style>body{font-family:sans-serif;margin:2em}b{font-size:1.4em}
button{font-size:1.2em;padding:.4em 1.2em;margin-right:.5em}</style>
</head><body>
<h1 id="id">...</h1>
<p>Temperature: <b id="t">-</b> C</p>
<p>Humidity: <b id="rh">-</b> %</p>
<p>LED: <b id="led">-</b></p>
<button onclick="setLed(true)">LED on</button>
<button onclick="setLed(false)">LED off</button>
<p><a href="/status">/status</a></p>
<script>
function show(s) {
  document.getElementById('id').textContent = s.id;
  document.getElementById('t').textContent = (s.t === null) ? '-' : s.t;
  document.getElementById('rh').textContent = (s.rh === null) ? '-' : s.rh;
  document.getElementById('led').textContent = s.led ? 'on' : 'off';
}
function poll() {
  fetch('/status').then(r => r.json()).then(show).catch(() => {});
}
function setLed(on) {
  fetch('/led', {method: 'POST', headers: {'Content-Type': 'application/json'},
                 body: JSON.stringify({on: on})})
    .then(r => r.json()).then(show);
}
poll();
setInterval(poll, 2000);
</script>
</body></html>
)html";

static void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

static void handleStatus() {
  char out[256];
  buildStatusJson(out, sizeof(out));
  server.send(200, "application/json", out);
}

// POST /led
//   form field:  on=1 / on=0
//   JSON body:   {"on":true} / {"on":false}
static void handleLed() {
  if (server.hasArg("on")) {
    ledSet(server.arg("on") == "1");
  } else if (server.hasArg("plain")) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err || !doc["on"].is<bool>()) {
      server.send(400, "text/plain", "expected {\"on\":true} or {\"on\":false}\n");
      return;
    }
    ledSet(doc["on"].as<bool>());
  } else {
    server.send(400, "text/plain", "missing \"on\"\n");
    return;
  }
  handleStatus();
}

static void handleNotFound() {
  server.send(404, "text/plain", "not found\n");
}

static void webBegin() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/led", HTTP_POST, handleLed);
  // Browsers ask for an icon by themselves; answer "no content" so the
  // log is not filled with "request handler not found".
  server.on("/favicon.ico", HTTP_GET, [] { server.send(204); });
  server.onNotFound(handleNotFound);
  server.begin();
}

// ---------------------------------------------------------------- MQTT
static void onMqttMessage(char *topic, byte *payload, unsigned int len) {
  if (len == 0) return;
  if (payload[0] == '1') {
    ledSet(true);
  } else if (payload[0] == '0') {
    ledSet(false);
  } else {
    Serial.printf("MQTT: %s - expected 1 or 0\n", topic);
    return;
  }
  Serial.printf("MQTT: %s -> LED %s\n", topic, ledOn ? "on" : "off");
}

static void mqttBegin() {
  if (!mqttEnabled()) return;
  snprintf(topicStatus, sizeof(topicStatus), "%s/%s/status", MQTT_PREFIX, deviceId);
  snprintf(topicLed, sizeof(topicLed), "%s/%s/led", MQTT_PREFIX, deviceId);
  snprintf(mqttClientId, sizeof(mqttClientId), "gamma-ed-%s", deviceId);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
  mqtt.setBufferSize(512);
  Serial.printf("MQTT: %s:%u, publish %s, subscribe %s\n",
                MQTT_HOST, MQTT_PORT, topicStatus, topicLed);
}

static void mqttService(uint32_t now) {
  if (!mqttEnabled() || !WiFi.isConnected()) return;

  if (!mqtt.connected()) {
    if (now - lastMqttTry < MQTT_RETRY_MS) return;
    lastMqttTry = now;
    // The client id must be unique on the broker - on a public broker,
    // unique worldwide: two clients with the same id keep kicking each
    // other off.
    bool ok = (MQTT_USER[0] != '\0')
                  ? mqtt.connect(mqttClientId, MQTT_USER, MQTT_PASS)
                  : mqtt.connect(mqttClientId);
    if (ok) {
      mqtt.subscribe(topicLed);
      Serial.println("MQTT: connected");
    } else {
      Serial.printf("MQTT: state %d (%s)\n", mqtt.state(), mqttStateText(mqtt.state()));
      return;
    }
  }

  mqtt.loop();

  if (now - lastMqttPublish >= MQTT_PERIOD_MS) {
    lastMqttPublish = now;
    char out[256];
    buildStatusJson(out, sizeof(out));
    mqtt.publish(topicStatus, out);
  }
}

// ---------------------------------------------------------------- print
static void printStatus() {
  char t[24];
  if (reading.valid) {
    snprintf(t, sizeof(t), "%.1f C  %.1f %%", reading.t, reading.rh);
  } else if (ahtPresent) {
    snprintf(t, sizeof(t), "first reading...");
  } else {
    snprintf(t, sizeof(t), "sensor missing");
  }

  if (USE_SOFTAP) {
    Serial.printf("%s | AP %s, %d client(s) | %s | LED %s\n", deviceId,
                  WiFi.softAPIP().toString().c_str(), WiFi.softAPgetStationNum(),
                  t, ledOn ? "on" : "off");
    return;
  }

  wl_status_t s = WiFi.status();
  char wifi[48];
  if (s == WL_CONNECTED) {
    snprintf(wifi, sizeof(wifi), "%s %d dBm", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    snprintf(wifi, sizeof(wifi), "%s", wifiStatusText(s));
  }
  Serial.printf("%s | WiFi %s | %s | LED %s | MQTT %s\n", deviceId, wifi, t,
                ledOn ? "on" : "off",
                !mqttEnabled() ? "off" : (mqtt.connected() ? "ok" : "-"));
}

// ---------------------------------------------------------------- main
void setup() {
  Serial.begin(115200);
  delay(2000);                        // USB CDC needs a moment after reset
  Serial.println();
  Serial.printf("Lesson 18 - Lab 2 - Arduino core %d.%d.%d, IDF %s\n",
                ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR,
                ESP_ARDUINO_VERSION_PATCH, esp_get_idf_version());

  ledBegin();
  ledSet(false);
  makeDeviceId();

  Wire.begin(PIN_SDA, PIN_SCL);
  lastAhtTry = millis() - AHT_RETRY_MS;   // try the sensor right away

  wifiBegin();
  webBegin();
  mqttBegin();
}

void loop() {
  uint32_t now = millis();

  server.handleClient();
  mdnsService();
  ahtService(now);
  mqttService(now);

  if (now - lastPrint >= PRINT_PERIOD_MS) {
    lastPrint = now;
    printStatus();
  }
}
