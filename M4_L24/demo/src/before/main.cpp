// Lesson 24 demo - BEFORE: every part glued into loop()
//
// This is what a project often looks like after "every part worked alone":
// the WiFi code, the sensor code, the bot code, the button code and the
// blink code, each copied in as it was tested, one after the other.
// Each part works. Together they wait for each other.
//
// Watch:
//   - the blue heartbeat should blink 4 times a second - it stutters
//   - press button A a few times quickly - some presses are lost
//   - /status in Telegram answers late
//   - switch the router off - everything stops, the button too
//   - "loop: ... longest pass ... ms" on Serial says why
//
// The fixed version is in src/after/. Do not copy this file.

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <UniversalTelegramBot.h>

#include "board.h"
#include "../common/loop_stats.h"
#include "../common/wifi_store.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy include/secrets.example.h to include/secrets.h and fill it in"
#endif

static WiFiMulti            wifiMulti;
static WiFiClientSecure     tls;
static UniversalTelegramBot bot(BOT_TOKEN, tls);
static Adafruit_AHTX0       aht;
static LoopStats            stats;

static bool  ahtOk = false;
static bool  ledOn = false;
static float tempC = NAN;
static float rhPct = NAN;

static void ledWrite() {
  digitalWrite(PIN_LED_G, ledOn ? LOW : HIGH);   // common anode: LOW = on
}

static void toggleLed() {
  ledOn = !ledOn;
  ledWrite();
  Serial.printf("LED %s\n", ledOn ? "on" : "off");
}

static void handle(const telegramMessage &m) {
  if (m.text == "/status") {
    char buf[96];
    snprintf(buf, sizeof(buf), "%.1f C, %.1f %% RH, LED %s",
             tempC, rhPct, ledOn ? "on" : "off");
    bot.sendMessage(m.chat_id, buf, "");
  } else if (m.text == "/led on") {
    ledOn = true;
    ledWrite();
    bot.sendMessage(m.chat_id, "LED on", "");
  } else if (m.text == "/led off") {
    ledOn = false;
    ledWrite();
    bot.sendMessage(m.chat_id, "LED off", "");
  } else {
    bot.sendMessage(m.chat_id, "Try /status, /led on, /led off", "");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("Lesson 24 - BEFORE: everything in one loop");

  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  digitalWrite(PIN_LED_R, HIGH);
  digitalWrite(PIN_LED_G, HIGH);
  digitalWrite(PIN_LED_B, HIGH);
  pinMode(PIN_BUTTON_A, INPUT_PULLUP);

  Wire.begin(PIN_SDA, PIN_SCL);
  ahtOk = aht.begin(&Wire);
  Serial.println(ahtOk ? "AHT10 found" : "AHT10 missing");

  wifi_store::Network nets[wifi_store::MAX_NETWORKS];
  uint8_t count = wifi_store::load(nets, wifi_store::MAX_NETWORKS);
  if (count == 0U) {
    Serial.println("No WiFi saved. Flash env 'after' and type: wifi add \"<ssid>\" <password>");
  }
  for (uint8_t i = 0U; i < count; i++) {
    wifiMulti.addAP(nets[i].ssid, nets[i].pass);
  }
  WiFi.mode(WIFI_STA);
  if (LIMIT_TX_POWER) {
    WiFi.setTxPower(TX_POWER);
  }
  tls.setInsecure();
  stats.begin();
}

void loop() {
  // WiFi: wait until connected (every part was tested like this)
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi: connecting...");
    delay(500);
  }

  // Sensor: read every pass (getEvent waits for the conversion)
  sensors_event_t h;
  sensors_event_t t;
  if (ahtOk && aht.getEvent(&h, &t)) {
    tempC = t.temperature;
    rhPct = h.relative_humidity;
  }

  // Bot: ask Telegram for new messages (HTTPS round trip)
  int n = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < n; i++) {
    handle(bot.messages[i]);
  }

  // Button: toggle the green LED
  if (digitalRead(PIN_BUTTON_A) == LOW) {
    toggleLed();
    delay(200);                      // "debounce"
  }

  // Heartbeat: one blink of the blue LED
  digitalWrite(PIN_LED_B, LOW);
  delay(125);
  digitalWrite(PIN_LED_B, HIGH);
  delay(125);

  stats.update(millis());
}
