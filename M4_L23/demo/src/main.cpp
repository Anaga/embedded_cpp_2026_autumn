// Lesson 23 demo - a Telegram bot on the ESP32-C3
//
// The board polls the Telegram servers (getUpdates over HTTPS) once a second
// and answers commands from your phone:
//   /start, /help     help text, keyboard, and this chat gets the alerts
//   /status           temperature, humidity, LED, WiFi, uptime
//   /led on, /led off green channel of the RGB LED (GPIO 6). Not the blue
//                     on-board LED: that one is GPIO 8 = I2C SDA (AHT10)
//   /alert            show the alert threshold
//   /alert 26.5       set the alert threshold in C
// Push alert: when the temperature reaches the threshold the board writes
// first, once. "Back to normal" comes when it drops 0.5 C below it.
//
// Serial: every incoming message with its chat id, every reply, and one
// status line every 10 s including how long the last poll blocked loop().

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <esp_mac.h>
#include <Adafruit_AHTX0.h>
#include <UniversalTelegramBot.h>

#include "board.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy include/secrets.example.h to include/secrets.h and fill it in"
#endif

// ---------------------------------------------------------------- timing
constexpr uint32_t SENSOR_PERIOD_MS = 2000U;
constexpr uint32_t AHT_RETRY_MS     = 5000U;
constexpr uint32_t BOT_POLL_MS      = 1000U;   // pause between two polls
constexpr uint32_t PRINT_PERIOD_MS  = 10000U;
constexpr uint8_t  AHT10_ADDRESS    = 0x38U;

// ---------------------------------------------------------------- alert
constexpr float ALERT_DEFAULT_C    = 28.0F;
constexpr float ALERT_HYSTERESIS_C = 0.5F;    // same idea as session 15
constexpr float ALERT_MIN_C        = -20.0F;
constexpr float ALERT_MAX_C        = 60.0F;

// Reply keyboard shown after /start: each inner array is one row of buttons.
// A button press simply sends its text as a message.
static const char KEYBOARD_JSON[] = "[[\"/status\"],[\"/led on\",\"/led off\"]]";

// The "/" menu in the Telegram app (setMyCommands). Text only, no logic.
static const char COMMANDS_JSON[] =
    "[{\"command\":\"status\",\"description\":\"Temperature and humidity\"},"
    "{\"command\":\"led\",\"description\":\"/led on or /led off\"},"
    "{\"command\":\"alert\",\"description\":\"Show or set the alert threshold\"},"
    "{\"command\":\"help\",\"description\":\"What this bot can do\"}]";

// ---------------------------------------------------------------- state
static char deviceId[16];                // "esp-xxxx" from the last two MAC bytes
static bool ledOn = false;

struct Reading {
  bool  valid;
  float t;
  float rh;
};
static Reading reading = {false, NAN, NAN};
static bool    newReading = false;       // set by ahtService, used by alertService

static Adafruit_AHTX0 aht;
static bool     ahtPresent = false;
static uint32_t lastAhtTry = 0U;
static uint32_t lastSample = 0U;

static WiFiClientSecure     tls;
static UniversalTelegramBot bot(BOT_TOKEN, tls);
static bool     botReady   = false;      // getMe() answered once
static uint32_t lastPoll   = 0U;
static uint32_t pollLastMs = 0U;         // how long the last poll blocked loop()
static uint32_t pollMaxMs  = 0U;

enum class TempState { NORMAL, HOT };
static TempState tempState = TempState::NORMAL;
static float     alertC    = ALERT_DEFAULT_C;
static String    alertChat = ALERT_CHAT_ID;   // empty = nobody gets alerts yet

static uint32_t lastPrint = 0U;

// ---------------------------------------------------------------- helpers
static void ledBegin() {
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
}

static void ledSet(bool on) {
  ledOn = on;
  digitalWrite(PIN_LED_R, HIGH);         // common anode: LOW = on
  digitalWrite(PIN_LED_G, on ? LOW : HIGH);
  digitalWrite(PIN_LED_B, HIGH);
}

static void makeDeviceId() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(deviceId, sizeof(deviceId), "esp-%02x%02x", mac[4], mac[5]);
}

// Messages from Telegram may contain any characters (Cyrillic, emoji).
// Our Serial log stays ASCII and on one line: a line break is printed
// as " | ", anything else outside ASCII as '?'.
static void printAscii(const String &s) {
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\n') {
      Serial.print(" | ");
      continue;
    }
    bool ascii = (static_cast<uint8_t>(c) >= 0x20U) && (static_cast<uint8_t>(c) < 0x7FU);
    Serial.print(ascii ? c : '?');
  }
}

static bool send(const String &chatId, const String &text) {
  Serial.print("  -> ");
  printAscii(text);
  Serial.println();
  bool ok = bot.sendMessage(chatId, text, "");
  if (!ok) {
    Serial.println("  -> send FAILED");
  }
  return ok;
}

