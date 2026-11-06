// Lesson 19 - Lab 3: a sleeping AHT10 node that reports over ESP-NOW
//
// Every wake: read the AHT10, send one NodeMsg to the gateway, wait for
// the delivery status, sleep again. The node never joins the WiFi
// network: it only needs to be on the same channel as the gateway.
//
// The channel: the gateway is joined to the class router, so it listens
// on the router's channel. The node scans for CLASS_SSID once and keeps
// the channel in RTC memory. After MAX_FAILS failed sends in a row it
// scans again (the router may have moved to another channel).
//
// LED, as in labs 1 and 2: red blink = just woke up, blue = awake,
// green = asleep (held LOW through deep sleep). Three red blinks = the
// gateway did not acknowledge the packet. Blinks count as "waiting".
//
// Before the first build: fill in include/class_net.h (whiteboard).
//
// Wiring (session 18):
//   AHT10     SDA GPIO 8, SCL GPIO 9, VIN 3V3, GND
//   Button A  GPIO 0 -> button -> GND (INPUT_PULLUP, active low)
//   RGB LED   R 5, G 6, B 7, common anode to 3V3, LOW = on
// GPIO 8 and 9 are strapping pins: the AHT10 pull-ups keep them HIGH at
// reset (normal boot). GPIO 9 is also the BOOT button.

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <Adafruit_AHTX0.h>

#include "class_net.h"
#include "msg.h"

// ---------------------------------------------------------------- pins
constexpr uint8_t PIN_BUTTON = 0;   // only GPIO 0..5 can wake the C3 from deep sleep
constexpr uint8_t PIN_SDA    = 8;
constexpr uint8_t PIN_SCL    = 9;
constexpr uint8_t PIN_LED_R  = 5;
constexpr uint8_t PIN_LED_G  = 6;
constexpr uint8_t PIN_LED_B  = 7;
constexpr uint8_t LED_PINS[] = {PIN_LED_R, PIN_LED_G, PIN_LED_B};

// ---------------------------------------------------------------- timing
constexpr uint64_t US_PER_S           = 1000000ULL;
constexpr uint64_t SLEEP_S            = 20;
constexpr uint32_t AWAKE_WINDOW_MS    = 30000;  // blue: time to start an upload
constexpr uint32_t SERIAL_WAIT_MS     = 2000;   // USB comes back after a wake
constexpr uint32_t RED_BLINK_MS       = 300;
constexpr uint32_t FAIL_BLINK_MS      = 150;
constexpr uint8_t  FAIL_BLINKS        = 3;
constexpr uint32_t RELEASE_TIMEOUT_MS = 5000;
constexpr uint32_t RELEASE_POLL_MS    = 10;
constexpr uint32_t SEND_TIMEOUT_MS    = 100;    // the ACK normally takes a few ms

// ---------------------------------------------------------------- radio
constexpr uint8_t  AHT10_ADDRESS = 0x38;
constexpr uint8_t  MAX_FAILS     = 3;           // then scan for the channel again
constexpr uint8_t  NO_CHANNEL    = 0;
constexpr int      MAC_LEN       = 6;
constexpr int16_t  TENTHS        = 10;
// Many Super Mini boards work only with reduced TX power (session 18).
constexpr wifi_power_t TX_POWER = WIFI_POWER_8_5dBm;

// ---------------------------------------------------------------- RTC memory
RTC_DATA_ATTR static uint32_t bootCount   = 0;
RTC_DATA_ATTR static uint16_t seq         = 0;
RTC_DATA_ATTR static uint8_t  channel     = NO_CHANNEL;
RTC_DATA_ATTR static uint8_t  failCount   = 0;
RTC_DATA_ATTR static uint16_t prevAwakeMs = 0;

// ---------------------------------------------------------------- state
static uint32_t waitedMs = 0;       // this wake: blinks + USB wait + upload window
static char deviceId[MSG_NAME_LEN];  // "esp-xxxx"
static Adafruit_AHTX0 aht;

// Written by the send callback (WiFi task), read in setup().
static volatile bool sendDone = false;
static volatile bool sendOk   = false;

