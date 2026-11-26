#include "history.h"

constexpr uint32_t S_PER_MIN = 60U;
constexpr uint32_t S_PER_H = 3600U;

void History::add(time_t now, const GuardianMsg& m, bool tempOk) {
  if (now < static_cast<time_t>(CLOCK_VALID_AFTER)) return;  // no clock yet (NTP not synced): skip
  Entry e;
  e.t = static_cast<uint32_t>(now);
  e.t_c10 = tempOk ? m.t_c10 : NO_TEMP;  // a faulty reading stays out of the averages
  e.rh_10 = m.rh_10;
  e.batt_mV = m.batt_mV;
  e.flags = m.flags;
  xSemaphoreTake(lock_, portMAX_DELAY);
  buf_[next_] = e;
  next_ = static_cast<uint16_t>((next_ + 1U) % HIST_SIZE);  // wrap around: that's the "ring"
  if (count_ < HIST_SIZE) count_++;
  xSemaphoreGive(lock_);
}

String History::text(uint8_t hours) {
  if (hours < 1U) hours = HIST_DEFAULT_H;
  if (hours > HIST_MAX_H) hours = HIST_MAX_H;
  time_t now = time(nullptr);
  if (now < static_cast<time_t>(CLOCK_VALID_AFTER)) return "No clock yet (NTP), so no history.";

  // Pick a "round" step so the times read nicely: 5, 10, 15, 30 min, 1, 2, 3, 4 h.
  static constexpr uint32_t STEPS[] = {300U, 600U, 900U, 1800U, 3600U, 7200U, 10800U, 14400U};
  uint32_t span = static_cast<uint32_t>(hours) * S_PER_H;
  uint32_t step = STEPS[sizeof(STEPS) / sizeof(STEPS[0]) - 1U];
  for (uint32_t st : STEPS) {
    if (st * HIST_ROWS >= span) {
      step = st;
      break;
    }
  }
  uint32_t rows = (span + step - 1U) / step + 1U;  // +1: the step that's running now
  if (rows > HIST_MAX_ROWS) rows = HIST_MAX_ROWS;
  uint32_t first = (static_cast<uint32_t>(now) / step - (rows - 1U)) * step;  // start of the oldest step

  struct Bucket {
    uint16_t n = 0U, tN = 0U, rhN = 0U, bN = 0U, moves = 0U;
    int32_t tSum = 0;
    uint32_t rhSum = 0U, bSum = 0U;
  };
  Bucket b[HIST_MAX_ROWS];

  xSemaphoreTake(lock_, portMAX_DELAY);
  for (uint16_t i = 0U; i < count_; i++) {
    const Entry& e = buf_[i];
    if (e.t < first) continue;
    uint32_t k = (e.t - first) / step;
    if (k >= rows) continue;
    b[k].n++;
    if (e.t_c10 != NO_TEMP) {
      b[k].tSum += e.t_c10;
      b[k].tN++;
      b[k].rhSum += e.rh_10;
      b[k].rhN++;
    }
    if (e.batt_mV > 0U) {
      b[k].bSum += e.batt_mV;
      b[k].bN++;
    }
    if ((e.flags & FLAG_TAMPER) != 0U) b[k].moves++;
  }
  uint16_t stored = count_;
  xSemaphoreGive(lock_);

  String s = "History, last " + String(hours) + " h, " + String(step / S_PER_MIN) + " min steps (" +
             String(stored) + " packets stored)\n```\n";
  s += "time   temp  hum   batt  pkts\n";
  for (uint32_t k = 0U; k < rows; k++) {
    time_t t0 = static_cast<time_t>(first + k * step);
    struct tm tm;
    localtime_r(&t0, &tm);
    char line[64];
    char temp[8] = "   --", hum[8] = " --", batt[8] = "   --";
    if (b[k].tN) snprintf(temp, sizeof temp, "%5.1f", b[k].tSum / 10.0 / b[k].tN);
    if (b[k].rhN) snprintf(hum, sizeof hum, "%3.0f", b[k].rhSum / 10.0 / b[k].rhN);
    if (b[k].bN) snprintf(batt, sizeof batt, "%5.2f", b[k].bSum / 1000.0 / b[k].bN);
    snprintf(line, sizeof line, "%02d:%02d %s  %s%%  %s  %3u%s\n", tm.tm_hour, tm.tm_min, temp, hum, batt,
             static_cast<unsigned>(b[k].n), b[k].moves ? "  MOVED" : "");
    s += line;
  }
  s += "```";
  return s;
}
