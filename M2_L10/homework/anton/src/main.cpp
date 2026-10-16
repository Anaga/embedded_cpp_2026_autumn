#include <Arduino.h>
#include <stdint.h>
#include "Button.h"
#include "RgbLed.h"
#include "RingBuffer.h"

// Hardware pin configuration (same board as lesson 9, ESP32-C3 Super Mini)
static const uint8_t PIN_BTN_P1 = 0U;
static const uint8_t PIN_BTN_P2 = 1U;
static const uint8_t PIN_LED_R = 5U;
static const uint8_t PIN_LED_G = 6U;
static const uint8_t PIN_LED_B = 7U;

// Game timing
static const uint32_t PAUSE_MIN_MS = 2000U;
static const uint32_t PAUSE_MAX_MS = 5000U;
static const uint32_t RESULT_SHOW_MS = 2000U;

// History display layout: 10 times of up to 3 digits, space separated
static const uint8_t LIST_WIDTH = 39U;
static const uint8_t PREFIX_WIDTH = 9U;   // "last NN:" padded

// Hardware drivers
static Button btn1(PIN_BTN_P1);
static Button btn2(PIN_BTN_P2);
static RgbLed led(PIN_LED_R, PIN_LED_G, PIN_LED_B);

// Game states
enum class GameState {
  WAIT_FOR_GREEN,
  GO_SIGNAL,
  RESULT_SHOW
};

// Per-player history of the last 10 winning reaction times (no heap)
static RingBuffer<uint16_t, 10> history_p1;
static RingBuffer<uint16_t, 10> history_p2;

static GameState state = GameState::WAIT_FOR_GREEN;

static uint32_t round_number = 1U;
static uint8_t score_p1 = 0U;
static uint8_t score_p2 = 0U;

static uint32_t state_start_ms = 0U;
static uint32_t pause_duration_ms = 0U;
static uint32_t go_time_ms = 0U;
static bool result_shown = false;

// Function taking parameter BY REFERENCE as required by the specification
void addPoint(uint8_t &score) {
  score++;
}

// Smallest time in the history; 0 when the history is empty
uint16_t bestTime(const RingBuffer<uint16_t, 10> &history) {
  if (history.count() == 0U) {
    return 0U;
  }
  uint16_t best = history.at(0U);
  for (uint8_t i = 1U; i < history.count(); i++) {
    if (history.at(i) < best) {
      best = history.at(i);
    }
  }
  return best;
}

// Average of the times in the history; 0 when the history is empty.
// The sum is kept in a uint32_t: ten slow times do not fit in a uint16_t.
uint16_t averageTime(const RingBuffer<uint16_t, 10> &history) {
  if (history.count() == 0U) {
    return 0U;
  }
  uint32_t sum = 0U;
  for (uint8_t i = 0U; i < history.count(); i++) {
    sum += history.at(i);
  }
  return (uint16_t)(sum / history.count());
}

void printPlayerHistory(uint8_t player, const RingBuffer<uint16_t, 10> &history) {
  const uint8_t n = history.count();

  // Prefix "last NN:" padded to PREFIX_WIDTH so the list column lines up
  char prefix[PREFIX_WIDTH + 1U];
  uint8_t pos = 0U;
  const char *word = "last ";
  while (*word != '\0') {
    prefix[pos++] = *word++;
  }
  if (n >= 10U) {
    prefix[pos++] = (char)('0' + (n / 10U));
  }
  prefix[pos++] = (char)('0' + (n % 10U));
  prefix[pos++] = ':';
  while (pos < PREFIX_WIDTH) {
    prefix[pos++] = ' ';
  }
  prefix[PREFIX_WIDTH] = '\0';

  char best_str[6U];
  char avg_str[6U];
  // 10 values of up to 5 digits, space separated: 10*5 + 9
  char list_str[60U];

  if (n == 0U) {
    list_str[0U] = '-';
    list_str[1U] = '\0';
    best_str[0U] = '-';
    best_str[1U] = '\0';
    avg_str[0U] = '-';
    avg_str[1U] = '\0';
  } else {
    uint8_t len = 0U;
    for (uint8_t i = 0U; i < n; i++) {
      uint16_t value = history.at(i);  // at(0) is the oldest
      if (i > 0U) {
        list_str[len++] = ' ';
      }
      // Print all digits, high to low (up to 5 for a uint16_t)
      uint16_t scale = 1U;
      while ((uint32_t)value / ((uint32_t)scale * 10U) > 0U) {
        scale = (uint16_t)(scale * 10U);
      }
      while (scale > 0U) {
        list_str[len++] = (char)('0' + (value / scale));
        value -= (uint16_t)(value / scale) * scale;
        scale = (uint16_t)(scale / 10U);
      }
    }
    list_str[len] = '\0';

    snprintf(best_str, sizeof(best_str), "%u", (unsigned)bestTime(history));
    snprintf(avg_str, sizeof(avg_str), "%u", (unsigned)averageTime(history));
  }

  // "Player 1   last 10: 312 287 ...   best 229   avg 264"
  Serial.printf("Player %u   %s%-39s   best %s   avg %s\n",
                (unsigned)player, prefix, list_str, best_str, avg_str);
}

