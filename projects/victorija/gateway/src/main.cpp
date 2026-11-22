// Winter Guardian gateway (ESP32-S3). Receives node packets over LoRa,
// raises alarms, keeps a history, and answers a Telegram bot.
//
// Two tasks: loop() (core 1) handles only the radio; botTask (core 0) does
// the slow HTTPS calls. Shared data is guarded by a mutex; alerts go from
// loop() to the bot through a queue.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <time.h>

#include "msg.h"
#include "radio.h"
#include "secrets.h"  // not in git, see secrets.h.example

constexpr uint8_t RGB_PIN = 38;
constexpr uint8_t BRIGHT = 20;
constexpr uint32_t FLASH_MS = 150;

constexpr uint32_t NODE_INTERVAL_S = 20;  // must match the node's SLEEP_S
constexpr uint32_t BOT_POLL_MS = 2000;

// ---- Shared between loop() and botTask -------------------------------------
struct Latest {
  bool have = false;
  GuardianMsg msg;
  int rssi = 0;
  float snr = 0;
  uint32_t atMs = 0;
  time_t atTime = 0;       // 0 until NTP sync
  uint32_t received = 0;
  uint32_t lost = 0;
};
Latest latest;
SemaphoreHandle_t latestLock;

struct Alert {
  char text[160];
};
QueueHandle_t alertQueue;

void queueAlert(const char* text) {
  Alert a;
  strncpy(a.text, text, sizeof(a.text) - 1);
  a.text[sizeof(a.text) - 1] = 0;
  if (xQueueSend(alertQueue, &a, 0) != pdTRUE) Serial.println("alert queue full - dropped");
}

// ---- Alarms ------------------------------------------------------------------
enum Alarm { AL_BATT_LOW, AL_BATT_CRIT, AL_TAMPER, AL_NODE_LOST, AL_AHT, AL_MPU, AL_COUNT };
const char* const ALARM_NAME[AL_COUNT] = {"Battery low",  "Battery CRITICAL",         "Movement",
                                          "Node silent",  "Temperature sensor fault", "Motion sensor fault"};

// Written only by loop(); botTask only reads. Single-byte access: no lock.
volatile bool alarmOn[AL_COUNT] = {};

constexpr float BATT_LOW_V = 12.2f;   // lead-acid at rest: ~50 %
constexpr float BATT_CRIT_V = 11.8f;  // ~10 %
constexpr float BATT_HYST_V = 0.1f;   // clears 0.1 V above the limit
constexpr uint32_t NODE_LOST_MS = 3 * NODE_INTERVAL_S * 1000;
constexpr uint32_t TAMPER_QUIET_MS = 120000;
constexpr float T_JUMP_MAX_C = 10.0f;  // larger jump between packets = sensor fault

// " at 14:03", or "" before NTP sync.
String clockText() {
  time_t now = time(nullptr);
  if (now < 1700000000) return "";
  char buf[12];
  struct tm t;
  localtime_r(&now, &t);
  strftime(buf, sizeof buf, " at %H:%M", &t);
  return buf;
}

// The single output path for alarms: once on start, once on clear.
void notify(Alarm a, bool starting, const String& detail) {
  String body = ALARM_NAME[a];
  if (!starting) body += " - cleared";
  if (detail.length()) body += ": " + detail;
  body += clockText();
  String serialText = body;
  serialText.replace("\u00B0", "");  // drop the degree sign: serial output is ASCII only
  Serial.printf(">> %s %s\n", starting ? "ALARM" : "OK   ", serialText.c_str());
  String text = (starting ? (a == AL_TAMPER ? "🚨 " : "🔴 ") : "✅ ") + body;  // Telegram keeps the emoji
  queueAlert(text.c_str());
}

// Calls notify() only on a change.
void setAlarm(Alarm a, bool on, const String& detail = "") {
  if (alarmOn[a] == on) return;
  alarmOn[a] = on;
  notify(a, on, detail);
}

