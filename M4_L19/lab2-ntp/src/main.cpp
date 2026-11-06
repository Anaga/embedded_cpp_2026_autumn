// Lesson 19 - Lab 2: NTP once, then deep sleep
//
// Cold start: WiFi on -> NTP -> WiFi off -> deep sleep.
// Every wake: print the local time from the RTC, no WiFi at all.
// Button wake while the time is still unknown: try NTP again.
//
// The two numbers to write down:
//   awake time on a cold start (WiFi + NTP)
//   awake time on a timer wake (no WiFi)
// "waiting" is time spent only for the USB monitor and the upload
// window; a battery device would not spend it. millis() starts after
// the bootloader, so the real wake is a little longer than printed.
//
// LED, as in lab 1: red blink = just woke up, blue = awake, green = asleep
// (held LOW through deep sleep). The blink counts as "waiting".
//
// Wiring (as in K3 and session 18):
//   Button A  GPIO 0 -> button -> GND (INPUT_PULLUP, active low)
//   RGB LED   R 5, G 6, B 7, common anode to 3V3, LOW = on

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy include/secrets.example.h to include/secrets.h and fill it in"
#endif

// ---------------------------------------------------------------- pins
constexpr uint8_t PIN_BUTTON = 0;   // only GPIO 0..5 can wake the C3 from deep sleep
constexpr uint8_t PIN_LED_R  = 5;
constexpr uint8_t PIN_LED_G  = 6;
constexpr uint8_t PIN_LED_B  = 7;
constexpr uint8_t LED_PINS[] = {PIN_LED_R, PIN_LED_G, PIN_LED_B};

// ---------------------------------------------------------------- timing
constexpr uint64_t US_PER_S           = 1000000ULL;
constexpr uint64_t SLEEP_S            = 60;
constexpr uint32_t AWAKE_WINDOW_MS    = 30000;  // blue: time to start an upload
constexpr uint32_t SERIAL_WAIT_MS     = 2000;   // USB comes back after a wake
constexpr uint32_t RED_BLINK_MS       = 300;
constexpr uint32_t RELEASE_TIMEOUT_MS = 5000;
constexpr uint32_t RELEASE_POLL_MS    = 10;
constexpr uint32_t WIFI_TIMEOUT_MS    = 15000;
constexpr uint32_t WIFI_POLL_MS       = 20;
constexpr uint32_t NTP_TIMEOUT_MS     = 10000;

// ---------------------------------------------------------------- time
// Tallinn: EET (UTC+2), EEST (UTC+3) from the last Sunday of March 03:00
// to the last Sunday of October 04:00.
constexpr const char *TZ_TALLINN = "EET-2EEST,M3.5.0/3,M10.5.0/4";
constexpr const char *NTP_SERVER = "pool.ntp.org";

// Many Super Mini boards connect only with reduced TX power (session 18).
constexpr wifi_power_t TX_POWER = WIFI_POWER_8_5dBm;

// ---------------------------------------------------------------- RTC memory
RTC_DATA_ATTR static uint32_t bootCount = 0;
RTC_DATA_ATTR static bool     timeValid = false;   // NTP has answered once

static uint32_t waitedMs = 0;       // this wake: red blink + USB wait + upload window

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

// ---------------------------------------------------------------- NTP
// Blocking on purpose: a sleeping program does its job in setup() and
// goes back to sleep. Returns true when the RTC now holds the right time.
static bool syncTime() {
  uint32_t t0 = millis();
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(TX_POWER);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < WIFI_TIMEOUT_MS) {
    delay(WIFI_POLL_MS);
  }
  uint32_t wifiMs = millis() - t0;

  bool ok = false;
  uint32_t ntpMs = 0;
  if (WiFi.status() == WL_CONNECTED) {
    uint32_t t1 = millis();
    configTzTime(TZ_TALLINN, NTP_SERVER);
    struct tm now;
    ok = getLocalTime(&now, NTP_TIMEOUT_MS);   // waits for the first answer
    ntpMs = millis() - t1;
    Serial.printf("   WiFi %lu ms, NTP %lu ms%s\n", static_cast<unsigned long>(wifiMs),
                  static_cast<unsigned long>(ntpMs), ok ? "" : " - no answer");
  } else {
    Serial.printf("   WiFi: no connection to \"%s\" after %lu ms\n", WIFI_SSID,
                  static_cast<unsigned long>(wifiMs));
  }

  WiFi.disconnect(true);              // radio off before sleeping
  WiFi.mode(WIFI_OFF);
  return ok;
}

// ---------------------------------------------------------------- print
static const char *wakeText(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER: return "timer";
    case ESP_SLEEP_WAKEUP_GPIO:  return "button";
    default:                     return "cold";
  }
}

// "2026-11-06 19:12:05" or "time unknown"
static void formatNow(char *out, size_t size) {
  struct tm now;
  if (timeValid && getLocalTime(&now, 0)) {
    strftime(out, size, "%Y-%m-%d %H:%M:%S", &now);
  } else {
    snprintf(out, size, "time unknown       ");
  }
}

// ---------------------------------------------------------------- sleep
static void waitForRelease() {
  uint32_t start = millis();
  while (digitalRead(PIN_BUTTON) == LOW && millis() - start < RELEASE_TIMEOUT_MS) {
    delay(RELEASE_POLL_MS);
  }
}

static void goToSleep(esp_sleep_wakeup_cause_t cause) {
  waitForRelease();

  char text[24];
  formatNow(text, sizeof(text));
  uint32_t awakeMs = millis() - waitedMs;
  Serial.printf("%s  #%lu  %-6s  awake %lu ms (+%lu waiting)\n", text,
                static_cast<unsigned long>(bootCount), wakeText(cause),
                static_cast<unsigned long>(awakeMs), static_cast<unsigned long>(waitedMs));
  Serial.flush();

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

  // The TZ setting lives in RAM and is lost in deep sleep: set it on
  // every wake, or localtime() prints UTC.
  setenv("TZ", TZ_TALLINN, 1);
  tzset();

  Serial.begin(115200);
  waitMs(SERIAL_WAIT_MS);             // only for the USB monitor

  if (cause == ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.println();
    Serial.println("Lesson 19 - Lab 2 - NTP once, then sleep");
    timeValid = false;                // a reset may keep the RTC, a power cycle does not
  }

  bool personThere = (cause != ESP_SLEEP_WAKEUP_TIMER);
  if (personThere && !timeValid) {    // cold start, or a button press while unsynced
    timeValid = syncTime();
  }

  if (personThere) {
    Serial.printf("   awake %lu s - LED blue, upload now\n",
                  static_cast<unsigned long>(AWAKE_WINDOW_MS / 1000U));
    waitMs(AWAKE_WINDOW_MS);
  }

  goToSleep(cause);
}

void loop() {
  // never reached: every path in setup() ends in deep sleep
}
