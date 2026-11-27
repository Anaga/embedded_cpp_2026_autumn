#include "alarms.h"

#include <math.h>

#include "config.h"
#include "emoji.h"
#include "format.h"
#include "shared.h"

// ---- The texts, one entry per alarm (same order as enum class Alarm) --------
static const char* const NAME[ALARM_COUNT] = {
    "Battery low",         "Battery CRITICAL",      "Movement",         "Node silent", "Temperature sensor fault",
    "Motion sensor fault", "Battery reading fault", "Condensation risk"};

// What Telegram says: the headline when it starts, the advice line under
// it, and the headline when it clears. Plain words, no codes.
static const char* const START[ALARM_COUNT] = {
    "Battery low",         "Battery CRITICAL",      "Someone is moving your bike!", "No contact with the bike",
    "Temperature sensor fault", "Motion sensor fault", "Battery reading fault", "Condensation risk"};
static const char* const ADVICE[ALARM_COUNT] = {
    "Charge it this week.",
    "It may not start. Charge it today.",
    "Go and check.",
    "The box may have lost power, or something is blocking the radio.",
    "Check the sensor's wires.",
    "Movement can't be detected until it's fixed. Check its wires.",
    "Check the divider's wires: the battery itself can't read this.",
    "Damp under the cover. Air it out on a dry day."};
static const char* const CLEAR[ALARM_COUNT] = {
    "Battery OK again",               "Battery OK again",          "The bike is still again",
    "Contact with the bike is back",  "Temperature sensor works again", "Motion sensor works again",
    "Battery reading works again",    "Humidity back to normal"};

bool Alarms::isSystemFault(Alarm a) {
  return a == Alarm::NODE_LOST || a == Alarm::AHT || a == Alarm::MPU || a == Alarm::BATT_SENSE;
}

const char* Alarms::name(Alarm a) { return NAME[idx(a)]; }

const char* Alarms::icon(Alarm a) {
  if (a == Alarm::TAMPER) return EMO_SIREN;
  if (a == Alarm::DAMP) return EMO_DROP;
  if (isSystemFault(a)) return EMO_YELLOW;
  return EMO_RED;
}

bool Alarms::anySystemFault() const {
  for (uint8_t i = 0U; i < ALARM_COUNT; i++) {
    if (on_[i] && isSystemFault(static_cast<Alarm>(i))) return true;
  }
  return false;
}

// THE one place every alarm goes through. Today it writes to Serial and
// Telegram. Anything new (quiet hours, a buzzer, e-mail) is a change here
// and nowhere else.
void Alarms::notify(Alarm a, bool starting, const String& detail) {
  bool silenced = a == Alarm::TAMPER && shared::muted();

  // Serial: one ASCII line.
  String line = String(NAME[idx(a)]) + (starting ? "" : " - cleared") + (detail.length() ? ": " + detail : "") +
                clockText();
  line.replace(TXT_DEG, "");  // the degree sign isn't ASCII
  Serial.printf(">> %s %s%s\n", starting ? (isSystemFault(a) ? "FAULT" : "ALARM") : "OK   ", line.c_str(),
                silenced ? "  (muted: not sent)" : "");
  if (silenced) return;

  // Telegram (HTML): what happened in bold, the details, then what to do.
  String text = String(starting ? icon(a) : EMO_OK) + " <b>" + (starting ? START[idx(a)] : CLEAR[idx(a)]) + "</b>";
  if (detail.length()) text += ": " + detail;
  text += clockText();
  if (starting) text += String("\n") + ADVICE[idx(a)];
  shared::queueAlert(text.c_str());
}

// Switches an alarm on or off. Calls notify() only when that CHANGES it.
void Alarms::set(Alarm a, bool on, const String& detail) {
  if (on_[idx(a)] == on) return;
  on_[idx(a)] = on;
  notify(a, on, detail);
}

// The battery as ONE state: OK, low or critical. Only one of the two battery
// alarms is ever on, and each change sends exactly one message ("critical"
// alone, not "critical" + "low"). Going back up needs 0.1 V more than the
// limit (hysteresis), so 12.19 / 12.21 V doesn't flicker.
void Alarms::checkBattery(float v) {
  enum class Level : uint8_t { OK, LOW_, CRITICAL };  // LOW is taken: Arduino #defines it
  Level cur = on(Alarm::BATT_CRIT) ? Level::CRITICAL : on(Alarm::BATT_LOW) ? Level::LOW_ : Level::OK;
  Level lvl;
  if (v < BATT_CRIT_V) {
    lvl = Level::CRITICAL;
  } else if (v < BATT_LOW_V) {
    lvl = (cur == Level::CRITICAL && v < BATT_CRIT_V + BATT_HYST_V) ? Level::CRITICAL : Level::LOW_;
  } else {
    lvl = (cur != Level::OK && v < BATT_LOW_V + BATT_HYST_V) ? Level::LOW_ : Level::OK;
  }
  if (lvl == cur) return;

  String d = String(v, 2) + " V (" + String(batteryPercent(v)) + " %)";
  if (lvl == Level::CRITICAL) {           // OK or low -> critical
    on_[idx(Alarm::BATT_LOW)] = false;    // quietly: "critical" says it all
    set(Alarm::BATT_CRIT, true, d);
  } else if (lvl == Level::LOW_ && cur == Level::CRITICAL) {  // better, but not fine yet
    on_[idx(Alarm::BATT_CRIT)] = false;
    on_[idx(Alarm::BATT_LOW)] = true;
    notify(Alarm::BATT_LOW, true, d + ", up from critical");
  } else if (lvl == Level::LOW_) {        // OK -> low
    set(Alarm::BATT_LOW, true, d);
  } else {                                // low or critical -> OK
    set(cur == Level::CRITICAL ? Alarm::BATT_CRIT : Alarm::BATT_LOW, false, d);
  }
}

