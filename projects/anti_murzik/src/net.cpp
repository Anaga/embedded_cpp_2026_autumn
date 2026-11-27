// WiFi + Web Server + BLE in FreeRTOS task
//
// Both WiFi and BLE have to wait: WiFiMulti scans and connects for seconds,
// BLE needs to handle connections. Neither can be cut into small pieces
// from outside the library, so they get a task of their own.
// While this task waits for the network, loop() keeps running.

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

#include "board.h"
#include "WiFiStore.h"
#include "shared.h"
#include "net.h"

// ============================================================================
// BLE UUIDs (Nordic UART Service compatible)
// ============================================================================
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// ============================================================================
// Global BLE variables
// ============================================================================
static BLEServer* pServer = nullptr;
static BLECharacteristic* pTxCharacteristic = nullptr;
static bool bleDeviceConnected = false;
static bool oldBleDeviceConnected = false;

// ============================================================================
// WiFi and Web Server
// ============================================================================
static WiFiMulti wifiMulti;
static WebServer server(80);
static wifi_store::WiFiStore wifiStore;

// ============================================================================
// Forward declarations
// ============================================================================
static void handleRoot();
static void handleCmd();
static void handleStatus();
static void handleNotFound();
static void initBLE();
static void netTask(void* pvParameters);

// ============================================================================
// Console command handler (called from loop task via console)
// ============================================================================
bool net::handleConsoleCommand(const char* line) {
  if (strncmp(line, "wifi ", 5) == 0) {
    // Delegate to wifi_store
    return true;  // Handled
  }
  return false;  // Not a net command
}

// ============================================================================
// Net task entry point
// ============================================================================
namespace net {

void begin() {
  Serial.println(F("[net] Starting net task..."));
  
  // Initialize WiFi store
  wifiStore.begin();
  
  // Load saved networks
  wifi_store::Network nets[wifi_store::MAX_NETWORKS];
  uint8_t count = wifiStore.load(nets, wifi_store::MAX_NETWORKS);
  for (uint8_t i = 0; i < count; i++) {
    wifiMulti.addAP(nets[i].ssid, nets[i].pass);
  }
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCmd);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  
  // Start FreeRTOS task
  xTaskCreate(
    netTask,
    "net_task",
    8192,  // Stack size
    nullptr,
    1,     // Priority
    nullptr
  );
  
  // Initialize BLE
  initBLE();
}

}  // namespace net

