// Winter Guardian node (ESP32-C3 Super Mini). Runs once per wake, entirely
// in setup(): measure, send one LoRa packet, deep sleep. Wakes on the RTC
// timer or on GPIO 1 (MPU-6050 motion interrupt).

#include <Wire.h>
#include <algorithm>
#include <driver/gpio.h>
#include <esp_sleep.h>

#include "aht10.h"
#include "mpu6050.h"
#include "msg.h"
#include "radio.h"

constexpr int PIN_SDA = 8;  // strapping pin; also the blue LED
constexpr int PIN_SCL = 9;  // strapping pin; also the BOOT button
constexpr int PIN_INT = 1;  // MPU-6050 INT; only GPIO 0-5 can wake the C3 from deep sleep

constexpr uint8_t MOTION_THRESHOLD = 10;
constexpr uint8_t MOTION_DURATION = 1;
constexpr uint32_t TAMPER_COOLDOWN_S = 120;  // at most one TAMPER per 2 min

// Battery divider: 100k / 10k into GPIO 3, 100 nF to GND.
constexpr int PIN_BATT = 3;
constexpr float R_TOP = 100000.0f;
constexpr float R_BOTTOM = 10000.0f;
constexpr int BATT_SAMPLES = 64;

// Two-point calibration against a reference at 11.97 V and 14.37 V.
constexpr float CAL_GAIN = 0.9860f;
constexpr float CAL_OFFSET_MV = 200.7f;

constexpr uint32_t SLEEP_S = 20;  // demo interval
constexpr const char* NODE_NAME = "fz8";

// RTC memory: survives deep sleep, cleared by power-off or reset.
RTC_DATA_ATTR uint16_t seq = 0;
RTC_DATA_ATTR uint16_t lastAwakeMs = 0;
RTC_DATA_ATTR int64_t lastTamperS = -1000000;

Aht10 aht;
Mpu6050 mpu;

// gettimeofday() keeps counting through deep sleep; millis() restarts at 0.
int64_t nowS() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}

bool inCooldown() { return nowS() - lastTamperS < (int64_t)TAMPER_COOLDOWN_S; }

uint16_t readBatteryMv() {
  analogSetPinAttenuation(PIN_BATT, ADC_11db);
  // The C3 ADC is noisy right after wake: discard 4, then a trimmed mean
  // (middle half of 64 sorted samples).
  for (int i = 0; i < 4; i++) analogReadMilliVolts(PIN_BATT);
  uint16_t v[BATT_SAMPLES];
  for (int i = 0; i < BATT_SAMPLES; i++) {
    v[i] = analogReadMilliVolts(PIN_BATT);
    delayMicroseconds(100);
  }
  std::sort(v, v + BATT_SAMPLES);
  uint32_t sum = 0;
  for (int i = BATT_SAMPLES / 4; i < BATT_SAMPLES * 3 / 4; i++) sum += v[i];
  float pinMv = sum / (float)(BATT_SAMPLES / 2);

  float battMv = pinMv * (R_TOP + R_BOTTOM) / R_BOTTOM;
  battMv = CAL_GAIN * battMv + CAL_OFFSET_MV;
  if (battMv < 500) battMv = 0;  // nothing connected
  Serial.printf("battery: pin %.0f mV -> %.0f mV\n", pinMv, battMv);
  return (uint16_t)lroundf(battMv);
}

