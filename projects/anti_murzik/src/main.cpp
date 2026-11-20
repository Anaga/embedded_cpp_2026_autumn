#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "MotorController.h"
#include "WaterPump.h"
#include "DistanceSensor.h"
#include "StateMachine.h"

// ==================== Globals ====================
WebServer server(WEB_SERVER_PORT);
MotorController motors(MOTOR_L_EN, MOTOR_L_IN1, MOTOR_L_IN2,
                        MOTOR_R_EN, MOTOR_R_IN1, MOTOR_R_IN2);
WaterPump pump(PUMP_PIN);
DistanceSensor distSensor(TRIG_PIN, ECHO_PIN);
AntiMurzikStateMachine fsm;

unsigned long lastHeartbeatMs = 0;
unsigned long lastSprayMs = 0;

// ==================== WiFi Setup ====================
void setupWiFiAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    delay(100);
}

// ==================== Web Handlers ====================
void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Anti-Murzik Control</title>
<style>
  body { font-family: sans-serif; background: #1a1a2e; color: #e0e0e0; text-align: center; padding: 20px; }
  h1 { color: #00d4ff; }
  .btn { font-size: 24px; padding: 20px 30px; margin: 10px; border: none; border-radius: 12px; cursor: pointer; }
  .btn:disabled { opacity: 0.4; cursor: not-allowed; }
  .forward { background: #4caf50; color: white; }
  .backward { background: #f44336; color: white; }
  .left { background: #ff9800; color: white; }
  .right { background: #ff9800; color: white; }
  .stop { background: #607d8b; color: white; }
  .water { background: #2196f3; color: white; font-size: 32px; padding: 30px 50px; }
  .status { background: #16213e; padding: 20px; border-radius: 10px; margin-top: 20px; }
  .grid { display: grid; grid-template-columns: repeat(3,1fr); gap: 10px; max-width: 400px; margin: 20px auto; }
</style>
</head>
<body>
<h1>Anti-Murzik Control</h1>
<div class="status" id="status">
  <p>State: <span id="fsm_state">UNKNOWN</span></p>
  <p>Distance: <span id="distance">--</span> cm</p>
  <p>Pump: <span id="pump_state">OFF</span></p>
  <p>Uptime: <span id="uptime">0</span> s</p>
</div>

<div class="grid">
  <div></div>
  <button class="btn forward" ontouchstart="cmd('forward')" onmousedown="cmd('forward')" ontouchend="cmd('stop')" onmouseup="cmd('stop')">FORWARD</button>
  <div></div>
  <button class="btn left" ontouchstart="cmd('left')" onmousedown="cmd('left')" ontouchend="cmd('stop')" onmouseup="cmd('stop')">LEFT</button>
  <button class="btn stop" onclick="cmd('stop')">STOP</button>
  <button class="btn right" ontouchstart="cmd('right')" onmousedown="cmd('right')" ontouchend="cmd('stop')" onmouseup="cmd('stop')">RIGHT</button>
  <div></div>
  <button class="btn backward" ontouchstart="cmd('backward')" onmousedown="cmd('backward')" ontouchend="cmd('stop')" onmouseup="cmd('stop')">BACKWARD</button>
  <div></div>
</div>

<button class="btn water" ontouchstart="cmd('water')" onmousedown="cmd('water')" ontouchend="cmd('water_off')" onmouseup="cmd('water_off')">WATER</button>

<script>
function cmd(c) {
  fetch('/cmd?action=' + c).then(r => r.text()).then(t => console.log(t));
}
setInterval(() => {
  fetch('/status').then(r => r.json()).then(d => {
    document.getElementById('fsm_state').textContent = d.state;
    document.getElementById('distance').textContent = d.distance_cm;
    document.getElementById('pump_state').textContent = d.pump;
    document.getElementById('uptime').textContent = d.uptime_s;
  });
}, 500);
</script>
</body>
</html>
)rawliteral";
    server.send(200, "text/html", html);
}

void handleCmd() {
    if (!server.hasArg("action")) {
        server.send(400, "application/json", "{\"error\":\"missing action\"}");
        return;
    }
    String action = server.arg("action");

    if (action == "forward") {
        motors.forward(150);
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::MANUAL_CONTROL);
    } else if (action == "backward") {
        motors.backward(150);
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::MANUAL_CONTROL);
    } else if (action == "left") {
        motors.turnLeft(150);
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::MANUAL_CONTROL);
    } else if (action == "right") {
        motors.turnRight(150);
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::MANUAL_CONTROL);
    } else if (action == "stop") {
        motors.stop();
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::IDLE);
    } else if (action == "water") {
        unsigned long now = millis();
        if (now - lastSprayMs > PUMP_COOLDOWN_MS) {
            pump.activate();
            lastSprayMs = now;
            fsm.setState(AntiMurzikStateMachine::AntiMurzikState::WATERING);
        }
    } else if (action == "water_off") {
        pump.deactivate();
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::MANUAL_CONTROL);
    } else if (action == "patrol") {
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::AUTONOMOUS_PATROL);
    } else {
        server.send(400, "application/json", "{\"error\":\"unknown action\"}");
        return;
    }

    server.send(200, "application/json", "{\"ok\":true,\"action\":\"" + action + "\"}");
}

void handleStatus() {
    DynamicJsonDocument doc(256);
    doc["state"] = fsm.getStateName();
    doc["distance_cm"] = distSensor.getDistanceCm();
    doc["pump"] = pump.isSpraying() ? "SPRAYING" : "OFF";
    doc["uptime_s"] = millis() / 1000;
    doc["motor_state"] = (int)motors.getMotorState();

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

// ==================== Setup ====================
void setup() {
    // Hardware init
    motors.init();
    pump.init();
    distSensor.init();

    // WiFi AP
    setupWiFiAP();

    // Web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/cmd", HTTP_GET, handleCmd);
    server.on("/status", HTTP_GET, handleStatus);
    server.onNotFound(handleNotFound);
    server.begin();
}

// ==================== Loop ====================
void loop() {
    server.handleClient();

    unsigned long now = millis();

    // Distance sensor periodic read
    distSensor.readDistanceCm();

    // Safety stop: if obstacle too close while moving
    if (distSensor.getState() == DistanceSensor::DistanceState::OBSTACLE_DETECTED &&
        motors.getMotorState() != MotorController::MotorState::STOP) {
        motors.stop();
        fsm.setState(AntiMurzikStateMachine::AntiMurzikState::SAFETY_STOP);
    }

    // Pump auto-off after spray duration
    if (pump.isSpraying() && (now - lastSprayMs > PUMP_SPRAY_DURATION_MS)) {
        pump.deactivate();
    }
}