// ---------------------------------------------------------------- LED
// See lab 1. Common anode: LOW = on.
static void ledSet(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, r ? LOW : HIGH);
  digitalWrite(PIN_LED_G, g ? LOW : HIGH);
  digitalWrite(PIN_LED_B, b ? LOW : HIGH);
}

static void ledsBegin() {
  for (uint8_t pin : LED_PINS) {
    gpio_hold_dis(static_cast<gpio_num_t>(pin));
    pinMode(pin, OUTPUT);
  }
  ledSet(false, false, false);
}

static void holdSleepColour() {
  ledSet(false, true, false);         // green on through deep sleep
  for (uint8_t pin : LED_PINS) {
    gpio_hold_en(static_cast<gpio_num_t>(pin));
  }
  gpio_deep_sleep_hold_en();
}

static void waitMs(uint32_t ms) {
  delay(ms);
  waitedMs += ms;
}

static void blinkFail() {
  for (uint8_t i = 0; i < FAIL_BLINKS; i++) {
    ledSet(true, false, false);
    waitMs(FAIL_BLINK_MS);
    ledSet(false, false, true);       // back to blue
    waitMs(FAIL_BLINK_MS);
  }
}

// ---------------------------------------------------------------- helpers
static void makeDeviceId() {
  uint8_t mac[MAC_LEN];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(deviceId, sizeof(deviceId), "esp-%02x%02x", mac[4], mac[5]);
}

static bool gatewayMacSet() {
  for (int i = 0; i < MAC_LEN; i++) {
    if (GATEWAY_MAC[i] != 0) return true;
  }
  return false;
}

static const char *wakeText(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER: return "timer";
    case ESP_SLEEP_WAKEUP_GPIO:  return "button";
    default:                     return "cold";
  }
}

// ---------------------------------------------------------------- AHT10
// As in session 18: check the address first, because the library hangs
// on a missing sensor (status reads 0xFF = busy).
static bool ahtResponds() {
  Wire.beginTransmission(AHT10_ADDRESS);
  return Wire.endTransmission() == 0;
}

static bool readSensor(float &t, float &rh) {
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!ahtResponds() || !aht.begin(&Wire)) return false;
  sensors_event_t h, temp;
  if (!ahtResponds() || !aht.getEvent(&h, &temp)) return false;
  t = temp.temperature;
  rh = h.relative_humidity;
  return true;
}

// ---------------------------------------------------------------- channel
// Scans all channels (about 2 s awake). Returns NO_CHANNEL if not found.
static uint8_t findChannel(const char *ssid) {
  int n = WiFi.scanNetworks();
  uint8_t found = NO_CHANNEL;
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == ssid) {
      found = static_cast<uint8_t>(WiFi.channel(i));
      break;
    }
  }
  WiFi.scanDelete();
  return found;
}

// The node is not connected to any AP, so it may choose its channel.
// The promiscuous on / off around the call is the usual way to make the
// driver accept the change in core 2.x.
static void setChannel(uint8_t ch) {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
}

// ---------------------------------------------------------------- ESP-NOW
// Core 2.x signature. Core 3.x examples online use a different one.
// Runs in the WiFi task: record and return.
static void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  (void)mac;
  sendOk = (status == ESP_NOW_SEND_SUCCESS);
  sendDone = true;
}

static bool espNowBegin() {
  if (esp_now_init() != ESP_OK) return false;
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, GATEWAY_MAC, MAC_LEN);
  peer.channel = 0;                   // 0 = the channel the radio is on now
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  return esp_now_add_peer(&peer) == ESP_OK;
}

// Returns true if the gateway's radio acknowledged the packet.
static bool sendReading(bool sensorOk, float t, float rh) {
  NodeMsg msg = {};
  msg.version  = MSG_VERSION;
  msg.flags    = sensorOk ? MSG_FLAG_SENSOR_OK : 0;
  msg.seq      = ++seq;
  msg.t_c10    = sensorOk ? static_cast<int16_t>(lroundf(t * TENTHS)) : 0;
  msg.rh_10    = sensorOk ? static_cast<uint16_t>(lroundf(rh * TENTHS)) : 0;
  msg.awake_ms = prevAwakeMs;
  memcpy(msg.name, deviceId, sizeof(msg.name));

  sendDone = false;
  sendOk = false;
  if (esp_now_send(GATEWAY_MAC, reinterpret_cast<const uint8_t *>(&msg), sizeof(msg)) != ESP_OK) {
    return false;                     // not even queued
  }
  uint32_t start = millis();
  while (!sendDone && millis() - start < SEND_TIMEOUT_MS) {
    delay(1);
  }
  return sendDone && sendOk;
}

