// Lesson 19 - Lab 3 gateway: ESP-NOW in, web page out
//
// Joins the class router (station), so it listens on the router's
// channel. Receives NodeMsg packets from the sleeping nodes over ESP-NOW,
// stamps each one with the local time (NTP) and shows every node on a
// web page. Runs on the instructor's board and never sleeps.
//
// At start it prints what goes on the whiteboard:
//   CLASS_SSID   the router name (from secrets.h)
//   GATEWAY_MAC  this board's STA MAC, already in C array form
//
// Serial:  one line per packet
// Browser: http://<ip>/        page, refreshes every 2 s
//          http://<ip>/nodes   the same data as JSON

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_now.h>
#include <time.h>
#include <ArduinoJson.h>

#include "msg.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy include/secrets.example.h to include/secrets.h and fill it in"
#endif

// ---------------------------------------------------------------- settings
constexpr const char *TZ_TALLINN   = "EET-2EEST,M3.5.0/3,M10.5.0/4";
constexpr const char *NTP_SERVER   = "pool.ntp.org";
constexpr uint32_t WIFI_TIMEOUT_MS = 20000;
constexpr uint32_t WIFI_POLL_MS    = 100;
constexpr uint32_t NTP_TIMEOUT_MS  = 10000;
constexpr size_t   MAX_NODES       = 10;
constexpr size_t   QUEUE_LEN       = 8;
constexpr int      MAC_LEN         = 6;
constexpr float    TENTHS          = 10.0f;
// Many Super Mini boards connect only with reduced TX power (session 18).
constexpr wifi_power_t TX_POWER = WIFI_POWER_8_5dBm;

// ---------------------------------------------------------------- queue
// The receive callback runs in the WiFi task. It only copies the packet
// into this small queue; loop() does the rest (printing, the table).
struct Packet {
  uint8_t mac[MAC_LEN];
  NodeMsg msg;
};

static Packet   queue[QUEUE_LEN];
static size_t   qHead = 0;            // next free slot
static size_t   qTail = 0;            // next packet to read
static uint32_t qDropped = 0;         // queue full
static uint32_t badPackets = 0;       // wrong length or version
static portMUX_TYPE qMux = portMUX_INITIALIZER_UNLOCKED;

// Core 2.x signature. Core 3.x examples online use a different one.
static void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != static_cast<int>(sizeof(NodeMsg)) || data[0] != MSG_VERSION) {
    portENTER_CRITICAL(&qMux);
    badPackets++;
    portEXIT_CRITICAL(&qMux);
    return;
  }
  portENTER_CRITICAL(&qMux);
  size_t next = (qHead + 1) % QUEUE_LEN;
  if (next == qTail) {
    qDropped++;
  } else {
    memcpy(queue[qHead].mac, mac, MAC_LEN);
    memcpy(&queue[qHead].msg, data, sizeof(NodeMsg));
    qHead = next;
  }
  portEXIT_CRITICAL(&qMux);
}

static bool queuePop(Packet &out) {
  bool got = false;
  portENTER_CRITICAL(&qMux);
  if (qTail != qHead) {
    out = queue[qTail];
    qTail = (qTail + 1) % QUEUE_LEN;
    got = true;
  }
  portEXIT_CRITICAL(&qMux);
  return got;
}

// ---------------------------------------------------------------- node table
struct Node {
  bool     used;
  uint8_t  mac[MAC_LEN];
  NodeMsg  last;
  time_t   lastSeen;      // local time of the last packet
  uint32_t packets;
  uint32_t missed;        // gaps in seq
};

static Node nodes[MAX_NODES];

static Node *findOrAddNode(const uint8_t *mac) {
  for (Node &n : nodes) {
    if (n.used && memcmp(n.mac, mac, MAC_LEN) == 0) return &n;
  }
  for (Node &n : nodes) {
    if (!n.used) {
      n = {};
      n.used = true;
      memcpy(n.mac, mac, MAC_LEN);
      return &n;
    }
  }
  return nullptr;                     // table full
}

static void formatTime(time_t t, char *out, size_t size) {
  struct tm lt;
  localtime_r(&t, &lt);
  strftime(out, size, "%H:%M:%S", &lt);
}

static void handlePacket(const Packet &p) {
  Node *n = findOrAddNode(p.mac);
  if (n == nullptr) {
    Serial.println("node table full - raise MAX_NODES");
    return;
  }
  if (n->packets > 0) {
    uint16_t expected = static_cast<uint16_t>(n->last.seq + 1U);
    if (p.msg.seq != expected) {
      n->missed += static_cast<uint16_t>(p.msg.seq - expected);   // wraps correctly
    }
  }
  n->last = p.msg;
  n->last.name[MSG_NAME_LEN - 1] = '\0';   // never trust a string from the air
  n->lastSeen = time(nullptr);
  n->packets++;

  char hhmmss[12];
  formatTime(n->lastSeen, hhmmss, sizeof(hhmmss));
  if (p.msg.flags & MSG_FLAG_SENSOR_OK) {
    Serial.printf("%s  %-9s %5.1f C %5.1f %%  seq %5u  awake %5u ms\n", hhmmss,
                  n->last.name, p.msg.t_c10 / TENTHS, p.msg.rh_10 / TENTHS,
                  p.msg.seq, p.msg.awake_ms);
  } else {
    Serial.printf("%s  %-9s sensor missing  seq %5u  awake %5u ms\n", hhmmss,
                  n->last.name, p.msg.seq, p.msg.awake_ms);
  }
}

