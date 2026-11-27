#include "format.h"

#include "config.h"
#include "emoji.h"

constexpr uint32_t S_PER_MIN = 60U;
constexpr uint32_t S_PER_H = 3600U;
constexpr uint8_t BAR_SEGMENTS = 10U;

String clockText() {
  time_t now = time(nullptr);
  if (now < static_cast<time_t>(CLOCK_VALID_AFTER)) return "";
  char buf[12];
  struct tm t;
  localtime_r(&now, &t);
  strftime(buf, sizeof buf, " at %H:%M", &t);
  return buf;
}

String clockAt(time_t t) {
  if (t < static_cast<time_t>(CLOCK_VALID_AFTER)) return "";
  char buf[8];
  struct tm tm;
  localtime_r(&t, &tm);
  strftime(buf, sizeof buf, "%H:%M", &tm);
  return buf;
}

String agoText(uint32_t s) {
  if (s < S_PER_MIN) return String(s) + " s ago";
  if (s < S_PER_H) return String(s / S_PER_MIN) + " min ago";
  return String(s / S_PER_H) + " h " + String((s % S_PER_H) / S_PER_MIN) + " min ago";
}

String durationText(uint32_t ms) {
  uint32_t s = ms / 1000U;
  if (s < S_PER_MIN) return String(s) + " s";
  return String(s / S_PER_MIN) + " min " + String(s % S_PER_MIN) + " s";
}

String html(String s) {
  s.replace("&", "&amp;");
  s.replace("<", "&lt;");
  s.replace(">", "&gt;");
  return s;
}

String ascii(const String& s) {
  String out;
  for (uint32_t i = 0U; i < s.length(); i++) {
    char c = s[i];
    if (c >= ' ' && c <= '~') {
      out += c;
    } else if ((static_cast<uint8_t>(c) & 0xC0U) != 0x80U) {  // a UTF-8 start byte, not a continuation byte
      out += '?';
    }
  }
  return out;
}

uint8_t batteryPercent(float v) {
  // The table: resting voltage -> % charged.
  static constexpr float V[] = {11.6f, 11.8f, 12.0f, 12.2f, 12.4f, 12.6f, 12.7f};
  static constexpr uint8_t P[] = {0U, 10U, 30U, 50U, 70U, 90U, 100U};
  constexpr uint8_t N = sizeof(V) / sizeof(V[0]);
  if (v <= V[0]) return 0U;
  for (uint8_t i = 1U; i < N; i++) {
    if (v < V[i]) {  // between two table points: a straight line between them
      float part = (v - V[i - 1U]) / (V[i] - V[i - 1U]);
      return static_cast<uint8_t>(P[i - 1U] + part * (P[i] - P[i - 1U]));
    }
  }
  return 100U;
}

String batteryBar(uint8_t percent) {
  uint8_t full = static_cast<uint8_t>((percent + 5U) / 10U);  // rounded to the nearest segment
  String s;
  for (uint8_t i = 0U; i < BAR_SEGMENTS; i++) s += i < full ? TXT_BAR_FULL : TXT_BAR_EMPTY;
  return s;
}