bool haveGoodT = false;  // last plausible temperature, for the jump check
float lastGoodT = 0;
uint32_t lastTamperMs = 0;

void checkPacketAlarms(const GuardianMsg& m, bool restarted, uint32_t silentMs) {
  if (alarmOn[AL_NODE_LOST]) setAlarm(AL_NODE_LOST, false, "packets again after " + String(silentMs / 1000) + " s");

  // 0 = not measured
  if (m.batt_mV > 0) {
    float v = m.batt_mV / 1000.0f;
    String d = String(v, 2) + " V";
    if (v < BATT_CRIT_V) setAlarm(AL_BATT_CRIT, true, d);
    else if (v >= BATT_CRIT_V + BATT_HYST_V) setAlarm(AL_BATT_CRIT, false, d);
    if (v < BATT_LOW_V) setAlarm(AL_BATT_LOW, true, d);
    else if (v >= BATT_LOW_V + BATT_HYST_V) setAlarm(AL_BATT_LOW, false, d);
  }

  // Cleared by checkTimeAlarms().
  if (m.flags & FLAG_TAMPER) {
    lastTamperMs = millis();
    setAlarm(AL_TAMPER, true, "the bike moved");
  }

  // An AHT10 that lost VCC still answers through the I2C pull-ups, with
  // readings ~50 C too high: treat a big jump as a fault.
  if (restarted) haveGoodT = false;
  bool ahtOk = m.flags & FLAG_AHT_OK;
  String d = "the AHT10 doesn't answer";
  if (ahtOk) {
    float t = m.t_c10 / 10.0f;
    if (haveGoodT && fabsf(t - lastGoodT) > T_JUMP_MAX_C) {
      ahtOk = false;
      d = "implausible " + String(t, 1) + " °C (was " + String(lastGoodT, 1) + " °C)";
    } else {
      lastGoodT = t;
      haveGoodT = true;
      d = String(t, 1) + " °C";
    }
  }
  setAlarm(AL_AHT, !ahtOk, d);

  bool mpuOk = m.flags & FLAG_MPU_OK;
  setAlarm(AL_MPU, !mpuOk, mpuOk ? "answers again" : "the MPU-6050 doesn't answer");
}

// ---- History: ring buffer, 2000 x 12 bytes = 24 KB ---------------------------
struct HistEntry {
  uint32_t t;        // Unix time
  int16_t t_c10;     // NO_TEMP if the sensor was faulty
  uint16_t rh_10;
  uint16_t batt_mV;  // 0 = not measured
  uint8_t flags;
};
constexpr int16_t NO_TEMP = INT16_MIN;
constexpr int HIST_SIZE = 2000;
HistEntry hist[HIST_SIZE];
int histNext = 0;
int histCount = 0;
// Guarded by latestLock.

void historyAdd(time_t now, const GuardianMsg& m, bool tempOk) {
  if (now < 1700000000) return;  // no NTP time yet
  HistEntry e;
  e.t = (uint32_t)now;
  e.t_c10 = tempOk ? m.t_c10 : NO_TEMP;
  e.rh_10 = m.rh_10;
  e.batt_mV = m.batt_mV;
  e.flags = m.flags;
  xSemaphoreTake(latestLock, portMAX_DELAY);
  hist[histNext] = e;
  histNext = (histNext + 1) % HIST_SIZE;
  if (histCount < HIST_SIZE) histCount++;
  xSemaphoreGive(latestLock);
}