static String statusText() {
  char buf[200];
  char sensor[48];
  if (reading.valid) {
    snprintf(sensor, sizeof(sensor), "%.1f C, %.1f %% RH", reading.t, reading.rh);
  } else if (ahtPresent) {
    snprintf(sensor, sizeof(sensor), "first reading...");
  } else {
    snprintf(sensor, sizeof(sensor), "sensor missing");
  }
  snprintf(buf, sizeof(buf),
           "%s\n%s\nGreen LED %s\nAlert at %.1f C (%s)\nWiFi %d dBm, up %lu min",
           deviceId, sensor, ledOn ? "on" : "off", alertC,
           (tempState == TempState::HOT) ? "HOT" : "normal",
           WiFi.RSSI(), static_cast<unsigned long>(millis() / 60000UL));
  return String(buf);
}

static String helpText() {
  return String(deviceId) + " here. Commands:\n"
         "/status - temperature and humidity\n"
         "/led on, /led off - green channel of the RGB LED\n"
         "/alert - show the alert threshold\n"
         "/alert 26.5 - set it\n"
         "This chat now gets the alerts.";
}

// ---------------------------------------------------------------- commands
// "/LED@my_c3_bot  on " -> cmd "/led", arg "on".
// The "@botname" part appears when the bot is used in a group chat.
static void splitCommand(const String &text, String &cmd, String &arg) {
  String s = text;
  s.trim();
  int space = s.indexOf(' ');
  cmd = (space < 0) ? s : s.substring(0, space);
  arg = (space < 0) ? String("") : s.substring(space + 1);
  arg.trim();
  int at = cmd.indexOf('@');
  if (at >= 0) {
    cmd = cmd.substring(0, at);
  }
  cmd.toLowerCase();
  arg.toLowerCase();
}

static void handleAlert(const String &chatId, const String &arg) {
  char buf[96];
  if (arg.length() == 0U) {
    snprintf(buf, sizeof(buf), "Alert at %.1f C, back to normal below %.1f C",
             alertC, alertC - ALERT_HYSTERESIS_C);
    send(chatId, buf);
    return;
  }

  char *end = nullptr;
  float v = strtof(arg.c_str(), &end);
  bool numberOk = (end != arg.c_str()) && (*end == '\0');
  if (!numberOk || (v < ALERT_MIN_C) || (v > ALERT_MAX_C)) {
    snprintf(buf, sizeof(buf), "Expected a number from %.0f to %.0f, e.g. /alert 26.5",
             ALERT_MIN_C, ALERT_MAX_C);
    send(chatId, buf);
    return;
  }

  alertC    = v;
  alertChat = chatId;
  tempState = TempState::NORMAL;   // so a new crossing can alert again
  snprintf(buf, sizeof(buf), "Alert set to %.1f C. Alerts go to this chat.", alertC);
  send(chatId, buf);
}

static void handleMessage(const telegramMessage &m) {
  Serial.print("chat ");
  Serial.print(m.chat_id);
  Serial.print(" <- ");
  printAscii(m.text);
  Serial.println();

  String cmd;
  String arg;
  splitCommand(m.text, cmd, arg);

  if ((cmd == "/start") || (cmd == "/help")) {
    alertChat = m.chat_id;
    Serial.println("  (this chat now gets the alerts)");
    bot.sendMessageWithReplyKeyboard(m.chat_id, helpText(), "", KEYBOARD_JSON, true);
  } else if (cmd == "/status") {
    send(m.chat_id, statusText());
  } else if ((cmd == "/led") && (arg == "on")) {
    ledSet(true);
    send(m.chat_id, "Green LED on");
  } else if ((cmd == "/led") && (arg == "off")) {
    ledSet(false);
    send(m.chat_id, "Green LED off");
  } else if (cmd == "/led") {
    send(m.chat_id, "Use /led on or /led off");
  } else if (cmd == "/alert") {
    handleAlert(m.chat_id, arg);
  } else {
    send(m.chat_id, "Unknown command. Try /help");
  }
}

// ---------------------------------------------------------------- bot
// Runs once, the first time WiFi is up: checks the token and fills the
// "/" menu in the app.
static void botBegin() {
  if (!bot.getMe()) {
    Serial.println("Bot: getMe failed - wrong BOT_TOKEN, or no internet");
    return;
  }
  botReady = true;
  Serial.print("Bot: @");
  Serial.print(bot.userName);
  Serial.println(" is online - open it in Telegram and send /start");
  bot.setMyCommands(COMMANDS_JSON);
  if (alertChat.length() > 0U) {
    send(alertChat, String(deviceId) + " online");
  }
}

