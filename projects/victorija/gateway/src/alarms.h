// The alarms: each one is either ON or OFF.
//
// Every change goes through ONE function, notify(): one message when an
// alarm starts, one when it clears, never repeated while it stays on.
//
// Two kinds:
//   bike alarms    the guardian is doing its job: battery, movement, damp
//   system faults  the guardian itself has a problem: node silent, a
//                  sensor, the battery reading (yellow on the LED and in Telegram)
//
// loop() owns it: onPacket() for every packet, update(now) every pass.
// The net task only reads on() for the status card.
#pragma once
#include <Arduino.h>

#include "msg.h"  // from ../common

enum class Alarm : uint8_t { BATT_LOW, BATT_CRIT, TAMPER, NODE_LOST, AHT, MPU, BATT_SENSE, DAMP, COUNT };
constexpr uint8_t ALARM_COUNT = static_cast<uint8_t>(Alarm::COUNT);

class Alarms {
 public:
  void begin() {}
  void onPacket(const GuardianMsg& m, bool nodeRestarted, uint32_t now);  // every packet that passed the checks
  void update(uint32_t now);  // the alarms that depend on time passing, not on a packet

  bool on(Alarm a) const { return on_[idx(a)]; }
  bool anySystemFault() const;
  bool temperatureTrusted() const { return !on(Alarm::AHT); }

  static bool isSystemFault(Alarm a);
  static const char* name(Alarm a);  // short, for Serial and the status card
  static const char* icon(Alarm a);  // the Telegram emoji while it's on

 private:
  static constexpr uint8_t idx(Alarm a) { return static_cast<uint8_t>(a); }
  void set(Alarm a, bool on, const String& detail = "");
  void notify(Alarm a, bool starting, const String& detail);
  void checkBattery(float v);
  void checkTemperature(const GuardianMsg& m, bool nodeRestarted);
  void checkHumidity(const GuardianMsg& m);

  // Written only by loop(); the net task reads them. A bool is read and
  // written in one step, so it can't be "half-written": no lock.
  volatile bool on_[ALARM_COUNT] = {};

  bool haveGoodT_ = false;  // the last believable temperature, for the jump check
  float lastGoodT_ = 0.0f;
  uint32_t lastTamperMs_ = 0U;
  bool havePacket_ = false;
  uint32_t lastPacketMs_ = 0U;
  uint32_t lastCheckMs_ = 0U;
};
