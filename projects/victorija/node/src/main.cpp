// Winter Guardian - the NODE (ESP32-C3 Super Mini, under the seat).
//
// Everything happens in setup();
// loop() is never reached:
//
//   wake -> why? -> read the AHT10 -> send one GuardianMsg -> radio to sleep
//        -> deep sleep 20 s -> (the chip restarts from the top of setup())
//
// DEEP SLEEP = almost everything in the C3 is switched off: the CPUs, the
// normal RAM, WiFi/BLE, USB. Only the RTC part keeps running: a timer that
// wakes the chip, and 8 KB of "RTC memory" that keeps its contents. Waking
// up is like a reset: the program starts again from setup(), and ordinary
// variables are back at their start values. Anything that must survive
// (the packet counter) lives in RTC memory: RTC_DATA_ATTR.
//
// Battery voltage: through a 100k/10k divider into the ADC (battery.h).
//
// Movement wakes the node. The C3 has two ways to wake up:
//   the RTC timer (every 20 s), or GPIO 1 going HIGH. GPIO 1 is the
//   MPU-6050's INT pin: the MPU watches for motion by itself while the C3
//   sleeps, and raises INT when the board moves. Then the node sends at
//   once, with the TAMPER flag. At most one TAMPER per TAMPER_COOLDOWN_S: while
//   that "cooldown" runs, the motion wake is switched off, so a bike being
//   pushed around doesn't wake the node 50 times a minute.

#include <Wire.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

#include "aht10.h"
#include "battery.h"
#include "mpu6050.h"
#include "msg.h"    // from ../common
#include "radio.h"  // from ../common

constexpr uint8_t PIN_SDA = 8U;  // also the blue LED: it flickers with I2C traffic
constexpr uint8_t PIN_SCL = 9U;  // also the BOOT button
constexpr uint8_t PIN_INT = 1U;  // MPU-6050 INT: one of the C3 pins (0-5) that can wake it from deep sleep

// ---- Motion alarm --------------------------------------------------------
// Threshold: smaller = more sensitive. Tune it on the bench: a tap on the
// table next to the board should trigger, the table just standing there not.
constexpr uint8_t MOTION_THRESHOLD = 10U;
constexpr uint8_t MOTION_DURATION = 1U;
constexpr uint32_t TAMPER_COOLDOWN_S = 5U;  // DEMO: at most one TAMPER per 5 s (real: 120). Keep it shorter than the gateway's TAMPER_QUIET_MS

// ---- Battery (battery.h) ----------------------------------------------------
constexpr uint8_t PIN_BATT = 3U;
// Two-point calibration: real = CAL_GAIN * measured + CAL_OFFSET_MV.
// Resistors are only 1-5 % accurate, so the divider ratio is a few % off.
// Measure at 12.0 V and 14.4 V with the multimeter, then fix these two.
// 1.0 and 0 = uncalibrated. Recalibrate on every new board (R1).
// The breadboard node (22.11.2026) needed gain 0.9860, offset 200.7 mV.
// The soldered node (26.11.2026): reads 12.005 V at 12.00 V and
// 14.42 V at 14.40 V uncalibrated, inside +-0.03 V, so it stays 1.0 / 0.
constexpr float CAL_GAIN = 1.0f;
constexpr float CAL_OFFSET_MV = 0.0f;

constexpr uint32_t SLEEP_S = 20U;  // the DEMO interval (the real one will be 10 min)
constexpr const char* NODE_NAME = "fz8";
constexpr uint32_t I2C_HZ = 100000U;
constexpr uint32_t BANNER_WAIT_MS = 2000U;  // cold start only: time to open the serial monitor
constexpr uint64_t US_PER_S = 1000000ULL;

// ---- RTC memory: survives deep sleep, cleared by power-off or RST ----------
RTC_DATA_ATTR uint16_t seq = 0U;
RTC_DATA_ATTR uint16_t lastAwakeMs = 0U;  // how long the PREVIOUS wake took (power budget)
RTC_DATA_ATTR int64_t lastTamperS = -1000000;  // when the last TAMPER was sent (long ago at first)

Aht10 aht;
Mpu6050 mpu;
Battery battery(PIN_BATT, CAL_GAIN, CAL_OFFSET_MV);

// Seconds since power-on. The C3's system clock keeps running through deep
// sleep (the RTC timer drives it), unlike millis(), which restarts at 0 on
// every wake.
int64_t nowS() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}

bool inCooldown() { return nowS() - lastTamperS < static_cast<int64_t>(TAMPER_COOLDOWN_S); }