// /history [hours]: default 3 h, about 12 rows.
String historyText(int hours) {
  if (hours < 1) hours = 3;
  if (hours > 48) hours = 48;
  time_t now = time(nullptr);
  if (now < 1700000000) return "No clock yet (NTP), so no history.";

  // Round step sizes: 5, 10, 15, 30 min, 1, 2, 3, 4 h.
  const uint32_t steps[] = {300, 600, 900, 1800, 3600, 7200, 10800, 14400};
  uint32_t step = steps[7];
  for (uint32_t st : steps)
    if (st * 12 >= (uint32_t)hours * 3600) { step = st; break; }
  int nb = (hours * 3600 + step - 1) / step + 1;  // +1: the current, partial step
  if (nb > 25) nb = 25;
  uint32_t first = ((uint32_t)now / step - (nb - 1)) * step;

  struct Bucket {
    int n = 0, tN = 0, rhN = 0, bN = 0, moves = 0;
    int32_t tSum = 0;
    uint32_t rhSum = 0, bSum = 0;
  };
  Bucket b[25];

  xSemaphoreTake(latestLock, portMAX_DELAY);
  for (int i = 0; i < histCount; i++) {
    const HistEntry& e = hist[i];
    if (e.t < first) continue;
    int k = (e.t - first) / step;
    if (k < 0 || k >= nb) continue;
    b[k].n++;
    if (e.t_c10 != NO_TEMP) {
      b[k].tSum += e.t_c10;
      b[k].tN++;
      b[k].rhSum += e.rh_10;
      b[k].rhN++;
    }
    if (e.batt_mV > 0) {
      b[k].bSum += e.batt_mV;
      b[k].bN++;
    }
    if (e.flags & FLAG_TAMPER) b[k].moves++;
  }
  int stored = histCount;
  xSemaphoreGive(latestLock);

  String s = "History, last " + String(hours) + " h, " + String(step / 60) + " min steps (" + String(stored) +
             " packets stored)\n```\n";
  s += "time   temp  hum   batt  pkts\n";
  for (int k = 0; k < nb; k++) {
    time_t t0 = first + k * step;
    struct tm tm;
    localtime_r(&t0, &tm);
    char line[64];
    char temp[8] = "   --", hum[8] = " --", batt[8] = "   --";
    if (b[k].tN) snprintf(temp, sizeof temp, "%5.1f", b[k].tSum / 10.0 / b[k].tN);
    if (b[k].rhN) snprintf(hum, sizeof hum, "%3.0f", b[k].rhSum / 10.0 / b[k].rhN);
    if (b[k].bN) snprintf(batt, sizeof batt, "%5.2f", b[k].bSum / 1000.0 / b[k].bN);
    snprintf(line, sizeof line, "%02d:%02d %s  %s%%  %s  %3d%s\n", tm.tm_hour, tm.tm_min, temp, hum, batt, b[k].n,
             b[k].moves ? "  MOVED" : "");
    s += line;
  }
  s += "```";
  return s;
}

// ---- Radio side (loop, core 1) ----------------------------------------------
bool haveSeq = false;
uint16_t lastSeq = 0;
uint32_t flashOffAt = 0;

void flash(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_PIN, r, g, b);
  flashOffAt = millis() + FLASH_MS;
}

const char* wakeName(uint8_t w) {
  switch (w) {
    case WAKE_COLD: return "cold";
    case WAKE_TIMER: return "timer";
    case WAKE_MOTION: return "motion";
    case WAKE_BUTTON: return "button";
    default: return "?";
  }
}

// Never trust the radio: guarantee a terminating 0.
void copyName(const GuardianMsg& m, char* out /* size 10 */) {
  memset(out, 0, sizeof(m.name) + 1);
  memcpy(out, m.name, sizeof(m.name));
}