static void botService() {
  if (!WiFi.isConnected()) return;
  if (millis() - lastPoll < BOT_POLL_MS) return;

  uint32_t t0 = millis();
  if (!botReady) {
    botBegin();
  } else {
    // offset = last update id + 1 tells Telegram "I have seen everything
    // up to here" - without it the same messages come back on every poll.
    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n > 0) {
      for (int i = 0; i < n; i++) {
        handleMessage(bot.messages[i]);
      }
      n = bot.getUpdates(bot.last_message_received + 1);
    }
  }
  pollLastMs = millis() - t0;
  if (pollLastMs > pollMaxMs) {
    pollMaxMs = pollLastMs;
  }
  lastPoll = millis();               // 1 s pause AFTER the poll, not between starts
}

// ---------------------------------------------------------------- alert
// Two states and a band: one message per crossing, not one every 2 s.
// The state changes only after the message went out, so a failed send is
// retried with the next reading.
static void alertService() {
  if (!newReading) return;
  newReading = false;
  if (!reading.valid || (alertChat.length() == 0U) || !botReady) return;

  char buf[96];
  switch (tempState) {
    case TempState::NORMAL:
      if (reading.t >= alertC) {
        snprintf(buf, sizeof(buf), "ALERT %s: %.1f C, threshold %.1f C",
                 deviceId, reading.t, alertC);
        if (send(alertChat, buf)) {
          tempState = TempState::HOT;
        }
      }
      break;
    case TempState::HOT:
      if (reading.t < alertC - ALERT_HYSTERESIS_C) {
        snprintf(buf, sizeof(buf), "Back to normal %s: %.1f C", deviceId, reading.t);
        if (send(alertChat, buf)) {
          tempState = TempState::NORMAL;
        }
      }
      break;
  }
}

// ---------------------------------------------------------------- AHT10
static bool ahtResponds() {
  // Check the address first: the library hangs on a missing sensor
  // (session 15).
  Wire.beginTransmission(AHT10_ADDRESS);
  return Wire.endTransmission() == 0;
}

static void ahtService(uint32_t now) {
  if (!ahtPresent) {
    if (now - lastAhtTry < AHT_RETRY_MS) return;
    lastAhtTry = now;
    if (ahtResponds() && aht.begin(&Wire)) {
      ahtPresent = true;
      Serial.println("AHT10: found");
    }
    return;
  }

  if (now - lastSample < SENSOR_PERIOD_MS) return;
  lastSample = now;

  sensors_event_t h;
  sensors_event_t t;
  if (ahtResponds() && aht.getEvent(&h, &t)) {
    reading = {true, t.temperature, h.relative_humidity};
    newReading = true;
  } else {
    ahtPresent = false;
    lastAhtTry = now;
    reading = {false, NAN, NAN};
    Serial.println("AHT10: missing");
  }
}

// ---------------------------------------------------------------- WiFi
static void wifiBegin() {
  WiFi.setHostname(deviceId);        // core 2.x: before WiFi.mode()
  WiFi.mode(WIFI_STA);
  if (LIMIT_TX_POWER) {
    WiFi.setTxPower(TX_POWER);
  }
  Serial.printf("WiFi: connecting to \"%s\"\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);  // the driver retries by itself
}

// ---------------------------------------------------------------- print
static void printStatus() {
  char t[32];
  if (reading.valid) {
    snprintf(t, sizeof(t), "%.1f C %.1f %%", reading.t, reading.rh);
  } else {
    snprintf(t, sizeof(t), "%s", ahtPresent ? "first reading..." : "sensor missing");
  }
  char wifi[40];
  if (WiFi.isConnected()) {
    snprintf(wifi, sizeof(wifi), "%s %d dBm", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    snprintf(wifi, sizeof(wifi), "not connected");
  }
  Serial.printf("%s | WiFi %s | %s | LED %s | alert %.1f %s | poll %lu ms (max %lu) | heap %lu\n",
                deviceId, wifi, t, ledOn ? "on" : "off", alertC,
                (tempState == TempState::HOT) ? "HOT" : "ok",
                static_cast<unsigned long>(pollLastMs),
                static_cast<unsigned long>(pollMaxMs),
                static_cast<unsigned long>(ESP.getFreeHeap()));
  if (alertChat.length() == 0U) {
    Serial.println("  no alert chat yet - send /start to the bot");
  }
}

// ---------------------------------------------------------------- main
void setup() {
  Serial.begin(115200);
  delay(2000);                       // USB CDC needs a moment after reset
  Serial.println();
  Serial.println("Lesson 23 - Telegram bot");

  ledBegin();
  ledSet(false);
  makeDeviceId();
  Serial.printf("Device %s\n", deviceId);

  Wire.begin(PIN_SDA, PIN_SCL);
  lastAhtTry = millis() - AHT_RETRY_MS;   // try the sensor right away

  // Encrypted, but the server certificate is NOT checked: fine in class,
  // see the slides for what that means and what a real device does.
  tls.setInsecure();

  wifiBegin();
}

void loop() {
  uint32_t now = millis();

  ahtService(now);
  alertService();
  botService();                      // blocks for the HTTPS round trip

  if (now - lastPrint >= PRINT_PERIOD_MS) {
    lastPrint = now;
    printStatus();
  }
}
