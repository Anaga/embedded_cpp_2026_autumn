#include "net.h"

#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <time.h>

#include "config.h"
#include "emoji.h"
#include "format.h"
#include "receiver.h"    // wakeName(), copyName()
#include "secrets.h"     // bot token + chat id (include/, not in git)
#include "shared.h"
#include "wifi_store.h"  // lesson 24: the WiFi list, saved in NVS

namespace net {

static const Alarms* alarms = nullptr;
static History* history = nullptr;

static WiFiClientSecure tlsClient;
static UniversalTelegramBot bot(BOT_TOKEN, tlsClient);
static WiFiMulti wifiMulti;
static uint8_t networkCount = 0U;

static bool everOnline = false;
static uint32_t offlineSinceMs = 0U;

// SNR limits for the words on the status card.
constexpr float SNR_GOOD_DB = 5.0f;
constexpr float SNR_FAIR_DB = -5.0f;

// ---- Buttons ------------------------------------------------------------------
// A reply keyboard: big buttons under the chat. Pressing one sends its text.
static const char* const BTN_STATUS = EMO_PIN " Status";
static const char* const BTN_HISTORY = EMO_CHART " History";
static const char* const BTN_MUTE = EMO_MUTED " Mute 1 h";
static const char* const BTN_UNMUTE = EMO_BELL " Unmute";

static const char* const WELCOME =
    EMO_BIKE " <b>Winter Guardian is watching your bike.</b>\n"
    "I message you when the battery runs low, when someone moves the bike, or when something is wrong with me.\n"
    "Tap " EMO_PIN " Status any time.";

// The "/" menu in Telegram: the commands with one line each.
static const char* const COMMANDS =
    "[{\"command\":\"status\",\"description\":\"How is the bike?\"},"
    "{\"command\":\"history\",\"description\":\"The last 3 hours (/history 12 = 12 h)\"},"
    "{\"command\":\"mute\",\"description\":\"No movement alerts for 1 hour\"},"
    "{\"command\":\"unmute\",\"description\":\"Movement alerts back on\"}]";

static String keyboard() {
  return String("[[\"") + BTN_STATUS + "\",\"" + BTN_HISTORY + "\"],[\"" + (shared::muted() ? BTN_UNMUTE : BTN_MUTE) +
         "\"]]";
}

// Every answer carries the keyboard, so the buttons are always there and
// the Mute / Unmute button always shows the right one.
static bool reply(const String& text, const String& parseMode = "HTML") {
  return bot.sendMessageWithReplyKeyboard(CHAT_ID, text, parseMode, keyboard(), true);
}

// ---- The status card (/status) --------------------------------------------
// The verdict first (the same priority as the LED), then the numbers in words.
static String statusText() {
  shared::Latest l = shared::read();
  if (!l.have) {
    return EMO_WAIT " <b>Waiting for the first report from the bike</b>\nThe node sends every " +
           String(NODE_INTERVAL_S) + " s.";
  }

  char raw[sizeof(l.msg.name) + 1U];
  copyName(l.msg, raw);
  String bike = html(raw);
  bike.toUpperCase();
  uint32_t ago = (millis() - l.atMs) / 1000U;
  const Alarms& a = *alarms;

  // 1. The verdict: the one most important thing.
  Alarm headline = Alarm::COUNT;  // which alarm the first line already says (COUNT = none)
  String s;
  s.reserve(600U);
  if (a.on(Alarm::TAMPER)) {
    s += EMO_SIREN " <b>Someone is moving the " + bike + "!</b>\n";
    headline = Alarm::TAMPER;
  } else if (a.on(Alarm::BATT_CRIT)) {
    s += EMO_RED " <b>Battery critical: charge it today</b>\n";
    headline = Alarm::BATT_CRIT;
  } else if (a.on(Alarm::BATT_LOW)) {
    s += EMO_RED " <b>Battery low: charge it this week</b>\n";
    headline = Alarm::BATT_LOW;
  } else if (a.on(Alarm::DAMP)) {
    s += EMO_DROP " <b>Damp under the cover: air it out</b>\n";
    headline = Alarm::DAMP;
  } else if (a.anySystemFault()) {
    s += EMO_YELLOW " <b>The guardian itself needs a check</b>\n";
  } else {
    s += EMO_OK " <b>" + bike + " is fine</b>\n";
  }
  // Anything else that's on, one line each.
  for (uint8_t i = 0U; i < ALARM_COUNT; i++) {
    Alarm al = static_cast<Alarm>(i);
    if (a.on(al) && al != headline) s += String(Alarms::icon(al)) + " " + Alarms::name(al) + "\n";
  }
  s += "\n";

  // 2. The numbers, in words.
  float v = l.msg.batt_mV / 1000.0f;
  if (a.on(Alarm::BATT_SENSE) || v < BATT_SENSE_MIN_V) {
    s += EMO_BATTERY " Battery: no reading (check the wires)\n";
  } else if (v >= BATT_CHARGING_V) {
    s += EMO_BATTERY " Battery " + String(v, 2) + " V  " EMO_ZAP " charging\n";
  } else {
    uint8_t p = batteryPercent(v);
    s += EMO_BATTERY " Battery " + String(v, 2) + " V  " + batteryBar(p) + " " + String(p) + " %\n";
  }
  if ((l.msg.flags & FLAG_AHT_OK) != 0U && a.temperatureTrusted()) {
    s += EMO_THERMO " " + String(l.msg.t_c10 / 10.0, 1) + " " TXT_DEG "C    " EMO_DROP " " +
         String(l.msg.rh_10 / 10.0, 0) + " %\n";
  } else {
    s += EMO_THERMO " Temperature: sensor fault\n";
  }
  if (ago > 3U * NODE_INTERVAL_S) {
    s += EMO_NO_SIGNAL " No report for " + durationText(ago * 1000U) + "\n";
  } else {
    const char* signal = l.snr > SNR_GOOD_DB ? "good" : l.snr > SNR_FAIR_DB ? "fair" : "weak";
    s += EMO_SIGNAL " Signal " + String(signal) + " " TXT_DOT " last report " + agoText(ago) + "\n";
  }
  if (shared::muted()) {
    String until = clockAt(time(nullptr) + static_cast<int32_t>(shared::muteUntilMs - millis()) / 1000);
    s += EMO_MUTED " Movement alerts muted" + (until.length() ? " until " + until : String("")) + "\n";
  }

  // 3. Details, small print.
  s += "\n<i>#" + String(l.msg.seq) + " " TXT_DOT " RSSI " + String(l.rssi) + " dBm " TXT_DOT " SNR " +
       String(l.snr, 1) + " dB " TXT_DOT " lost " + String(l.lost) + " of " + String(l.received + l.lost) +
       " " TXT_DOT " woke by " + wakeName(l.msg.wake_cause) + " " TXT_DOT " WiFi \"" + html(WiFi.SSID()) + "\"</i>";
  return s;
}

// ---- Incoming messages ------------------------------------------------------
static void handleMessages(int count) {
  for (int i = 0; i < count; i++) {
    const String& chat = bot.messages[i].chat_id;
    const String& text = bot.messages[i].text;

    // Only answer YOU. Anyone can find a bot by its name and message it;
    // the chat id check makes the gateway ignore them.
    if (chat != CHAT_ID) {
      Serial.printf("bot: message from chat id %s (\"%s\") ignored. If that's you, put this id into "
                    "CHAT_ID in secrets.h\n", chat.c_str(), ascii(text).c_str());
      continue;
    }
    Serial.printf("bot: \"%s\"\n", ascii(text).c_str());

    if (text == BTN_STATUS || text.startsWith("/status")) {
      reply(statusText());
    } else if (text == BTN_HISTORY || text.startsWith("/history")) {
      // "/history 12" -> 12 hours; the button or a bare /history -> the default
      long hours = text.startsWith("/history") ? text.substring(8).toInt() : 0;
      if (hours < 0) hours = 0;  // 0 = the default
      if (hours > HIST_MAX_H) hours = HIST_MAX_H;
      reply(history->text(static_cast<uint8_t>(hours)), "Markdown");  // Markdown: the ``` block keeps the columns
    } else if (text == BTN_MUTE || text.startsWith("/mute")) {
      uint32_t until = millis() + MUTE_MS;
      shared::muteUntilMs = until == 0U ? 1U : until;  // 0 means "not muted"
      String at = clockAt(time(nullptr) + MUTE_MS / 1000U);
      reply(EMO_MUTED " <b>Movement alerts off" + (at.length() ? " until " + at : String(" for 1 h")) +
            ".</b>\nBattery and fault alerts still come. Tap " EMO_BELL " Unmute to switch them back on.");
    } else if (text == BTN_UNMUTE || text.startsWith("/unmute")) {
      shared::muteUntilMs = 0U;
      reply(EMO_BELL " <b>Movement alerts are back on.</b>");
    } else {  // /start, /help, anything else
      reply(WELCOME);
    }
  }
}

// ---- WiFi: several networks, saved in NVS (lesson 24) ----------------------
// Type  wifi add "<ssid>" <password>  in the serial monitor, then  reboot.
// WiFiMulti scans and joins the strongest saved network it can see.
static void wifiBegin() {
#ifdef WIFI_SSID
  // One-time move: the old single network from secrets.h goes into the list.
  wifi_store::Network nets[wifi_store::MAX_NETWORKS];
  if (strlen(WIFI_SSID) > 0U && wifi_store::load(nets, wifi_store::MAX_NETWORKS) == 0U) {
    Serial.printf("WiFi: saving \"%s\" from secrets.h into the list: %s\n", WIFI_SSID,
                  wifi_store::resultText(wifi_store::add(WIFI_SSID, WIFI_PASSWORD)));
  }
#endif
  wifi_store::Network list[wifi_store::MAX_NETWORKS];
  networkCount = wifi_store::load(list, wifi_store::MAX_NETWORKS);
  for (uint8_t i = 0U; i < networkCount; i++) wifiMulti.addAP(list[i].ssid, list[i].pass);
  Serial.printf("WiFi: %u saved network(s)\n", static_cast<unsigned>(networkCount));
  if (networkCount == 0U) Serial.println("WiFi: type  wifi add \"<ssid>\" <password>  and then  reboot");
  WiFi.mode(WIFI_STA);
}

static bool connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.println("WiFi: scanning for saved networks...");
  if (wifiMulti.run(WIFI_CONNECT_MS) != WL_CONNECTED) {
    Serial.println("WiFi: no saved network found (2.4 GHz only; iPhone hotspot: Maximize Compatibility) - retrying in 5 s");
    return false;
  }
  Serial.printf("WiFi: connected to \"%s\", IP %s, signal %d dBm\n", WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str(), static_cast<int>(WiFi.RSSI()));
  return true;
}