// Temperature sensor: missing, OR a reading that can't be real. An AHT10
// without power still answers through the I2C pull-ups, with +50 C too much.
void Alarms::checkTemperature(const GuardianMsg& m, bool nodeRestarted) {
  if (nodeRestarted) haveGoodT_ = false;
  bool ok = (m.flags & FLAG_AHT_OK) != 0U;
  String d = "the AHT10 doesn't answer";
  if (ok) {
    float t = m.t_c10 / 10.0f;
    if (t < T_MIN_C || t > T_MAX_C) {  // impossible on its own, even with nothing to compare to
      ok = false;
      d = "impossible " + String(t, 1) + " " TXT_DEG "C";
    } else if (haveGoodT_ && fabsf(t - lastGoodT_) > T_JUMP_MAX_C) {
      ok = false;
      d = "implausible " + String(t, 1) + " " TXT_DEG "C (was " + String(lastGoodT_, 1) + " " TXT_DEG "C)";
    } else {
      lastGoodT_ = t;
      haveGoodT_ = true;
      d = String(t, 1) + " " TXT_DEG "C";
    }
  }
  set(Alarm::AHT, !ok, d);
}

// Condensation risk: only from a reading we believe.
void Alarms::checkHumidity(const GuardianMsg& m) {
  if ((m.flags & FLAG_AHT_OK) == 0U || on(Alarm::AHT)) return;
  float rh = m.rh_10 / 10.0f;
  String d = String(rh, 0) + " % humidity";
  if (rh > DAMP_ON_RH) {
    set(Alarm::DAMP, true, d);
  } else if (rh < DAMP_OFF_RH) {
    set(Alarm::DAMP, false, d);
  }
}

void Alarms::onPacket(const GuardianMsg& m, bool nodeRestarted, uint32_t now) {
  if (on(Alarm::NODE_LOST)) {
    set(Alarm::NODE_LOST, false, "packets again after " + String((now - lastPacketMs_) / 1000U) + " s");
  }
  havePacket_ = true;
  lastPacketMs_ = now;

  // Battery. Under 3 V or over 16 V means the divider's wiring is wrong:
  // that's OUR fault, not the battery's. Then the battery alarms go off
  // quietly, so an old "critical" doesn't stay on with nothing behind it.
  float v = m.batt_mV / 1000.0f;
  if (v < BATT_SENSE_MIN_V || v > BATT_SENSE_MAX_V) {
    set(Alarm::BATT_SENSE, true, "impossible " + String(v, 2) + " V, check the divider's wires");
    on_[idx(Alarm::BATT_CRIT)] = false;
    on_[idx(Alarm::BATT_LOW)] = false;
  } else {
    set(Alarm::BATT_SENSE, false, String(v, 2) + " V");
    checkBattery(v);
  }

  // Movement: on with each TAMPER packet; update() clears it after a quiet time.
  if ((m.flags & FLAG_TAMPER) != 0U) {
    lastTamperMs_ = now;
    set(Alarm::TAMPER, true);
  }

  checkTemperature(m, nodeRestarted);

  bool mpuOk = (m.flags & FLAG_MPU_OK) != 0U;
  set(Alarm::MPU, !mpuOk, mpuOk ? "" : "the MPU-6050 doesn't answer");

  checkHumidity(m);
}

void Alarms::update(uint32_t now) {
  if (now - lastCheckMs_ < TIME_ALARM_CHECK_MS) return;
  lastCheckMs_ = now;
  if (on(Alarm::TAMPER) && now - lastTamperMs_ >= TAMPER_QUIET_MS) {
    set(Alarm::TAMPER, false, "no movement for " + String(TAMPER_QUIET_MS / 1000U) + " s");
  }
  if (havePacket_ && !on(Alarm::NODE_LOST)) {
    uint32_t silent = now - lastPacketMs_;
    if (silent >= NODE_LOST_MS) set(Alarm::NODE_LOST, true, "nothing heard for " + String(silent / 1000U) + " s");
  }
}