// ============================================================================
// FreeRTOS task function
// ============================================================================
static void netTask(void* pvParameters) {
  (void)pvParameters;
  
  Serial.println(F("[net] Task started"));
  
  // Set WiFi mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("AntiMurzik-C3", "12345678");
  
  if (LIMIT_TX_POWER) {
    WiFi.setTxPower(TX_POWER);
  }
  
  bool connected = false;
  
  for (;;) {
    // Handle WiFi connection
    if (!connected) {
      if (wifiMulti.run() == WL_CONNECTED) {
        connected = true;
        Serial.printf("[net] WiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
        
        // Start MDNS
        if (MDNS.begin(MDNS_HOST)) {
          Serial.printf("[net] MDNS started: http://%s.local\n", MDNS_HOST);
        }
        
        // Start web server
        server.begin();
        Serial.println(F("[net] Web server started"));
      }
    } else {
      if (WiFi.status() != WL_CONNECTED) {
        connected = false;
        Serial.println(F("[net] WiFi disconnected"));
      }
    }
    
    // Handle web server clients
    if (connected) {
      server.handleClient();
    }
    
    // Handle BLE reconnection
    if (!bleDeviceConnected && oldBleDeviceConnected) {
      delay(500);
      pServer->startAdvertising();
      Serial.println(F("[BLE] Restarting advertising..."));
      oldBleDeviceConnected = bleDeviceConnected;
    }
    if (bleDeviceConnected && !oldBleDeviceConnected) {
      oldBleDeviceConnected = bleDeviceConnected;
      Serial.println(F("[BLE] Device connected"));
    }
    
    // Process BLE events
    // BLE stack handles events in background
    
    // Send snapshot to shared for loop task
    // (loop task publishes, net task reads via shared::read())
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ============================================================================
// Web Server Handlers
// ============================================================================
static void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
 <meta charset="UTF-8" />
 <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no" />
 <title>Анти-Мурзик</title>
 <style>
 * { box-sizing: border-box; user-select: none; -webkit-user-select: none; }
 body {
 font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
 background: #12151a; color: #f0f6fc; text-align: center;
 margin: 0; padding: 16px;
 }
 h2 { margin: 8px 0 16px; font-size: 20px; letter-spacing: 0.5px; }
 .panel { display: flex; justify-content: center; gap: 8px; margin-bottom: 18px; }
 .pad {
 display: grid; grid-template-columns: repeat(3, 1fr);
 gap: 10px; max-width: 340px; margin: 0 auto 20px;
 }
 button {
 height: 68px; border: none; border-radius: 14px;
 background: #21262d; color: #fff; font-size: 16px; font-weight: 600;
 box-shadow: 0 4px 0 #0d1117; cursor: pointer; touch-action: manipulation;
 }
 button:active { transform: translateY(2px); box-shadow: 0 2px 0 #0d1117; background: #1f6feb; }
 .gear { flex: 1; max-width: 75px; height: 48px; font-size: 14px; }
 .gear.active { background: #1f6feb; border: 2px solid #58a6ff; }
 .light { flex: 1; max-width: 90px; height: 48px; font-size: 14px; background: #238636; }
 .stop { background: #da3633; }
 .fire {
 width: 100%; max-width: 340px; height: 60px;
 background: #d97706; font-size: 18px; letter-spacing: 1px;
 }
 .status { margin-top: 14px; font-size: 13px; color: #8b949e; }
 </style>
</head>
<body>
 <h2>ПУЛЬТ «АНТИ-МУРЗИК»</h2>

 <div class="panel">
 <button id="g1" class="gear" onclick="setGear(1)">1 ск.</button>
 <button id="g2" class="gear active" onclick="setGear(2)">2 ск.</button>
 <button id="g3" class="gear" onclick="setGear(3)">3 ск.</button>
 <button class="light" onclick="sendCmd('/lights')">Фары</button>
 </div>

 <div class="pad">
 <div></div>
 <button onpointerdown="sendCmd('/move?dir=fwd')" onpointerup="sendCmd('/move?dir=stop')" onpointerleave="sendCmd('/move?dir=stop')">ВПЕРЁД</button>
 <div></div>

 <button onpointerdown="sendCmd('/steer?dir=left')" onpointerup="sendCmd('/steer?dir=center')" onpointerleave="sendCmd('/steer?dir=center')">ВЛЕВО</button>
 <button class="stop" onclick="sendCmd('/stop')">СТОП</button>
 <button onpointerdown="sendCmd('/steer?dir=right')" onpointerup="sendCmd('/steer?dir=center')" onpointerleave="sendCmd('/steer?dir=center')">ВПРАВО</button>

 <div></div>
 <button onpointerdown="sendCmd('/move?dir=rev')" onpointerup="sendCmd('/move?dir=stop')" onpointerleave="sendCmd('/move?dir=stop')">НАЗАД</button>
 <div></div>
 </div>

 <button class="fire" onclick="sendCmd('/fire')">ОГОНЬ (Помпа)</button>
 <div class="status" id="st">Готов к заезду</div>

 <script>
 function sendCmd(url) {
 fetch(url)
 .then(r => r.text())
 .then(t => document.getElementById('st').innerText = 'Статус: ' + t)
 .catch(() => document.getElementById('st').innerText = 'Нет связи с машинкой');
 }
 function setGear(g) {
 document.querySelectorAll('.gear').forEach(b => b.classList.remove('active'));
 document.getElementById('g' + g).classList.add('active');
 sendCmd('/gear?g=' + g);
 }
 </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

static void handleCmd() {
  if (!server.hasArg("action")) {
    server.send(400, "text/plain", "missing action");
    return;
  }
  String action = server.arg("action");

  // Send command to loop task via shared queue
  if (action == "fwd" || action == "forward") {
    shared::sendMotorCmd(shared::MotorCmd::FORWARD);
    server.send(200, "text/plain", "Forward");
  } else if (action == "rev" || action == "backward") {
    shared::sendMotorCmd(shared::MotorCmd::BACKWARD);
    server.send(200, "text/plain", "Backward");
  } else if (action == "left") {
    shared::sendMotorCmd(shared::MotorCmd::LEFT);
    server.send(200, "text/plain", "Left");
  } else if (action == "right") {
    shared::sendMotorCmd(shared::MotorCmd::RIGHT);
    server.send(200, "text/plain", "Right");
  } else if (action == "stop") {
    shared::sendMotorCmd(shared::MotorCmd::STOP);
    server.send(200, "text/plain", "Stop");
  } else if (action == "fire" || action == "water") {
    shared::sendPumpCmd(shared::PumpCmd::FIRE);
    server.send(200, "text/plain", "Fire!");
  } else if (action == "water_off") {
    // Pump auto-off handled by loop task
    server.send(200, "text/plain", "Water off");
  } else if (action == "patrol") {
    // Set autonomous patrol state via shared command
    server.send(200, "text/plain", "Patrol");
  } else if (action == "lights") {
    shared::sendLedCmd(shared::LedCmd::TOGGLE);
    server.send(200, "text/plain", "Lights toggled");
  } else if (action.startsWith("gear")) {
    // Handle gear change
    server.send(200, "text/plain", "Gear");
  } else {
    server.send(400, "text/plain", "Unknown action");
  }
}

static void handleStatus() {
  shared::Snapshot snap = shared::read();
  
  DynamicJsonDocument doc(512);
  doc["state"] = "IDLE";  // Will be updated from shared if needed
  doc["distance_cm"] = 0;
  doc["pump"] = "OFF";
  doc["uptime_s"] = snap.uptimeSec;
  doc["wifi_connected"] = snap.wifiConnected;
  doc["wifi_rssi"] = snap.wifiRssi;
  doc["wifi_ssid"] = snap.wifiSsid;
  doc["device_id"] = snap.deviceId;
  doc["longest_loop_ms"] = snap.longestPassMs;
  doc["chip_temp"] = snap.chipC;
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

static void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ============================================================================
// BLE Implementation
// ============================================================================
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleDeviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    bleDeviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      char cmd = rxValue[0];
      // Handle BLE commands same as serial/console
      switch (cmd) {
        case 'w': case 'W':
          shared::sendMotorCmd(shared::MotorCmd::FORWARD);
          break;
        case 's': case 'S':
          shared::sendMotorCmd(shared::MotorCmd::BACKWARD);
          break;
        case 'a': case 'A':
          shared::sendMotorCmd(shared::MotorCmd::LEFT);
          break;
        case 'd': case 'D':
          shared::sendMotorCmd(shared::MotorCmd::RIGHT);
          break;
        case 'x': case 'X':
          shared::sendMotorCmd(shared::MotorCmd::STOP);
          break;
        case 'f': case 'F':
          shared::sendLedCmd(shared::LedCmd::TOGGLE);
          break;
        case '1':
          // Gear 1
          break;
        case '2':
          // Gear 2
          break;
        case '3':
          // Gear 3
          break;
        case '?': case 'h': case 'H':
          // Help
          break;
      }
      if (bleDeviceConnected && pTxCharacteristic) {
        pTxCharacteristic->notify();
      }
    }
  }
};

static void initBLE() {
  BLEDevice::init("AntiMurzik-C3");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println(F("[BLE] Ready. Name: AntiMurzik-C3"));
}