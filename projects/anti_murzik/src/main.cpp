// Anti-Murzik (Диванный страж) - Lesson 24 AFTER architecture
//
// Modular non-blocking firmware for ESP32-C3 Super Mini robot:
//   - DRV8833 motor control (drive + steer)
//   - Water pump (non-blocking spray)
//   - HC-SR04 distance sensor
//   - State machine (IDLE, MANUAL_CONTROL, AUTONOMOUS_PATROL, SAFETY_STOP, WATERING)
//   - Web server + BLE + Serial console in FreeRTOS task (net.cpp)
//   - RGB LED status + heartbeat
//   - Button A for local control
//   - Chip temperature monitoring
//   - Loop timing statistics
//
// Rules:
//   - a part never waits: it checks the clock, does a little, returns
//   - each part: begin() once from setup(), update(now) every pass
//   - WiFi and BLE must wait, so they run in their own task (net.cpp)
//   - loop() owns the pins; the net task sends commands via shared queues
//   - LoopStats prints the longest loop pass every 5 s

#include <Arduino.h>
#include <WiFi.h>

#include "board.h"
#include "rgb_led.h"
#include "heartbeat.h"
#include "button.h"
#include "chip_temp.h"
#include "console.h"
#include "shared.h"
#include "net.h"
#include "MotorController.h"
#include "WaterPump.h"
#include "DistanceSensor.h"
#include "StateMachine.h"
#include "loop_stats.h"

static void printStatus();

// ---------------------------------------------------------------- Components
static RgbLed            rgb;
static Heartbeat         heartbeat(rgb);
static Button            button(PIN_BUTTON_A);
static ChipTemp          chipTemp;
static Console           console(printStatus);
static LoopStats         stats;

static MotorController   motors;
static WaterPump         pump;
static DistanceSensor    distSensor(PIN_DIST_TRIG, PIN_DIST_ECHO);
static AntiMurzikStateMachine fsm;

// ---------------------------------------------------------------- State
static bool     ledOn        = false;
static uint32_t lastSnapshot = 0U;
static uint8_t  currentGear  = 2;

// ---------------------------------------------------------------- Glue functions

// LED control from button/console/bot
static void ledSet(bool on) {
  ledOn = on;
  rgb.set(RgbLed::Channel::GREEN, on);
}

static void ledToggle() {
  ledSet(!ledOn);
  Serial.printf("LED %s (button)\n", ledOn ? "on" : "off");
}

// Commands the net task has queued for us. loop() owns the pins.
static void netCommands() {
  shared::Command cmd;
  while (shared::receiveCommand(cmd)) {
    switch (cmd.type) {
      case shared::Command::Type::LED:
        switch (cmd.cmd.led) {
          case shared::LedCmd::ON:     ledSet(true);   break;
          case shared::LedCmd::OFF:    ledSet(false);  break;
          case shared::LedCmd::TOGGLE: ledSet(!ledOn); break;
        }
        Serial.printf("LED %s (bot)\n", ledOn ? "on" : "off");
        break;
        
      case shared::Command::Type::MOTOR:
        switch (cmd.cmd.motor) {
          case shared::MotorCmd::FORWARD:  motors.forward();  break;
          case shared::MotorCmd::BACKWARD: motors.backward(); break;
          case shared::MotorCmd::LEFT:     motors.turnLeft(); break;
          case shared::MotorCmd::RIGHT:    motors.turnRight(); break;
          case shared::MotorCmd::STOP:     motors.stop();     break;
          case shared::MotorCmd::GEAR_1:   motors.setGear(1); currentGear = 1; break;
          case shared::MotorCmd::GEAR_2:   motors.setGear(2); currentGear = 2; break;
          case shared::MotorCmd::GEAR_3:   motors.setGear(3); currentGear = 3; break;
        }
        Serial.printf("Motor: %d (speed=%d)\n", (int)cmd.cmd.motor, cmd.param);
        break;
        
      case shared::Command::Type::PUMP:
        if (cmd.cmd.pump == shared::PumpCmd::FIRE) {
          pump.activate();
          fsm.setState(AntiMurzikStateMachine::AntiMurzikState::WATERING);
        }
        break;
    }
  }
}

// Console commands that affect hardware
static void consoleCommands() {
  // Console commands are handled via the Console class
  // which calls printStatus() for 'status' command
  // Other commands (wifi, reboot) are handled internally
}