void printMessage(const GuardianMsg& m, int rssi, float snr, uint16_t gap, uint32_t received, uint32_t lost) {
  char name[sizeof(m.name) + 1];
  copyName(m, name);
  Serial.printf("#%-5u %-4s ", m.seq, name);
  if (m.flags & FLAG_AHT_OK) {
    Serial.printf("%5.1f C  %5.1f %%RH  ", m.t_c10 / 10.0, m.rh_10 / 10.0);
  } else {
    Serial.print("  AHT10 FAULT       ");
  }
  Serial.printf("batt %6.3f V  wake %-6s awake %4u ms  flags 0x%02X%s%s",
                m.batt_mV / 1000.0, wakeName(m.wake_cause), m.awake_ms, m.flags,
                (m.flags & FLAG_TAMPER) ? " TAMPER" : "", (m.flags & FLAG_BUTTON) ? " BUTTON" : "");
  Serial.printf("  | RSSI %4d dBm  SNR %5.1f dB  | received %lu, lost %lu%s\n", rssi, snr,
                (unsigned long)received, (unsigned long)lost, gap ? "  <- gap" : "");
}

void handlePacket(int size) {
  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();

  // Size, magic and version must all match.
  if (size != (int)sizeof(GuardianMsg)) {
    Serial.printf("?? ignored: %d bytes, expected %u  (RSSI %d dBm)\n", size, (unsigned)sizeof(GuardianMsg), rssi);
    while (LoRa.available()) LoRa.read();
    return;
  }
  GuardianMsg msg;
  LoRa.readBytes((uint8_t*)&msg, sizeof(msg));
  if (msg.magic != MSG_MAGIC || msg.version != MSG_VERSION) {
    Serial.printf("?? ignored: magic 0x%02X version %u  (RSSI %d dBm)\n", msg.magic, msg.version, rssi);
    return;
  }

  // Gaps in seq = lost packets (uint16_t wraps). Backwards = node restarted.
  uint16_t gap = 0;
  bool restarted = false;
  if (haveSeq) {
    uint16_t diff = (uint16_t)(msg.seq - lastSeq);
    if (diff == 0 || diff > 1000) {
      restarted = true;
    } else {
      gap = diff - 1;
    }
  }
  haveSeq = true;
  lastSeq = msg.seq;

  uint32_t silentMs = latest.have ? millis() - latest.atMs : 0;  // sole writer: no lock to read

  time_t now = time(nullptr);
  xSemaphoreTake(latestLock, portMAX_DELAY);
  if (restarted) {
    latest.received = 0;
    latest.lost = 0;
  }
  latest.lost += gap;
  latest.received++;
  latest.msg = msg;
  latest.rssi = rssi;
  latest.snr = snr;
  latest.atMs = millis();
  latest.atTime = (now > 1700000000) ? now : 0;
  latest.have = true;
  uint32_t received = latest.received, lost = latest.lost;
  xSemaphoreGive(latestLock);

  if (restarted) Serial.println("-- node restarted, counts reset --");
  printMessage(msg, rssi, snr, gap, received, lost);

  checkPacketAlarms(msg, restarted, silentMs);
  historyAdd(now, msg, (msg.flags & FLAG_AHT_OK) && !alarmOn[AL_AHT]);

  if (msg.flags & FLAG_TAMPER) {
    flash(0, 0, BRIGHT);  // blue
  } else if (gap || !(msg.flags & FLAG_AHT_OK)) {
    flash(BRIGHT, 0, 0);  // red
  } else {
    flash(0, BRIGHT, 0);  // green
  }
}

// Time-based alarms, checked once a second.
void checkTimeAlarms() {
  static uint32_t last = 0;
  if (millis() - last < 1000) return;
  last = millis();
  if (alarmOn[AL_TAMPER] && millis() - lastTamperMs >= TAMPER_QUIET_MS) {
    setAlarm(AL_TAMPER, false, "no movement for 2 min");
  }
  if (latest.have && !alarmOn[AL_NODE_LOST]) {
    uint32_t silent = millis() - latest.atMs;
    if (silent >= NODE_LOST_MS) setAlarm(AL_NODE_LOST, true, "nothing heard for " + String(silent / 1000) + " s");
  }
}

