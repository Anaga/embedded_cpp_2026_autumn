#include "receiver.h"

#include <time.h>

#include "config.h"
#include "radio.h"  // from ../common: pins + LoRa settings

// A seq that moves forward by more than this is a node restart, not lost packets.
constexpr uint16_t SEQ_RESTART_JUMP = 1000U;

const char* wakeName(uint8_t w) {
  switch (w) {
    case WAKE_COLD: return "cold";
    case WAKE_TIMER: return "timer";
    case WAKE_MOTION: return "motion";
    case WAKE_BUTTON: return "button";
    default: return "?";
  }
}

// name[] is zero-terminated by the node, but never trust the radio: copy it
// into a buffer that certainly ends in 0.
void copyName(const GuardianMsg& m, char* out) {
  memset(out, 0, sizeof(m.name) + 1U);
  memcpy(out, m.name, sizeof(m.name));
}

static void printMessage(const shared::Latest& l, uint16_t gap) {
  const GuardianMsg& m = l.msg;
  char name[sizeof(m.name) + 1U];
  copyName(m, name);
  Serial.printf("#%-5u %-4s ", static_cast<unsigned>(m.seq), name);
  if ((m.flags & FLAG_AHT_OK) != 0U) {
    Serial.printf("%5.1f C  %5.1f %%RH  ", m.t_c10 / 10.0, m.rh_10 / 10.0);
  } else {
    Serial.print("  AHT10 FAULT       ");
  }
  Serial.printf("batt %6.3f V  wake %-6s awake %4u ms  flags 0x%02X%s%s", m.batt_mV / 1000.0, wakeName(m.wake_cause),
                static_cast<unsigned>(m.awake_ms), static_cast<unsigned>(m.flags),
                (m.flags & FLAG_TAMPER) ? " TAMPER" : "", (m.flags & FLAG_BUTTON) ? " BUTTON" : "");
  Serial.printf("  | RSSI %4d dBm  SNR %5.1f dB  | received %lu, lost %lu%s\n", static_cast<int>(l.rssi), l.snr,
                static_cast<unsigned long>(l.received), static_cast<unsigned long>(l.lost), gap ? "  <- gap" : "");
}

bool Receiver::begin() {
  printRadioSettings();
  if (!radioBegin()) return false;
  Serial.printf("LoRa OK, listening for %u-byte GuardianMsg packets...\n", static_cast<unsigned>(sizeof(GuardianMsg)));
  return true;
}

void Receiver::update(uint32_t now) {
  int size = LoRa.parsePacket();  // 0 = nothing there: returns at once
  if (size > 0) handle(size, now);
}

void Receiver::handle(int size, uint32_t now) {
  int16_t rssi = static_cast<int16_t>(LoRa.packetRssi());
  float snr = LoRa.packetSnr();

  // Three checks before trusting the bytes: right size, right magic byte,
  // right version. Anything else on 434 MHz fails at least one of them.
  if (size != static_cast<int>(sizeof(GuardianMsg))) {
    Serial.printf("?? ignored: %d bytes, expected %u  (RSSI %d dBm)\n", size,
                  static_cast<unsigned>(sizeof(GuardianMsg)), static_cast<int>(rssi));
    while (LoRa.available()) LoRa.read();
    return;
  }
  GuardianMsg msg;
  LoRa.readBytes(reinterpret_cast<uint8_t*>(&msg), sizeof(msg));
  if (msg.magic != MSG_MAGIC || msg.version != MSG_VERSION) {
    Serial.printf("?? ignored: magic 0x%02X version %u  (RSSI %d dBm)\n", static_cast<unsigned>(msg.magic),
                  static_cast<unsigned>(msg.version), static_cast<int>(rssi));
    return;
  }

  // Lost packets = gaps in seq (uint16_t wraps correctly 65535 -> 0).
  // A seq that jumps backwards (or far ahead) means the node restarted.
  uint16_t gap = 0U;
  bool restarted = false;
  if (haveSeq_) {
    uint16_t diff = static_cast<uint16_t>(msg.seq - lastSeq_);
    if (diff == 0U || diff > SEQ_RESTART_JUMP) {
      restarted = true;
    } else {
      gap = static_cast<uint16_t>(diff - 1U);
    }
  }
  haveSeq_ = true;
  lastSeq_ = msg.seq;

  time_t clock = time(nullptr);
  if (restarted) {
    current_.received = 0U;
    current_.lost = 0U;
    Serial.println("-- node restarted, counts reset --");
  }
  current_.lost += gap;
  current_.received++;
  current_.msg = msg;
  current_.rssi = rssi;
  current_.snr = snr;
  current_.atMs = now;
  current_.atTime = clock > static_cast<time_t>(CLOCK_VALID_AFTER) ? clock : 0;  // before NTP the clock says 1970
  current_.have = true;
  shared::publish(current_);  // the bot's copy, under the lock

  printMessage(current_, gap);
  alarms_.onPacket(msg, restarted, now);
  history_.add(clock, msg, (msg.flags & FLAG_AHT_OK) != 0U && alarms_.temperatureTrusted());

  if ((msg.flags & FLAG_TAMPER) != 0U) {
    led_.flash(StatusLed::Flash::MOVEMENT);
  } else if (gap != 0U || (msg.flags & FLAG_AHT_OK) == 0U || (msg.flags & FLAG_MPU_OK) == 0U) {
    led_.flash(StatusLed::Flash::WARN);
  } else {
    led_.flash(StatusLed::Flash::GOOD);
  }
}

void Receiver::printStatus() const {
  if (current_.have) {
    Serial.printf("last packet #%u, %lu s ago, received %lu, lost %lu\n", static_cast<unsigned>(current_.msg.seq),
                  static_cast<unsigned long>((millis() - current_.atMs) / 1000U),
                  static_cast<unsigned long>(current_.received), static_cast<unsigned long>(current_.lost));
  } else {
    Serial.println("no packet yet");
  }
}
