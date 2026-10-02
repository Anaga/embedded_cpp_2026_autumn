/*
 * Lesson 09 - homework
 * Reaction game
 */

#include <Arduino.h>
#include <stdint.h>

#include "Colour.h"
#include "RgbLed.h"
#include "Button.h"

static const uint8_t LED_RED_PIN = 5U;
static const uint8_t LED_GREEN_PIN = 6U;
static const uint8_t LED_BLUE_PIN = 7U;

static const Colour WHITE = {255U, 255U, 255U};
static const Colour RED = {255U, 0U, 0U};
static const Colour GREEN = {0U, 255U, 0U};
static const Colour BLUE = {0U, 0U, 255U};

static RgbLed led(LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);

static const uint8_t PLAYER_A_BUTTON_PIN = 0U;
static const uint8_t PLAYER_B_BUTTON_PIN = 1U;

static Button player1(PLAYER_A_BUTTON_PIN);
static Button player2(PLAYER_B_BUTTON_PIN);

static uint8_t player_a_score = 0U;
static uint8_t player_b_score = 0U;

static const uint8_t STATE_WAIT = 0U;
static const uint8_t STATE_GO = 1U;
static const uint8_t STATE_RESULT = 2U;
static const uint8_t STATE_CELEBRATE = 3U;

static const uint32_t STATE_RESULT_DURATION = 2000U;
static const uint32_t STATE_CELEBRATE_DURATION = 5000U;

static uint8_t round_number = 1U;
static uint8_t current_state = STATE_WAIT;
static uint32_t current_state_time;
static uint32_t pause_ms;

static const uint8_t WINNER_SCORE = 5U;
static Colour winner_colour;

static void addPoint(uint8_t &score) {
    score = (uint8_t)(score + 1U);
}

static void celebrate(const char *name, const Colour &colour) {
    Serial.printf("\n|============================|\n");
    Serial.printf("|                            |\n");
    Serial.printf("|      %4s PLAYER WINS!     |\n", name);
    Serial.printf("|                            |\n");
    Serial.printf("|============================|\n\n");
    led.setColour(colour);
    player_a_score = 0U;
    player_b_score = 0U;
    round_number = 0U;    
}


void setup(void) {
    Serial.begin(115200);
    delay(3000U);

    led.begin();
    led.off();

    player1.begin();
    player2.begin();

    pause_ms = (uint32_t)random(2000U, 5001U);   // 2000 to 5000
    current_state_time = millis();
    
    Serial.printf("Round %u - get ready... Press on green light!\n", (unsigned)round_number);

}

void loop(void) {
    const bool player_a_pressed = player1.wasPressed();
    const bool player_b_pressed = player2.wasPressed();

    switch (current_state)
    {
    case STATE_WAIT:
        if (player_a_pressed && player_b_pressed) {
            led.setColour(WHITE);
            current_state = STATE_RESULT;
            current_state_time = millis();
            Serial.println("Double false start");
        } else if (player_a_pressed) {
            led.setColour(BLUE);
            current_state = STATE_RESULT;
            current_state_time = millis();
            addPoint(player_b_score);
            Serial.printf("                                                False start. BLUE wins! %u pts\n", (unsigned)player_b_score);
        } else if (player_b_pressed) {
            led.setColour(RED);
            current_state = STATE_RESULT;
            current_state_time = millis();
            addPoint(player_a_score);
            Serial.printf("                  False start. RED wins! %u pts\n", (unsigned)player_a_score);
        } else if ((millis() - current_state_time) >= pause_ms) {
            led.setColour(GREEN);
            current_state = STATE_GO;
            current_state_time = millis();
            Serial.println("GO!");
        }
        break;
    case STATE_GO: {
        const uint32_t reaction_time = millis() - current_state_time;
        if (player_a_pressed && player_b_pressed) {
            led.setColour(WHITE);
            current_state = STATE_RESULT;
            current_state_time = millis();
            Serial.println("DRAW");
        } else if (player_a_pressed) {
            led.setColour(RED);
            current_state = STATE_RESULT;
            current_state_time = millis();
            addPoint(player_a_score);
            Serial.printf("                  RED wins! %u ms, %u pts\n", (unsigned)reaction_time, (unsigned)player_a_score);
        } else if (player_b_pressed) {
            led.setColour(BLUE);
            current_state = STATE_RESULT;
            current_state_time = millis();
            addPoint(player_b_score);
            Serial.printf("                                                BLUE wins! %u ms, %u pts\n", (unsigned)reaction_time, (unsigned)player_b_score);
        }
        break;
    }
    case STATE_RESULT:
        if ((millis() - current_state_time) >= STATE_RESULT_DURATION) {
            if (player_a_score >= WINNER_SCORE) {
                celebrate("RED", RED);
                winner_colour = RED;
                current_state = STATE_CELEBRATE;
            } else if (player_b_score >= WINNER_SCORE) {
                celebrate("BLUE", BLUE);
                winner_colour = BLUE;
                current_state = STATE_CELEBRATE;
            } else {
                led.off();
                current_state = STATE_WAIT;
                round_number++;
                pause_ms = (uint32_t)random(2000U, 5001U);
                Serial.printf("Round %u: wait... \n", (unsigned)round_number);
            }
            current_state_time = millis();
        }
        break;
    case STATE_CELEBRATE: {
        const uint32_t elapsed = millis() - current_state_time;
        if (((elapsed / 300) % 2U) == 0U) {
            led.setColour(winner_colour);
        } else {
            led.off();
        }
        if ((millis() - current_state_time) >= STATE_CELEBRATE_DURATION || player_a_pressed || player_b_pressed) {
            led.off();
            current_state = STATE_WAIT;
            current_state_time = millis();
            round_number++;
            pause_ms = (uint32_t)random(2000U, 5001U);
            Serial.printf("Round %u: wait... \n", (unsigned)round_number);
        }
        break;
    }
    default:
        Serial.println("This shouldn't have happened - we're in default switch state");
        break;
    }

}