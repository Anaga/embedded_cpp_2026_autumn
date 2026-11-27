#include "shared.h"
#include <Arduino.h>
#include <esp_mac.h>
#include <stdio.h>

namespace shared {

namespace {

constexpr UBaseType_t CMD_QUEUE_LENGTH = 8;

QueueHandle_t cmdQueue = nullptr;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
Snapshot current = {};

char id[16] = "esp-????";

}  // namespace

void begin() {
    cmdQueue = xQueueCreate(CMD_QUEUE_LENGTH, sizeof(Command));
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(id, sizeof(id), "esp-%02x%02x", mac[4], mac[5]);
    Serial.printf("[shared] Device ID: %s\n", id);
}

const char* deviceId() {
    return id;
}

bool sendLedCmd(LedCmd cmd) {
    Command c;
    c.type = Command::Type::LED;
    c.cmd.led = cmd;
    return xQueueSend(cmdQueue, &c, 0) == pdTRUE;
}

bool sendMotorCmd(MotorCmd cmd, uint8_t speed) {
    Command c;
    c.type = Command::Type::MOTOR;
    c.cmd.motor = cmd;
    c.param = speed;
    return xQueueSend(cmdQueue, &c, 0) == pdTRUE;
}

bool sendPumpCmd(PumpCmd cmd) {
    Command c;
    c.type = Command::Type::PUMP;
    c.cmd.pump = cmd;
    return xQueueSend(cmdQueue, &c, 0) == pdTRUE;
}

bool receiveCommand(Command& out) {
    return xQueueReceive(cmdQueue, &out, 0) == pdTRUE;
}

void publish(const Snapshot& s) {
    portENTER_CRITICAL(&mux);
    current = s;
    portEXIT_CRITICAL(&mux);
}

Snapshot read() {
    portENTER_CRITICAL(&mux);
    Snapshot copy = current;
    portEXIT_CRITICAL(&mux);
    return copy;
}

}  // namespace shared