// ---------------------------------------------------------------- web
static WebServer server(80);

static void handleNodes() {
  JsonDocument doc;
  doc["now"] = static_cast<long>(time(nullptr));
  doc["bad"] = badPackets;
  doc["dropped"] = qDropped;
  JsonArray arr = doc["nodes"].to<JsonArray>();
  for (const Node &n : nodes) {
    if (!n.used) continue;
    JsonObject o = arr.add<JsonObject>();
    char mac[18];
    snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
             n.mac[0], n.mac[1], n.mac[2], n.mac[3], n.mac[4], n.mac[5]);
    o["mac"] = mac;
    o["name"] = n.last.name;
    if (n.last.flags & MSG_FLAG_SENSOR_OK) {
      o["t"] = serialized(String(n.last.t_c10 / TENTHS, 1));
      o["rh"] = serialized(String(n.last.rh_10 / TENTHS, 1));
    } else {
      o["t"] = nullptr;
      o["rh"] = nullptr;
    }
    o["seq"] = n.last.seq;
    o["awake_ms"] = n.last.awake_ms;
    o["packets"] = n.packets;
    o["missed"] = n.missed;
    o["last_seen"] = static_cast<long>(n.lastSeen);
  }
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

static const char INDEX_HTML[] = R"html(<!DOCTYPE html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Lesson 19 gateway</title>
<style>body{font-family:sans-serif;margin:2em}
table{border-collapse:collapse}td,th{padding:.3em .8em;border-bottom:1px solid #ccc;text-align:right}
td:first-child,th:first-child{text-align:left}.old{color:#aaa}</style>
</head><body>
<h1>Lesson 19 - ESP-NOW gateway</h1>
<table><thead><tr><th>Node</th><th>Temp C</th><th>RH %</th><th>Last packet</th>
<th>Seconds ago</th><th>Awake ms</th><th>Packets</th><th>Missed</th></tr></thead>
<tbody id="rows"><tr><td colspan="8">waiting for the first packet...</td></tr></tbody></table>
<p id="info"></p>
<script>
function hms(t) { return new Date(t * 1000).toLocaleTimeString(); }
function show(d) {
  let rows = '';
  for (const n of d.nodes) {
    const ago = d.now - n.last_seen;
    rows += '<tr class="' + (ago > 60 ? 'old' : '') + '"><td>' + n.name + '</td><td>' +
      (n.t === null ? '-' : n.t) + '</td><td>' + (n.rh === null ? '-' : n.rh) + '</td><td>' +
      hms(n.last_seen) + '</td><td>' + ago + '</td><td>' + n.awake_ms + '</td><td>' +
      n.packets + '</td><td>' + n.missed + '</td></tr>';
  }
  if (rows) document.getElementById('rows').innerHTML = rows;
  document.getElementById('info').textContent =
    'bad packets: ' + d.bad + ', dropped: ' + d.dropped;
}
function poll() { fetch('/nodes').then(r => r.json()).then(show).catch(() => {}); }
poll();
setInterval(poll, 2000);
</script>
</body></html>
)html";

static void webBegin() {
  server.on("/", HTTP_GET, [] { server.send(200, "text/html", INDEX_HTML); });
  server.on("/nodes", HTTP_GET, handleNodes);
  server.on("/favicon.ico", HTTP_GET, [] { server.send(204); });
  server.onNotFound([] { server.send(404, "text/plain", "not found\n"); });
  server.begin();
}

// ---------------------------------------------------------------- WiFi
static void printWhiteboard() {
  uint8_t mac[MAC_LEN];
  WiFi.macAddress(mac);
  Serial.println("---- for the whiteboard ----");
  Serial.printf("CLASS_SSID  = \"%s\"\n", WIFI_SSID);
  Serial.printf("GATEWAY_MAC = {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("channel %d, page http://%s/\n", WiFi.channel(),
                WiFi.localIP().toString().c_str());
  Serial.println("----------------------------");
}

// ---------------------------------------------------------------- main
void setup() {
  Serial.begin(115200);
  delay(2000);                        // USB CDC needs a moment after reset
  Serial.println();
  Serial.println("Lesson 19 - Lab 3 gateway");

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(TX_POWER);
  // Modem sleep turns the radio off between beacons and the gateway
  // misses ESP-NOW packets. A gateway on USB power keeps it on.
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("WiFi: connecting to \"%s\"", WIFI_SSID);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_TIMEOUT_MS) {
    delay(WIFI_POLL_MS);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi: no connection - the nodes need the router's channel; check secrets.h");
  }

  configTzTime(TZ_TALLINN, NTP_SERVER);
  struct tm now;
  Serial.println(getLocalTime(&now, NTP_TIMEOUT_MS) ? "NTP: ok" : "NTP: no answer yet");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW: init failed");
  }
  esp_now_register_recv_cb(onRecv);

  webBegin();
  printWhiteboard();
}

void loop() {
  server.handleClient();
  Packet p;
  while (queuePop(p)) {
    handlePacket(p);
  }
}