// Safety stop: if obstacle too close while moving
static void safetyCheck(uint32_t now) {
  if (distSensor.getState() == DistanceSensor::DistanceState::OBSTACLE_DETECTED &&
      motors.getState() != MotorController::MotorState::STOP) {
    motors.stop();
    fsm.setState(AntiMurzikStateMachine::AntiMurzikState::SAFETY_STOP);
    Serial.println(F("[SAFETY] Obstacle detected - EMERGENCY STOP"));
  }
}

// A copy of what /status needs, for the net task.
static void publishSnapshot(uint32_t now) {
  if (now - lastSnapshot < SNAPSHOT_PERIOD_MS) return;
  lastSnapshot = now;
  
    shared::Snapshot snap;
  snap.sensorValid = (distSensor.getState() == DistanceSensor::DistanceState::CLEAR);
  snap.tempC = NAN;
  snap.rhPct = NAN;
  snap.chipC = chipTemp.celsius();
  snap.ledOn = ledOn;
  snap.motorState = (int)motors.getState();
  snap.gear = currentGear;
  snap.longestPassMs = stats.longestMs();
  snap.uptimeSec = now / 1000;
  snap.wifiConnected = WiFi.isConnected();
  snap.wifiRssi = WiFi.RSSI();
  strncpy(snap.wifiSsid, WiFi.SSID().c_str(), 32);
  snap.wifiSsid[32] = '\0';
  strncpy(snap.deviceId, shared::deviceId(), 15);
  snap.deviceId[15] = '\0';
  shared::publish(snap);
}

static void printStatus() {
  Serial.printf("%s, up %lu min\n", shared::deviceId(),
                static_cast<unsigned long>(millis() / 60000UL));
  Serial.printf("State: %s\n", fsm.getStateName());
  Serial.printf("Gear: %u, Motor: %d\n", currentGear, (int)motors.getState());
  Serial.printf("Distance: %.1f cm (%s)\n", 
                distSensor.getDistanceCm(),
                distSensor.getState() == DistanceSensor::DistanceState::OBSTACLE_DETECTED ? "OBSTACLE" : "CLEAR");
  Serial.printf("Pump: %s\n", pump.isSpraying() ? "SPRAYING" : "OFF");
  Serial.printf("LED: %s\n", ledOn ? "ON" : "OFF");
  Serial.printf("Chip temp: %.1f C\n", chipTemp.celsius());
  if (WiFi.isConnected()) {
    Serial.printf("WiFi: \"%s\", %d dBm\n", WiFi.SSID().c_str(), WiFi.RSSI());
  } else {
    Serial.println("WiFi: not connected");
  }
  Serial.printf("Longest loop pass: %lu ms (last 5 s)\n",
                static_cast<unsigned long>(stats.longestMs()));
}

// ---------------------------------------------------------------- Main
void setup() {
  Serial.begin(115200);
  delay(2000);                       // USB CDC needs a moment after reset
  Serial.println();
  Serial.println(F("=== Anti-Murzik v2.0 (Lesson 24 Architecture) ==="));

  // Initialize components
  rgb.begin();
  heartbeat.begin();
  button.begin();
  chipTemp.begin();
  console.begin();
  
  motors.begin();
  pump.begin();
  distSensor.begin();
  fsm.begin();

  shared::begin();
  Serial.printf("Device %s\n", shared::deviceId());
  
  net::begin();                      // WiFi + WebServer + BLE: own task
  
  stats.begin();
  
  Serial.println(F("(SYSTEM) Anti-Murzik READY!"));
  console.update();  // Print initial help
}

void loop() {
  uint32_t now = millis();
  
  // Button handling
  button.update(now);
  if (button.pressed()) ledToggle();
  
  // Process commands from net task (WiFi/BLE/Console)
  netCommands();
  consoleCommands();
  
  // Console input (serial monitor)
  console.update();
  
  // Sensor updates
  distSensor.update(now);
  
  // Safety check
  safetyCheck(now);
  
  // Actuator updates
  pump.update(now);
  
  // Heartbeat
  heartbeat.update(now);
  
  // Chip temperature
  chipTemp.update(now);
  
  // Publish snapshot for net task
  publishSnapshot(now);
  
  // Loop statistics (call LAST)
  stats.update(now);
}