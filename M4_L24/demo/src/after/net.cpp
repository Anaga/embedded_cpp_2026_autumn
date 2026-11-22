#include "net.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <UniversalTelegramBot.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "board.h"
#include "shared.h"
#include "../common/wifi_store.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy include/secrets.example.h to include/secrets.h and fill it in"
#endif

namespace net {

namespace {

constexpr uint32_t WIFI_CONNECT_MS = 10000U;  // per attempt, inside run()
constexpr uint32_t WIFI_RETRY_MS   = 5000U;   // pause after a failed attempt
constexpr uint32_t BOT_POLL_MS     = 1000U;   // pause between two polls
constexpr uint32_t NET_IDLE_MS     = 20U;     // let loop() run
constexpr uint32_t NET_STACK_BYTES = 12288U;  // TLS needs a big stack
constexpr UBaseType_t NET_PRIORITY = 1U;      // same as loopTask: they share

WiFiMulti            wifiMulti;
WiFiClientSecure     tls;
UniversalTelegramBot bot(BOT_TOKEN, tls);

uint8_t  networkCount = 0U;
bool     botReady     = false;
uint32_t lastPoll     = 0U;

// ---------------------------------------------------------------- bot
void reply(const String &chatId, const String &text) {
  if (!bot.sendMessage(chatId, text, "")) {
    Serial.println("bot: send FAILED");
  }
}

String statusText() {
  shared::Snapshot s = shared::read();
  char sensor[40];
  if (s.sensorValid) {
    snprintf(sensor, sizeof(sensor), "%.1f C, %.1f %% RH", s.tempC, s.rhPct);
  } else {
    snprintf(sensor, sizeof(sensor), "sensor missing");
  }
  char buf[200];
  snprintf(buf, sizeof(buf),
           "%s\n%s\nchip %.1f C\nLED %s\nWiFi %s, %d dBm\nlongest loop pass %lu ms\nup %lu min",
           shared::deviceId(), sensor, s.chipC, s.ledOn ? "on" : "off",
           WiFi.SSID().c_str(), WiFi.RSSI(),
           static_cast<unsigned long>(s.longestPassMs),
           static_cast<unsigned long>(millis() / 60000UL));
  return String(buf);
}

void handleMessage(const telegramMessage &m) {
  Serial.printf("bot: chat %s sent a message\n", m.chat_id.c_str());

  String text = m.text;
  text.trim();
  text.toLowerCase();

  if ((text == "/start") || (text == "/help")) {
    reply(m.chat_id, String(shared::deviceId()) +
                     " here.\n/status - sensor, LED, WiFi\n/led on, /led off, /led toggle");
  } else if (text == "/status") {
    reply(m.chat_id, statusText());
  } else if ((text == "/led on") || (text == "/led off") || (text == "/led toggle")) {
    shared::LedCmd cmd = (text == "/led on")  ? shared::LedCmd::ON
                       : (text == "/led off") ? shared::LedCmd::OFF
                                              : shared::LedCmd::TOGGLE;
    // Not ledSet() here: loop() owns the pins. Send a command instead.
    reply(m.chat_id, shared::sendLedCmd(cmd) ? "OK" : "busy, try again");
  } else {
    reply(m.chat_id, "Unknown command. Try /help");
  }
}

void pollBot() {
  if (!botReady) {
    if (!bot.getMe()) {
      Serial.println("bot: getMe failed - wrong BOT_TOKEN, or no internet");
      return;
    }
    botReady = true;
    Serial.printf("bot: @%s is online\n", bot.userName.c_str());
  }
  // offset = last update id + 1: "I have seen everything up to here"
  int n = bot.getUpdates(bot.last_message_received + 1);
  while (n > 0) {
    for (int i = 0; i < n; i++) {
      handleMessage(bot.messages[i]);
    }
    n = bot.getUpdates(bot.last_message_received + 1);
  }
}

// ---------------------------------------------------------------- WiFi
void connectWifi() {
  if (networkCount == 0U) {
    return;                          // nothing to connect to; see begin()
  }
  Serial.println("net: scanning for saved networks...");
  // Scans, picks the strongest saved network, connects. Blocks for seconds -
  // but only this task.
  if (wifiMulti.run(WIFI_CONNECT_MS) == WL_CONNECTED) {
    Serial.printf("net: connected to \"%s\", %d dBm, %s\n",
                  WiFi.SSID().c_str(), WiFi.RSSI(),
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.println("net: no saved network found, retry in 5 s");
    vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_MS));
  }
}

// ---------------------------------------------------------------- task
void netTask(void *) {
  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      connectWifi();                 // seconds
    } else if (millis() - lastPoll >= BOT_POLL_MS) {
      pollBot();                     // 0.3-2 s
      lastPoll = millis();           // the pause starts AFTER the poll
    }
    // Inside a task this is fine: only THIS task sleeps, loop() runs on.
    vTaskDelay(pdMS_TO_TICKS(NET_IDLE_MS));
  }
}

}  // namespace

void begin() {
  wifi_store::Network nets[wifi_store::MAX_NETWORKS];
  networkCount = wifi_store::load(nets, wifi_store::MAX_NETWORKS);
  for (uint8_t i = 0U; i < networkCount; i++) {
    wifiMulti.addAP(nets[i].ssid, nets[i].pass);   // strdup()s the strings
  }
  Serial.printf("net: %u saved network(s)\n", static_cast<unsigned>(networkCount));
  if (networkCount == 0U) {
    Serial.println("net: type  wifi add \"<ssid>\" <password>  and then  reboot");
  }

  WiFi.setHostname(shared::deviceId());   // core 2.x: before WiFi.mode()
  WiFi.mode(WIFI_STA);
  if (LIMIT_TX_POWER) {
    WiFi.setTxPower(TX_POWER);
  }
  // Encrypted, but the server certificate is not checked (lesson 23).
  tls.setInsecure();

  xTaskCreate(netTask, "net", NET_STACK_BYTES, nullptr, NET_PRIORITY, nullptr);
}

}  // namespace net