void printHistory() {
  printPlayerHistory(1U, history_p1);
  printPlayerHistory(2U, history_p2);
}

void startNewRound() {
  led.setColour(0, 0, 0);

  // Clear leftover button presses
  btn1.wasPressed();
  btn2.wasPressed();

  pause_duration_ms = (uint32_t)random(PAUSE_MIN_MS, PAUSE_MAX_MS + 1U);
  state_start_ms = millis();
  state = GameState::WAIT_FOR_GREEN;
  result_shown = false;

  Serial.printf("\nRound %u: get ready...\n", (unsigned)round_number);
}

void setup() {
  Serial.begin(115200);
  btn1.begin();
  btn2.begin();
  led.begin();

  startNewRound();
}

void loop() {
  // Update button debounce loops continuously
  btn1.update();
  btn2.update();

  const uint32_t current_ms = millis();

  switch (state) {
  case GameState::WAIT_FOR_GREEN: {
    const bool p1_press = btn1.wasPressed();
    const bool p2_press = btn2.wasPressed();

    // Check for false starts: nothing is stored in the history
    if (p1_press && p2_press) {
      Serial.println("False start by BOTH players!");
      led.setColour(255, 255, 255);  // white for a draw
      state_start_ms = current_ms;
      state = GameState::RESULT_SHOW;
    } else if (p1_press) {
      Serial.println("False start by player 1");
      Serial.println("Player 2 wins");
      addPoint(score_p2);
      led.setColour(0, 0, 255);  // blue for P2
      Serial.printf("Score: player 1 - %u, player 2 - %u\n",
                    (unsigned)score_p1, (unsigned)score_p2);
      state_start_ms = current_ms;
      state = GameState::RESULT_SHOW;
    } else if (p2_press) {
      Serial.println("False start by player 2");
      Serial.println("Player 1 wins");
      addPoint(score_p1);
      led.setColour(255, 0, 0);  // red for P1
      Serial.printf("Score: player 1 - %u, player 2 - %u\n",
                    (unsigned)score_p1, (unsigned)score_p2);
      state_start_ms = current_ms;
      state = GameState::RESULT_SHOW;
    } else if (current_ms - state_start_ms >= pause_duration_ms) {
      // Pause is over -> GO!
      state = GameState::GO_SIGNAL;
      go_time_ms = current_ms;
      led.setColour(0, 255, 0);  // green LED
      Serial.println("GO!");

      // Clear any presses that happened exactly on the boundary
      btn1.wasPressed();
      btn2.wasPressed();
    }
    break;
  }

  case GameState::GO_SIGNAL: {
    const bool p1_press = btn1.wasPressed();
    const bool p2_press = btn2.wasPressed();

    if (p1_press && p2_press) {
      // A draw: the time goes into both histories
      const uint32_t reaction_ms = current_ms - go_time_ms;
      Serial.println("Draw! Both pressed at the same time.");
      led.setColour(255, 255, 255);  // white for a draw
      history_p1.push((uint16_t)reaction_ms);
      history_p2.push((uint16_t)reaction_ms);
      Serial.printf("Score: player 1 - %u, player 2 - %u\n",
                    (unsigned)score_p1, (unsigned)score_p2);
      state_start_ms = current_ms;
      state = GameState::RESULT_SHOW;
    } else if (p1_press) {
      const uint32_t reaction_ms = current_ms - go_time_ms;
      addPoint(score_p1);
      Serial.printf("Player 1 wins in %lu ms\n",
                    (unsigned long)reaction_ms);
      led.setColour(255, 0, 0);  // red for P1
      Serial.printf("Score: player 1 - %u, player 2 - %u\n",
                    (unsigned)score_p1, (unsigned)score_p2);
      history_p1.push((uint16_t)reaction_ms);
      state_start_ms = current_ms;
      state = GameState::RESULT_SHOW;
    } else if (p2_press) {
      const uint32_t reaction_ms = current_ms - go_time_ms;
      addPoint(score_p2);
      Serial.printf("Player 2 wins in %lu ms\n",
                    (unsigned long)reaction_ms);
      led.setColour(0, 0, 255);  // blue for P2
      Serial.printf("Score: player 1 - %u, player 2 - %u\n",
                    (unsigned)score_p1, (unsigned)score_p2);
      history_p2.push((uint16_t)reaction_ms);
      state_start_ms = current_ms;
      state = GameState::RESULT_SHOW;
    }
    break;
  }

  case GameState::RESULT_SHOW: {
    if (!result_shown) {
      result_shown = true;
      printHistory();
    }
    if (current_ms - state_start_ms >= RESULT_SHOW_MS) {
      round_number++;
      startNewRound();
    }
    break;
  }
  }
}