// Between flashes: dim blue = movement, dim red = other alarm, dim green = OK.
void updateLed() {
  static uint32_t shown = 0xFFFFFFFF;
  if (flashOffAt != 0) {
    shown = 0xFFFFFFFF;  // redraw after the flash
    return;
  }
  constexpr uint8_t DIM = 3;
  bool other = false;
  for (int a = 0; a < AL_COUNT; a++)
    if (a != AL_TAMPER && alarmOn[a]) other = true;
  uint8_t r = 0, g = 0, b = 0;
  if (alarmOn[AL_TAMPER]) b = DIM;
  else if (other) r = DIM;
  else g = DIM;
  uint32_t colour = (r << 16) | (g << 8) | b;
  if (colour != shown) {
    neopixelWrite(RGB_PIN, r, g, b);
    shown = colour;
  }
}

// ---- Telegram side (botTask, core 0) ----------------------------------------
WiFiClientSecure tlsClient;
UniversalTelegramBot bot(BOT_TOKEN, tlsClient);

String statusText() {
  Latest l;
  xSemaphoreTake(latestLock, portMAX_DELAY);
  l = latest;  // copy under the lock, format without it
  xSemaphoreGive(latestLock);

  if (!l.have) return "No packet from the node yet since the gateway started.";

  char name[sizeof(l.msg.name) + 1];
  copyName(l.msg, name);
  uint32_t ago = (millis() - l.atMs) / 1000;

  String s;
  s.reserve(300);
  s += "Node " + String(name) + "\n";
  if (l.msg.flags & FLAG_AHT_OK) {
    s += "Temperature " + String(l.msg.t_c10 / 10.0, 1) + " °C\n";
    s += "Humidity " + String(l.msg.rh_10 / 10.0, 1) + " %\n";
  } else {
    s += "⚠️ Temperature sensor fault\n";
  }
  if (l.msg.batt_mV > 0) {
    s += "Battery " + String(l.msg.batt_mV / 1000.0, 2) + " V\n";
  } else {
    s += "Battery: not connected\n";
  }
  s += "Last packet " + String(ago) + " s ago";
  if (l.atTime) {
    char hhmmss[12];
    struct tm t;
    localtime_r(&l.atTime, &t);
    strftime(hhmmss, sizeof hhmmss, "%H:%M:%S", &t);
    s += String(" (") + hhmmss + ")";
  }
  s += "\n";
  if (ago > 3 * NODE_INTERVAL_S) s += "⚠️ Nothing heard for " + String(ago) + " s\n";
  s += "Woke by " + String(wakeName(l.msg.wake_cause)) + ", previous wake took " + String(l.msg.awake_ms) + " ms\n";
  s += "#" + String(l.msg.seq) + ", RSSI " + String(l.rssi) + " dBm, SNR " + String(l.snr, 1) + " dB\n";
  s += "Received " + String(l.received) + ", lost " + String(l.lost) + "\n";

  String alarms;
  for (int a = 0; a < AL_COUNT; a++)
    if (alarmOn[a]) alarms += String("\n") + (a == AL_TAMPER ? "🚨 " : "🔴 ") + ALARM_NAME[a];
  s += alarms.length() ? "Alarms:" + alarms : String("No alarms ✅");
  return s;
}

void handleBotMessages(int count) {
  for (int i = 0; i < count; i++) {
    const String& chat = bot.messages[i].chat_id;
    const String& text = bot.messages[i].text;

    // Anyone can message a public bot: answer only CHAT_ID.
    if (chat != CHAT_ID) {
      Serial.printf("bot: message from chat id %s (\"%s\") ignored. If that's you, put this id into "
                    "CHAT_ID in secrets.h\n", chat.c_str(), text.c_str());
      continue;
    }
    Serial.printf("bot: \"%s\"\n", text.c_str());

    if (text.startsWith("/status")) {
      bot.sendMessage(chat, statusText(), "");
    } else if (text.startsWith("/history")) {
      int hours = text.substring(8).toInt();  // 0 -> default
      bot.sendMessage(chat, historyText(hours), "Markdown");  // ``` block = monospace
    } else {
      bot.sendMessage(chat,
                      "Winter Guardian gateway.\n/status - the latest reading and active alarms\n"
                      "/history - the last 3 hours\n/history 12 - the last 12 hours (up to 48)",
                      "");
    }
  }
}