void printStatus() {
  if (WiFi.isConnected()) {
    Serial.printf("WiFi: connected to \"%s\", %d dBm\n", WiFi.SSID().c_str(), static_cast<int>(WiFi.RSSI()));
  } else {
    Serial.println("WiFi: not connected");
  }
  Serial.printf("Telegram: %s, %u alert(s) waiting\n", shared::online ? "online" : "OFFLINE",
                static_cast<unsigned>(shared::alertsWaiting()));
}

// ---- Online / offline -------------------------------------------------------
static void markOffline(const char* why) {
  if (!shared::online) return;
  shared::online = false;
  offlineSinceMs = millis();
  Serial.printf("net: OFFLINE (%s) - alerts are held until it's back\n", why);
}

// The first message after (re)connecting. It also proves Telegram is
// reachable: only if it goes through do we count as online.
static bool sayOnline() {
  if (strlen(CHAT_ID) == 0U) {
    shared::online = true;
    return true;
  }
  String text;
  if (!everOnline) {
    text = WELCOME;
  } else {
    uint8_t held = shared::alertsWaiting();
    text = EMO_ONLINE " Back online after " + durationText(millis() - offlineSinceMs) + " (WiFi \"" + WiFi.SSID() +
           "\"). ";
    text += held ? String(held) + " alert(s) from while I was offline:" : String("Nothing happened meanwhile.");
  }
  if (!reply(text, everOnline ? "" : "HTML")) return false;
  Serial.printf("net: online, WiFi \"%s\"\n", WiFi.SSID().c_str());
  shared::online = true;
  everOnline = true;
  return true;
}