// ---------------------------------------------------------------- sleep
static void waitForRelease() {
  uint32_t start = millis();
  while (digitalRead(PIN_BUTTON) == LOW && millis() - start < RELEASE_TIMEOUT_MS) {
    delay(RELEASE_POLL_MS);
  }
}

static void goToSleep() {
  waitForRelease();
  uint32_t awakeMs = millis() - waitedMs;
  prevAwakeMs = static_cast<uint16_t>(awakeMs > UINT16_MAX ? UINT16_MAX : awakeMs);
  Serial.printf("   awake %lu ms (+%lu waiting), sleeping %lu s\n",
                static_cast<unsigned long>(awakeMs), static_cast<unsigned long>(waitedMs),
                static_cast<unsigned long>(SLEEP_S));
  Serial.flush();

  WiFi.mode(WIFI_OFF);
  holdSleepColour();
  esp_sleep_enable_timer_wakeup(SLEEP_S * US_PER_S);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BUTTON, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();             // does not return
}

// ---------------------------------------------------------------- main
void setup() {
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  bootCount++;

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  ledsBegin();
  ledSet(true, false, false);         // red blink: just woke up
  waitMs(RED_BLINK_MS);
  ledSet(false, false, true);         // blue: awake
  makeDeviceId();

  Serial.begin(115200);
  waitMs(SERIAL_WAIT_MS);             // only for the USB monitor

  if (cause == ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.println();
    Serial.printf("Lesson 19 - Lab 3 - sleeping node %s\n", deviceId);
    channel = NO_CHANNEL;             // find it again after a reset
    failCount = 0;
  }
  Serial.printf("#%lu %s\n", static_cast<unsigned long>(bootCount), wakeText(cause));

  // 1. Sensor first, while the radio is still off.
  float t = NAN;
  float rh = NAN;
  bool sensorOk = readSensor(t, rh);
  if (sensorOk) {
    Serial.printf("   AHT10 %.1f C %.1f %%\n", t, rh);
  } else {
    Serial.println("   AHT10 missing - sending without a reading");
  }

  // 2. Radio on, right channel. No WiFi.begin(): we never join the network.
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(TX_POWER);

  if (!gatewayMacSet()) {
    Serial.println("   GATEWAY_MAC is all zeros - fill in include/class_net.h");
  } else {
    if (channel == NO_CHANNEL || failCount >= MAX_FAILS) {
      Serial.printf("   scanning for \"%s\"...\n", CLASS_SSID);
      channel = findChannel(CLASS_SSID);
      failCount = 0;
    }

    if (channel == NO_CHANNEL) {
      Serial.printf("   \"%s\" not found - check CLASS_SSID\n", CLASS_SSID);
    } else {
      setChannel(channel);
      // 3. Send and wait for the delivery status.
      if (!espNowBegin()) {
        Serial.println("   ESP-NOW init failed");
      } else {
        bool ok = sendReading(sensorOk, t, rh);
        failCount = ok ? 0 : static_cast<uint8_t>(failCount + 1);
        Serial.printf("   ch %u  seq %u  -> %s\n", channel, seq,
                      ok ? "SUCCESS" : "FAIL (no ACK from the gateway)");
        if (!ok) blinkFail();
      }
    }
  }

  if (cause != ESP_SLEEP_WAKEUP_TIMER) {
    Serial.printf("   awake %lu s - LED blue, upload now\n",
                  static_cast<unsigned long>(AWAKE_WINDOW_MS / 1000U));
    waitMs(AWAKE_WINDOW_MS);
  }

  goToSleep();
}

void loop() {
  // never reached: every path in setup() ends in deep sleep
}
