// Lesson 12 - one Pico, two UARTs.
//
// The pot side (slave) talks through Serial1 = UART0 on GP0/GP1, the LED side
// (master) through Serial2 = UART1 on GP8/GP9. Two wires connect them:
//   GP0 (UART0 TX) -> GP9 (UART1 RX)   answers "POT n"
//   GP8 (UART1 TX) -> GP1 (UART0 RX)   requests "POT?"
// Serial (USB) carries status messages only.

#include <Arduino.h>
#include <stdint.h>
#include "LineReader.h"

// Wiring
static const uint8_t PIN_POT = 26U;          // GP26 / ADC0
static const uint8_t PIN_ONBOARD_LED = LED_BUILTIN;  // GP25

// Protocol
static const uint32_t UART_BAUD = 115200U;
static const uint32_t SERIAL_BAUD = 115200U;
static const char REQUEST_LINE[] = "POT?";
static const char ANSWER_PREFIX[] = "POT ";
static const uint32_t REQUEST_PERIOD_MS = 100U;  // ask every 100 ms
static const uint32_t ANSWER_TIMEOUT_MS = 200U;  // no answer for 200 ms
static const uint16_t POT_MAX = 1023U;

// Blink mapping: value 0 -> 50 ms half-period, 1023 -> 500 ms
static const uint32_t HALF_PERIOD_MIN_MS = 50U;
static const uint32_t HALF_PERIOD_MAX_MS = 500U;

// Status on USB
static const uint32_t STATUS_PERIOD_MS = 1000U;

// One line buffer per UART
static LineReader pot_reader;
static LineReader led_reader;

// Master (LED side) state machine
enum class MasterState { IDLE, WAITING };
static MasterState master_state = MasterState::IDLE;
static uint32_t next_request_ms = 0U;
static uint32_t wait_started_ms = 0U;

// Last answer
static uint16_t last_pot_value = 0U;
static uint32_t last_half_ms = HALF_PERIOD_MAX_MS;
static bool have_answer = false;
static bool led_off = false;

// Blinking
static uint32_t next_blink_ms = 0U;
static bool led_is_on = false;

static uint32_t last_status_ms = 0U;

static bool isRequest(const char *line) {
  return strcmp(line, REQUEST_LINE) == 0;
}

// Parse "POT n" with 0 <= n <= 1023
static bool isAnswer(const char *line, uint16_t &value) {
  if (strncmp(line, ANSWER_PREFIX, 4U) != 0) {
    return false;
  }
  const char *digits = line + 4U;
  if (*digits == '\0') {
    return false;
  }
  uint32_t parsed = 0U;
  while (*digits != '\0') {
    if (*digits < '0' || *digits > '9') {
      return false;
    }
    parsed = parsed * 10U + (uint32_t)(*digits - '0');
    digits++;
  }
  if (parsed > (uint32_t)POT_MAX) {
    return false;
  }
  value = (uint16_t)parsed;
  return true;
}

// 0 -> 50 ms, 1023 -> 500 ms, linear in between
static uint32_t mapHalfPeriod(uint16_t value) {
  const uint32_t span = HALF_PERIOD_MAX_MS - HALF_PERIOD_MIN_MS;
  return HALF_PERIOD_MIN_MS + ((uint32_t)value * span) / POT_MAX;
}

// Pot side (slave) on Serial1: answer "POT?" and ignore everything else
static void potSide() {
  if (pot_reader.readLine(Serial1)) {
    if (isRequest(pot_reader.line())) {
      Serial1.print("POT ");
      Serial1.print(analogRead(PIN_POT));
      Serial1.print('\n');
    }
  }
}

// LED side (master) on Serial2: ask, wait, check - never block
static void ledSide(uint32_t now_ms) {
  switch (master_state) {
  case MasterState::IDLE:
    if (now_ms >= next_request_ms) {
      Serial2.print(REQUEST_LINE);
      Serial2.print('\n');
      wait_started_ms = now_ms;
      master_state = MasterState::WAITING;
    }
    break;

  case MasterState::WAITING:
    if (led_reader.readLine(Serial2)) {
      uint16_t value = 0U;
      if (isAnswer(led_reader.line(), value)) {
        last_pot_value = value;
        last_half_ms = mapHalfPeriod(value);
        have_answer = true;
        led_off = false;
        next_blink_ms = now_ms;
        next_request_ms = now_ms + REQUEST_PERIOD_MS;
        master_state = MasterState::IDLE;
      }
      // Anything else: ignore the line, keep waiting for the answer
    } else if (now_ms - wait_started_ms >= ANSWER_TIMEOUT_MS) {
      // No answer within 200 ms
      Serial.println("no reply");
      led_off = true;
      next_request_ms = now_ms + REQUEST_PERIOD_MS;
      master_state = MasterState::IDLE;
    }
    break;
  }
}

// Blink at the half-period from the last answer
static void blinkLed(uint32_t now_ms) {
  if (led_off || !have_answer) {
    if (led_is_on) {
      led_is_on = false;
      digitalWrite(PIN_ONBOARD_LED, LOW);
    }
    next_blink_ms = now_ms;
    return;
  }
  if (now_ms - next_blink_ms >= last_half_ms) {
    next_blink_ms = now_ms;
    led_is_on = !led_is_on;
    digitalWrite(PIN_ONBOARD_LED, led_is_on ? HIGH : LOW);
  }
}

// Status once per second on the USB serial
static void printStatus(uint32_t now_ms) {
  if (now_ms - last_status_ms >= STATUS_PERIOD_MS) {
    last_status_ms = now_ms;
    if (have_answer) {
      Serial.printf("pot=%u half=%lu ms\n",
                    (unsigned)last_pot_value, (unsigned long)last_half_ms);
    } else {
      Serial.println("pot=- half=-");
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial1.begin(UART_BAUD);
  Serial2.begin(UART_BAUD);

  pinMode(PIN_POT, INPUT);
  analogRead(PIN_POT);  // kick the ADC
  pinMode(PIN_ONBOARD_LED, OUTPUT);
  digitalWrite(PIN_ONBOARD_LED, LOW);
}

void loop() {
  const uint32_t now_ms = millis();

  potSide();
  ledSide(now_ms);
  blinkLed(now_ms);
  printStatus(now_ms);
}