// Alerts first: they're the urgent part. Peek, send, and take it out of the
// queue only once it went through: a failed send is not lost.
static void sendAlerts() {
  shared::Alert a;
  while (strlen(CHAT_ID) > 0U && shared::peekAlert(a)) {
    if (!bot.sendMessage(CHAT_ID, a.text, "HTML")) {
      Serial.println("bot: alert send FAILED - kept for later");
      markOffline("send failed");
      return;
    }
    shared::dropAlert();
    Serial.println("bot: alert sent");
  }
}

static void startClock() {
  // NTP: the real clock time. HTTPS needs it too: a certificate is only
  // valid between two dates, and without NTP the ESP32 thinks it's 1970.
  configTzTime(TIME_ZONE, "pool.ntp.org", "time.google.com");
  struct tm t;
  if (getLocalTime(&t, NTP_WAIT_MS)) {
    Serial.printf("NTP: %02d.%02d.%04d %02d:%02d:%02d\n", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour,
                  t.tm_min, t.tm_sec);
  } else {
    Serial.println("NTP: no time yet (keeps trying in the background)");
  }
}

// Inside a task, vTaskDelay() is fine: it waits only for THIS task, and
// loop() on the other core carries on.
static void netTask(void*) {
  if (networkCount == 0U || strlen(BOT_TOKEN) == 0U) {
    Serial.println("bot: no WiFi saved or no BOT_TOKEN - Telegram is off, the radio part still works");
    vTaskDelete(nullptr);
  }

  while (!connectWifi()) vTaskDelay(pdMS_TO_TICKS(NET_RETRY_MS));
  startClock();
  // Check that we really talk to api.telegram.org: the library ships the
  // root certificate Telegram's server certificate is signed by.
  tlsClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  if (strlen(CHAT_ID) == 0U) Serial.println("bot: CHAT_ID is empty: send the bot any message to find out your chat id");
  bot.setMyCommands(COMMANDS);

  for (;;) {
    if (!connectWifi()) {
      markOffline("no WiFi");
      vTaskDelay(pdMS_TO_TICKS(NET_RETRY_MS));
      continue;
    }
    if (!shared::online && !sayOnline()) {  // WiFi is there, but is Telegram?
      Serial.println("net: WiFi OK but Telegram not reachable - retrying in 5 s");
      vTaskDelay(pdMS_TO_TICKS(NET_RETRY_MS));
      continue;
    }
    sendAlerts();
    if (!shared::online) continue;

    if (shared::muteUntilMs != 0U && !shared::muted()) {  // the hour is over
      shared::muteUntilMs = 0U;
      reply(EMO_BELL " <b>Movement alerts are back on</b> (the hour is over).");
    }

    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n > 0) {
      handleMessages(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }
    vTaskDelay(pdMS_TO_TICKS(BOT_POLL_MS));
  }
}

void begin(const Alarms& a, History& h) {
  alarms = &a;
  history = &h;
  wifiBegin();
  // Core 0 = next to the WiFi driver; loop() keeps core 1 to itself.
  xTaskCreatePinnedToCore(netTask, "net", NET_STACK_BYTES, nullptr, 1, nullptr, 0);
}

}  // namespace net
