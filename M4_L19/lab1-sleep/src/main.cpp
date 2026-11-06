// Lesson 19 - Lab 1: deep sleep, timer wake and button wake
//
// Every start of this program is one of three:
//   cold start  - power-on or RESET
//   timer wake  - SLEEP_S seconds have passed
//   button wake - button A was pressed while the board slept
//
// Cold start and button wake: the board stays awake AWAKE_WINDOW_MS, so
// an upload can start (a sleeping board has no USB port to upload to).
// Timer wake: print, flash the green LED, sleep again.
//
// Wiring (as in K3 and session 18):
//   Button A  GPIO 0 -> button -> GND (INPUT_PULLUP, active low)
//   RGB LED   R 5, G 6, B 7, common anode to 3V3, LOW = on
//
// Deep sleep is a reboot: setup() runs again from the top after every
// wake. Only RTC memory (RTC_DATA_ATTR) and the RTC timer survive.
// loop() is never reached.

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

// ---------------------------------------------------------------- pins
constexpr uint8_t PIN_BUTTON = 0;   // only GPIO 0..5 can wake the C3 from deep sleep
constexpr uint8_t PIN_LED_R  = 5;
constexpr uint8_t PIN_LED_G  = 6;
constexpr uint8_t PIN_LED_B  = 7;
constexpr uint8_t LED_PINS[] = {PIN_LED_R, PIN_LED_G, PIN_LED_B};

// ---------------------------------------------------------------- timing
constexpr uint64_t US_PER_S           = 1000000ULL;
constexpr uint64_t SLEEP_S            = 20;
constexpr uint32_t AWAKE_WINDOW_MS    = 10000;  // time to start an upload
constexpr uint32_t SERIAL_WAIT_MS     = 2000;   // USB comes back after a wake
constexpr uint32_t FLASH_MS           = 50;
constexpr uint32_t RELEASE_TIMEOUT_MS = 5000;
constexpr uint32_t RELEASE_POLL_MS    = 10;

// ---------------------------------------------------------------- RTC memory
// Survives deep sleep. A power cycle clears it.
RTC_DATA_ATTR static uint32_t bootCount   = 0;
RTC_DATA_ATTR static uint32_t prevAwakeMs = 0;

// ---------------------------------------------------------------- LED
// During deep sleep the GPIO matrix is off. Without a hold the LED pins
// may float or be pulled low, and a common-anode LED can glow.
// gpio_hold_en() + gpio_deep_sleep_hold_en() keep them HIGH (LED off).
// After a wake the hold is still active, so it is released first.
static void ledsBegin() {
  for (uint8_t pin : LED_PINS) {
    gpio_hold_dis(static_cast<gpio_num_t>(pin));
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);          // common anode: HIGH = off
  }
}

static void ledsHoldOff() {
  for (uint8_t pin : LED_PINS) {
    digitalWrite(pin, HIGH);
    gpio_hold_en(static_cast<gpio_num_t>(pin));
  }
  gpio_deep_sleep_hold_en();
}

static void flashGreen() {
  digitalWrite(PIN_LED_G, LOW);
  delay(FLASH_MS);
  digitalWrite(PIN_LED_G, HIGH);
}

// ---------------------------------------------------------------- sleep
static const char *wakeText(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER: return "timer";
    case ESP_SLEEP_WAKEUP_GPIO:  return "button";
    default:                     return "cold start (power-on or reset)";
  }
}

// A button still pressed when sleep starts wakes the board at once.
static void waitForRelease() {
  uint32_t start = millis();
  while (digitalRead(PIN_BUTTON) == LOW && millis() - start < RELEASE_TIMEOUT_MS) {
    delay(RELEASE_POLL_MS);
  }
}

static void goToSleep() {
  waitForRelease();
  prevAwakeMs = millis();             // millis() starts at 0 on every wake
  Serial.printf("   sleeping %lu s\n", static_cast<unsigned long>(SLEEP_S));
  Serial.flush();

  ledsHoldOff();
  esp_sleep_enable_timer_wakeup(SLEEP_S * US_PER_S);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BUTTON, ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();             // does not return
}

// ---------------------------------------------------------------- main
void setup() {
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  bootCount++;

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  ledsBegin();
  flashGreen();

  Serial.begin(115200);
  delay(SERIAL_WAIT_MS);              // only for the USB monitor, not for a battery

  if (cause == ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.println();
    Serial.println("Lesson 19 - Lab 1 - deep sleep");
  }
  Serial.printf("#%lu wake: %s\n", static_cast<unsigned long>(bootCount), wakeText(cause));

  if (cause != ESP_SLEEP_WAKEUP_TIMER) {
    Serial.printf("   awake %lu s - upload now\n",
                  static_cast<unsigned long>(AWAKE_WINDOW_MS / 1000U));
  }
  if (cause != ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.printf("   prev awake %lu ms\n", static_cast<unsigned long>(prevAwakeMs));
  }
  if (cause != ESP_SLEEP_WAKEUP_TIMER) {
    delay(AWAKE_WINDOW_MS);
  }

  goToSleep();
}

void loop() {
  // never reached: every path in setup() ends in deep sleep
}