void goToSleep(bool mpuOk) {
  // The radio: from standby (~1.6 mA) to sleep (~0.2 uA). The SX1278 keeps
  // its settings while asleep, as long as it has power.
  LoRa.sleep();
  SPI.end();

  // Is the motion wake on this time? (Not during a TAMPER cooldown.)
  bool cooldown = inCooldown();
  bool armed = mpuOk && !cooldown;
  if (armed) {
    // GPIO 1 must be an INPUT, or the sleeping C3 can't "see" it at all.
    // Configure it BEFORE the pin hold below: the hold freezes every pin's
    // configuration as it is at that moment. The pull-down keeps the pin
    // LOW (no false wakes) if the INT wire ever comes loose.
    pinMode(PIN_INT, INPUT_PULLDOWN);
    gpio_set_direction(static_cast<gpio_num_t>(PIN_INT), GPIO_MODE_INPUT);
  }

  // During deep sleep the C3's pins let go (they float). A floating NSS
  // could drift low, and the SX1278 would think a SPI transfer is starting.
  // So: drive NSS high and HOLD it there through the sleep.
  pinMode(PIN_NSS, OUTPUT);
  digitalWrite(PIN_NSS, HIGH);
  gpio_hold_en(static_cast<gpio_num_t>(PIN_NSS));
  gpio_deep_sleep_hold_en();

  // Wake source 1: the timer. During a TAMPER cooldown, wake when it ends
  // (or after SLEEP_S, whichever comes first), so the motion wake is
  // switched back on in time.
  uint32_t sleepS = SLEEP_S;
  if (cooldown) {
    int64_t left = lastTamperS + TAMPER_COOLDOWN_S - nowS();
    if (left >= 1 && left < static_cast<int64_t>(sleepS)) sleepS = static_cast<uint32_t>(left);
  }
  esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(sleepS) * US_PER_S);

  // Wake source 2: GPIO 1 HIGH = the MPU-6050 saw motion. Read INT_STATUS
  // first: that lowers INT, in case something moved while we were awake.
  // (A pin that's already HIGH would wake the C3 again immediately.)
  if (armed) {
    mpu.clearInterrupt();
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_INT, ESP_GPIO_WAKEUP_GPIO_HIGH);
  }

  lastAwakeMs = static_cast<uint16_t>(millis());  // sent with the NEXT packet
  Serial.printf("awake %u ms, sleeping %lu s, motion wake %s\n", static_cast<unsigned>(lastAwakeMs),
                static_cast<unsigned long>(sleepS),
                armed ? "ON" : (mpuOk ? "off (cooldown)" : "off (no MPU)"));
  Serial.flush();
  esp_deep_sleep_start();  // never returns
}

void setup() {
  // The pin held high during the previous sleep has to be let go first,
  // or SPI can't use it.
  gpio_hold_dis(static_cast<gpio_num_t>(PIN_NSS));

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  bool motion = cause == ESP_SLEEP_WAKEUP_GPIO;
  bool cold = !motion && cause != ESP_SLEEP_WAKEUP_TIMER;

  Serial.begin(115200);
  // USB is off during deep sleep, so after a timer wake the Mac usually
  // hasn't reconnected yet. Don't wait for it: printing must never cost
  // awake time. Only after a cold start wait 2 s, so the banner can be read.
  Serial.setTxTimeoutMs(0);
  if (cold) {
    delay(BANNER_WAIT_MS);
    Serial.println("\n=== Winter Guardian - node (C3) ===");
    printRadioSettings();
    Serial.printf("Wakes every %lu s. The serial port disappears while it sleeps: watch the gateway.\n",
                  static_cast<unsigned long>(SLEEP_S));
  }

  // ---- Fill in the message ------------------------------------------------
  GuardianMsg msg = {};
  msg.magic = MSG_MAGIC;
  msg.version = MSG_VERSION;
  msg.seq = ++seq;  // RTC memory: keeps counting across sleeps
  msg.wake_cause = motion ? WAKE_MOTION : (cold ? WAKE_COLD : WAKE_TIMER);
  msg.awake_ms = lastAwakeMs;
  strncpy(msg.name, NODE_NAME, sizeof(msg.name) - 1);

  battery.begin();
  msg.batt_mV = battery.readMv();

  Wire.begin(PIN_SDA, PIN_SCL, I2C_HZ);
  float t, h;
  if (aht.begin() && aht.read(t, h)) {
    msg.flags |= FLAG_AHT_OK;
    msg.t_c10 = static_cast<int16_t>(lroundf(t * 10.0f));
    msg.rh_10 = static_cast<uint16_t>(lroundf(h * 10.0f));
  }

  // ---- MPU-6050 -------------------------------------------------------------
  // Cold boot: set up the motion alarm (the MPU keeps it through our sleeps).
  // Any other wake: just check it still answers.
  bool mpuOk = cold ? mpu.setupMotionWake(MOTION_THRESHOLD, MOTION_DURATION) : (mpu.clearInterrupt() >= 0);
  if (mpuOk) msg.flags |= FLAG_MPU_OK;
  if (motion && !inCooldown()) {
    msg.flags |= FLAG_TAMPER;
    lastTamperS = nowS();
  }

  // ---- Send ---------------------------------------------------------------
  if (!radioBegin()) {
    Serial.println("LoRa NOT FOUND - sleeping anyway, trying again next wake");
    goToSleep(mpuOk);
  }
  uint32_t t0 = millis();
  LoRa.beginPacket();
  LoRa.write(reinterpret_cast<const uint8_t*>(&msg), sizeof(msg));
  LoRa.endPacket();  // waits until the packet is out (~205 ms at SF9)

  const char* why = motion ? "motion" : (cold ? "cold" : "timer");
  Serial.printf("sent #%u (%s)%s%s%s, air time %lu ms\n", static_cast<unsigned>(msg.seq), why, (msg.flags & FLAG_TAMPER) ? " TAMPER" : "",
                (msg.flags & FLAG_AHT_OK) ? "" : " AHT10 FAILED", mpuOk ? "" : " MPU FAILED",
                static_cast<unsigned long>(millis() - t0));

  goToSleep(mpuOk);
}

// The lesson 24 rule "loop() is a list of parts" doesn't apply here: the
// node never reaches loop(). Each wake is ONE pass through setup(), and
// deep sleep in between switches the CPU off, which saves far more battery
// than any loop could.
void loop() {
  // never reached: every wake ends in goToSleep()
}
