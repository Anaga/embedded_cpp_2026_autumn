#include <Arduino.h>
#include <stdint.h>
#include "Button.h"
#include "RgbLed.h"

// Hardware Pin Configuration
static const uint8_t PIN_BTN_P1 = 0U;
static const uint8_t PIN_BTN_P2 = 1U;
static const uint8_t PIN_LED_R = 5U;
static const uint8_t PIN_LED_G = 6U;
static const uint8_t PIN_LED_B = 7U;

// Hardware Drivers
static Button btn1(PIN_BTN_P1);
static Button btn2(PIN_BTN_P2);
static RgbLed led(PIN_LED_R, PIN_LED_G, PIN_LED_B);

// Game States
enum class GameState {
 WAIT_FOR_GREEN,
 GO_SIGNAL,
 RESULT_SHOW
};

static GameState state = GameState::WAIT_FOR_GREEN;

// Game Variables
static uint32_t round_number = 1U;
static uint8_t score_p1 = 0U;
static uint8_t score_p2 = 0U;

static uint32_t state_start_ms = 0U;
static uint32_t pause_duration_ms = 0U;
static uint32_t go_time_ms = 0U;

// Function taking parameter BY REFERENCE as required by specification
void addPoint(uint8_t &score) {
 score++;
}

void startNewRound() {
 led.setColour(0, 0, 0);

 // Clear leftover button presses
 btn1.wasPressed();
 btn2.wasPressed();

 pause_duration_ms = (uint32_t)random(2000, 5001);
 state_start_ms = millis();
 state = GameState::WAIT_FOR_GREEN;

 Serial.printf("\nRound %u: get ready...\n", round_number);
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

 uint32_t current_ms = millis();

 switch (state) {
 case GameState::WAIT_FOR_GREEN: {
 bool p1_press = btn1.wasPressed();
 bool p2_press = btn2.wasPressed();

 // Check for False Starts
 if (p1_press && p2_press) {
 Serial.println("False start by BOTH players!");
 led.setColour(255, 255, 255); // White for tie
 state_start_ms = current_ms;
 state = GameState::RESULT_SHOW;
 } else if (p1_press) {
 Serial.println("False start by player 1");
 Serial.println("Player 2 wins");
 addPoint(score_p2);
 led.setColour(0, 0, 255); // Blue for P2
 Serial.printf("Score: player 1 - %u, player 2 - %u\n", score_p1, score_p2);
 state_start_ms = current_ms;
 state = GameState::RESULT_SHOW;
 } else if (p2_press) {
 Serial.println("False start by player 2");
 Serial.println("Player 1 wins");
 addPoint(score_p1);
 led.setColour(255, 0, 0); // Red for P1
 Serial.printf("Score: player 1 - %u, player 2 - %u\n", score_p1, score_p2);
 state_start_ms = current_ms;
 state = GameState::RESULT_SHOW;
 } else if (current_ms - state_start_ms >= pause_duration_ms) {
 // Pause over -> GO!
 state = GameState::GO_SIGNAL;
 go_time_ms = current_ms;
 led.setColour(0, 255, 0); // Green LED
 Serial.println("GO!");

 // Clear any presses that happened exactly on the boundary
 btn1.wasPressed();
 btn2.wasPressed();
 }
 break;
 }

 case GameState::GO_SIGNAL: {
 bool p1_press = btn1.wasPressed();
 bool p2_press = btn2.wasPressed();

 if (p1_press && p2_press) {
 Serial.println("Draw! Both pressed at the same time.");
 led.setColour(255, 255, 255); // White for tie
 Serial.printf("Score: player 1 - %u, player 2 - %u\n", score_p1, score_p2);
 state_start_ms = current_ms;
 state = GameState::RESULT_SHOW;
 } else if (p1_press) {
 uint32_t reaction_ms = current_ms - go_time_ms;
 addPoint(score_p1);
 Serial.printf("Player 1 wins in %u ms\n", reaction_ms);
 led.setColour(255, 0, 0); // Red for P1
 Serial.printf("Score: player 1 - %u, player 2 - %u\n", score_p1, score_p2);
 state_start_ms = current_ms;
 state = GameState::RESULT_SHOW;
 } else if (p2_press) {
 uint32_t reaction_ms = current_ms - go_time_ms;
 addPoint(score_p2);
 Serial.printf("Player 2 wins in %u ms\n", reaction_ms);
 led.setColour(0, 0, 255); // Blue for P2
 Serial.printf("Score: player 1 - %u, player 2 - %u\n", score_p1, score_p2);
 state_start_ms = current_ms;
 state = GameState::RESULT_SHOW;
 }
 break;
 }

 case GameState::RESULT_SHOW: {
 if (current_ms - state_start_ms >= 2000U) {
 round_number++;
 startNewRound();
 }
 break;
 }
 }
}