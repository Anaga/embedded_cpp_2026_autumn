#include "shared.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_mac.h>
#include <stdio.h>

namespace shared {

namespace {

constexpr UBaseType_t LED_QUEUE_LENGTH = 4U;

QueueHandle_t ledQueue = nullptr;

portMUX_TYPE mux     = portMUX_INITIALIZER_UNLOCKED;
Snapshot     current = {false, NAN, NAN, NAN, false, 0U};

char id[16] = "esp-????";

}  // namespace

void begin() {
  ledQueue = xQueueCreate(LED_QUEUE_LENGTH, sizeof(LedCmd));
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(id, sizeof(id), "esp-%02x%02x", mac[4], mac[5]);
}

const char *deviceId() {
  return id;                         // written once in begin(), then read only
}

bool sendLedCmd(LedCmd cmd) {
  // Timeout 0: never wait. A full queue means loop() is stuck anyway.
  return xQueueSend(ledQueue, &cmd, 0) == pdTRUE;
}

bool receiveLedCmd(LedCmd &cmd) {
  return xQueueReceive(ledQueue, &cmd, 0) == pdTRUE;
}

void publish(const Snapshot &s) {
  portENTER_CRITICAL(&mux);
  current = s;                       // a few bytes: microseconds
  portEXIT_CRITICAL(&mux);
}

Snapshot read() {
  portENTER_CRITICAL(&mux);
  Snapshot copy = current;
  portEXIT_CRITICAL(&mux);
  return copy;
}

}  // namespace shared