bool connectWifi(uint32_t timeoutMs) {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.printf("WiFi: connecting to \"%s\"...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeoutMs) vTaskDelay(pdMS_TO_TICKS(250));
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi: FAILED (wrong name/password? antenna on? 2.4 GHz network?) - retrying in 30 s");
    return false;
  }
  Serial.printf("WiFi: connected, IP %s, signal %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  return true;
}

void botTask(void*) {
  if (strlen(WIFI_SSID) == 0 || strlen(BOT_TOKEN) == 0) {
    Serial.println("bot: secrets.h is not filled in - Telegram is off, the radio part still works");
    vTaskDelete(nullptr);
  }

  while (!connectWifi(20000)) vTaskDelay(pdMS_TO_TICKS(30000));

  // NTP first: TLS certificate validity needs the real date.
  configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.google.com");  // Europe/Tallinn
  struct tm t;
  if (getLocalTime(&t, 10000)) {
    Serial.printf("NTP: %02d.%02d.%04d %02d:%02d:%02d\n", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour,
                  t.tm_min, t.tm_sec);
  } else {
    Serial.println("NTP: no time yet (keeps trying in the background)");
  }

  // Verify api.telegram.org against the root CA shipped with the library.
  tlsClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  if (strlen(CHAT_ID) > 0) {
    bot.sendMessage(CHAT_ID, "Gateway online ✅  Send /status", "");
    Serial.println("bot: ready, sent \"Gateway online\"");
  } else {
    Serial.println("bot: ready. CHAT_ID is empty: send the bot any message to find out your chat id");
  }

  while (true) {
    if (!connectWifi(20000)) {
      vTaskDelay(pdMS_TO_TICKS(30000));
      continue;
    }
    // Alerts first.
    Alert a;
    while (xQueueReceive(alertQueue, &a, 0) == pdTRUE) {
      bool ok = strlen(CHAT_ID) > 0 && bot.sendMessage(CHAT_ID, a.text, "");
      Serial.printf("bot: alert %s\n", ok ? "sent" : "FAILED");
    }

    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n > 0) {
      handleBotMessages(n);
      n = bot.getUpdates(bot.last_message_received + 1);
    }
    vTaskDelay(pdMS_TO_TICKS(BOT_POLL_MS));
  }
}

// ---- setup / loop ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  neopixelWrite(RGB_PIN, 0, 0, 0);
  Serial.println("\n=== Winter Guardian - gateway (S3) ===");
  printRadioSettings();

  latestLock = xSemaphoreCreateMutex();
  alertQueue = xQueueCreate(8, sizeof(Alert));

  if (!radioBegin()) {
    Serial.println("LoRa NOT FOUND - check the wiring");
    neopixelWrite(RGB_PIN, BRIGHT, 0, 0);  // steady red = error
    while (true) delay(1000);
  }
  Serial.printf("LoRa OK, listening for %u-byte GuardianMsg packets...\n", (unsigned)sizeof(GuardianMsg));

  // 12 KB stack for TLS. Core 0 with the WiFi driver; loop() keeps core 1.
  xTaskCreatePinnedToCore(botTask, "bot", 12288, nullptr, 1, nullptr, 0);
}

void loop() {
  int size = LoRa.parsePacket();
  if (size > 0) handlePacket(size);

  if (flashOffAt != 0 && (long)(millis() - flashOffAt) >= 0) {
    neopixelWrite(RGB_PIN, 0, 0, 0);
    flashOffAt = 0;
  }
  checkTimeAlarms();
  updateLed();
  delay(1);  // feed the idle task / watchdog
}
