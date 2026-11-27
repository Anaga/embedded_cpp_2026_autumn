#include "shared.h"

#include "config.h"

namespace shared {

volatile bool online = false;
volatile uint32_t muteUntilMs = 0U;

static Latest latest;
static SemaphoreHandle_t latestLock = nullptr;  // a mutex: only one task holds it at a time
static QueueHandle_t alertQueue = nullptr;      // a thread-safe mailbox

void begin() {
  latestLock = xSemaphoreCreateMutex();
  alertQueue = xQueueCreate(ALERT_QUEUE_LEN, sizeof(Alert));
}

void publish(const Latest& l) {
  xSemaphoreTake(latestLock, portMAX_DELAY);
  latest = l;
  xSemaphoreGive(latestLock);
}

Latest read() {
  xSemaphoreTake(latestLock, portMAX_DELAY);
  Latest copy = latest;  // copy it, then let go of the lock straight away
  xSemaphoreGive(latestLock);
  return copy;
}

void queueAlert(const char* text) {
  Alert a;
  strncpy(a.text, text, sizeof(a.text) - 1U);
  a.text[sizeof(a.text) - 1U] = '\0';
  if (xQueueSend(alertQueue, &a, 0) != pdTRUE) Serial.println("alert queue full - dropped");
}

bool peekAlert(Alert& a) { return xQueuePeek(alertQueue, &a, 0) == pdTRUE; }

void dropAlert() {
  Alert a;
  xQueueReceive(alertQueue, &a, 0);
}

uint8_t alertsWaiting() { return static_cast<uint8_t>(uxQueueMessagesWaiting(alertQueue)); }

bool muted() {
  uint32_t until = muteUntilMs;
  return until != 0U && static_cast<int32_t>(until - millis()) > 0;
}

}  // namespace shared