void goToSleep(bool mpuOk) {
  LoRa.sleep();  // ~1.6 mA standby -> ~0.2 uA
  SPI.end();

  bool cooldown = inCooldown();
  bool armed = mpuOk && !cooldown;
  if (armed) {
    // Must be configured before gpio_deep_sleep_hold_en(), which freezes pin config.
    pinMode(PIN_INT, INPUT_PULLDOWN);
    gpio_set_direction((gpio_num_t)PIN_INT, GPIO_MODE_INPUT);
  }

  // Hold NSS high through sleep, or a floating NSS can start an SPI transfer.
  pinMode(PIN_NSS, OUTPUT);
  digitalWrite(PIN_NSS, HIGH);
  gpio_hold_en((gpio_num_t)PIN_NSS);
  gpio_deep_sleep_hold_en();

  // During cooldown, wake when it ends so the motion wake is re-armed in time.
  uint32_t sleepS = SLEEP_S;
  if (cooldown) {
    int64_t left = lastTamperS + TAMPER_COOLDOWN_S - nowS();
    if (left >= 1 && left < (int64_t)sleepS) sleepS = (uint32_t)left;
  }
  esp_sleep_enable_timer_wakeup((uint64_t)sleepS * 1000000ULL);

  if (armed) {
    mpu.clearInterrupt();  // INT already high would wake us immediately
    esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_INT, ESP_GPIO_WAKEUP_GPIO_HIGH);
  }

  lastAwakeMs = (uint16_t)millis();
  Serial.printf("awake %u ms, sleeping %lu s, motion wake %s\n", lastAwakeMs, (unsigned long)sleepS,
                armed ? "ON" : (mpuOk ? "off (cooldown)" : "off (no MPU)"));
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
  gpio_hold_dis((gpio_num_t)PIN_NSS);  // release the hold from the previous sleep

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  bool motion = cause == ESP_SLEEP_WAKEUP_GPIO;
  bool cold = !motion && cause != ESP_SLEEP_WAKEUP_TIMER;

  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);  // don't block when USB CDC isn't reconnected after wake
  if (cold) {
    delay(2000);
    Serial.println("\n=== Winter Guardian - node (C3) ===");
    printRadioSettings();
    Serial.printf("Wakes every %lu s. The serial port disappears while it sleeps: watch the gateway.\n",
                  (unsigned long)SLEEP_S);
  }

  GuardianMsg msg = {};
  msg.magic = MSG_MAGIC;
  msg.version = MSG_VERSION;
  msg.seq = ++seq;
  msg.wake_cause = motion ? WAKE_MOTION : (cold ? WAKE_COLD : WAKE_TIMER);
  msg.awake_ms = lastAwakeMs;
  strncpy(msg.name, NODE_NAME, sizeof(msg.name) - 1);

  msg.batt_mV = readBatteryMv();

  Wire.begin(PIN_SDA, PIN_SCL, 100000);
  float t, h;
  if (aht.begin() && aht.read(t, h)) {
    msg.flags |= FLAG_AHT_OK;
    msg.t_c10 = (int16_t)lroundf(t * 10);
    msg.rh_10 = (uint16_t)lroundf(h * 10);
  }

  // The MPU keeps its motion setup through our deep sleep: configure on cold boot only.
  bool mpuOk = cold ? mpu.setupMotionWake(MOTION_THRESHOLD, MOTION_DURATION) : (mpu.clearInterrupt() >= 0);
  if (mpuOk) msg.flags |= FLAG_MPU_OK;
  if (motion && !inCooldown()) {
    msg.flags |= FLAG_TAMPER;
    lastTamperS = nowS();
  }

  if (!radioBegin()) {
    Serial.println("LoRa NOT FOUND - sleeping anyway, trying again next wake");
    goToSleep(mpuOk);
  }
  uint32_t t0 = millis();
  LoRa.beginPacket();
  LoRa.write((const uint8_t*)&msg, sizeof(msg));
  LoRa.endPacket();  // blocks until sent (~205 ms at SF9)

  const char* why = motion ? "motion" : (cold ? "cold" : "timer");
  Serial.printf("sent #%u (%s)%s%s%s, air time %lu ms\n", msg.seq, why, (msg.flags & FLAG_TAMPER) ? " TAMPER" : "",
                (msg.flags & FLAG_AHT_OK) ? "" : " AHT10 FAILED", mpuOk ? "" : " MPU FAILED",
                (unsigned long)(millis() - t0));

  goToSleep(mpuOk);
}

void loop() {}  // never reached: every wake ends in deep sleep
