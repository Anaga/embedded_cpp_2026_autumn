/*
 * Lesson 10 - homework
 * Reaction game v2
 */

#include <Arduino.h>
#include <stdint.h>

#include "Colour.h"
#include "RgbLed.h"
#include "Button.h"
#include "RingBuffer.h"

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

static RingBuffer<uint16_t, 10> player_a_history;
static RingBuffer<uint16_t, 10> player_b_history;

static const uint32_t STATE_RESULT_DURATION = 2000U;
static const uint32_t STATE_CELEBRATE_DURATION = 5000U;
static const uint32_t STATE_WAIT_DURATION_MIN = 1000U;
static const uint32_t STATE_WAIT_DURATION_MAX = 3001U;

static const uint8_t WINNER_SCORE = 5U;
static Colour winner_colour;

static uint8_t round_number = 1U;
static uint32_t current_state_time;
static uint32_t pause_ms;

static volatile uint32_t g_player_a_pressed_at = 0U;
static volatile bool g_player_a_pressed = false;
static volatile uint32_t g_player_b_pressed_at = 0U;
static volatile bool g_player_b_pressed = false;
static uint32_t g_go_at = 0;

static void IRAM_ATTR onButtonA(void) {
    if (!g_player_a_pressed) {
        g_player_a_pressed_at = micros();
        g_player_a_pressed = true;
    }    
}    
static void IRAM_ATTR onButtonB(void) {
    if (!g_player_b_pressed) {
        g_player_b_pressed_at = micros();
        g_player_b_pressed = true;
    }    
}    

enum class State : uint8_t {
    Waiting,
    Go,
    Result,
    Celebrate
};

static State current_state = State::Waiting;

static void enterState(State next) {
    current_state = next;
    current_state_time = millis();
}

static void addPoint(uint8_t &score) {
    score = (uint8_t)(score + 1U);
}

static uint16_t bestTime(const RingBuffer<uint16_t, 10> &history) {
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

static uint16_t averageTime(const RingBuffer<uint16_t, 10> &history) {
    if (history.count() == 0U) {
        return 0U;
    }
    uint32_t total = history.at(0U);
    for (uint8_t i = 1U; i < history.count(); i++) {
        total = total + history.at(i);
    }
    return (uint16_t) (total / history.count());
}

static void printHistory(const char *name, const RingBuffer<uint16_t, 10> &history) {
    if (history.count() == 0U) {
        Serial.printf("Player %s has no history.\n", name);
        return;
    }
    Serial.printf("Player %s: ", name);
    Serial.printf("Avg reaction: %u, ", (unsigned) averageTime(history));
    Serial.printf("Best: %u ", (unsigned) bestTime(history));
    Serial.printf("(All %u:", (unsigned) history.count());
    for (uint8_t i = 0; i < history.count(); i++) {
        Serial.printf(" %u", (unsigned) history.at(i));
    }
    Serial.printf(")\n");
}

static void celebrate(const char *name, const Colour &colour) {
    Serial.printf("\n=====================================================================\n");
    Serial.printf("%s PLAYER WINS! \n", name);
    Serial.printf("---------------------------------------------------------------------\n");
    Serial.printf("Both player histories:\n");
    printHistory("RED", player_a_history);
    printHistory("BLUE", player_b_history);
    Serial.printf("=====================================================================\n\n");

    led.setColour(colour);
    player_a_score = 0U;
    player_b_score = 0U;
    round_number = 0U;    
    player_a_history.clear();
    player_b_history.clear();
}

void setup(void) {
    Serial.begin(115200);
    delay(4000U);

    led.begin();
    led.off();

    player1.begin();
    attachInterrupt(digitalPinToInterrupt(PLAYER_A_BUTTON_PIN), onButtonA, FALLING);

    player2.begin();
    attachInterrupt(digitalPinToInterrupt(PLAYER_B_BUTTON_PIN), onButtonB, FALLING);
    
    Serial.printf("\n=====================================================================\n");
    Serial.printf("STARTING THE GAME \n");
    Serial.printf("=====================================================================\n\n");
    Serial.printf("Round %u - get ready... Press on green light!\n", (unsigned)round_number);
    
    pause_ms = (uint32_t)random(STATE_WAIT_DURATION_MIN, STATE_WAIT_DURATION_MAX);
    current_state_time = millis();
}

void loop(void) {

    const bool player_a_pressed = player1.wasPressed();
    const bool player_b_pressed = player2.wasPressed();

    switch (current_state)
    {
    case State::Waiting:
        if (player_a_pressed && player_b_pressed) {
            led.setColour(WHITE);
            enterState(State::Result);
            Serial.println("Double false start");
        } else if (player_a_pressed) {
            led.setColour(BLUE);
            enterState(State::Result);
            addPoint(player_b_score);
            Serial.printf("                                                False start. BLUE wins! %u pts\n", (unsigned)player_b_score);
        } else if (player_b_pressed) {
            led.setColour(RED);
            enterState(State::Result);
            addPoint(player_a_score);
            Serial.printf("                  False start. RED wins! %u pts\n", (unsigned)player_a_score);
        } else if ((millis() - current_state_time) >= pause_ms) {
            led.setColour(GREEN);
            g_go_at = micros();
            g_player_a_pressed = false;
            g_player_b_pressed = false;
            enterState(State::Go);
            Serial.println("GO!");
        }
        break;
    case State::Go: {
        const bool a_wins = player_a_pressed && (!player_b_pressed || g_player_a_pressed_at <= g_player_b_pressed_at);
        const bool b_wins = player_b_pressed && !a_wins;
        if (a_wins) {
            const uint32_t reaction_us = g_player_a_pressed_at - g_go_at;
            const uint32_t tenths = reaction_us / 100U;
            led.setColour(RED);
            player_a_history.push((uint16_t)(reaction_us / 1000U));
            enterState(State::Result);
            addPoint(player_a_score);
            Serial.printf("                  RED wins! %u.%u ms, %u pts\n", (unsigned)(tenths / 10U), (unsigned) (tenths % 10U), (unsigned)player_a_score);
            Serial.printf("                  ");
            printHistory("RED", player_a_history);
        } else if (b_wins) {
            const uint32_t reaction_us = g_player_b_pressed_at - g_go_at;
            const uint32_t tenths = reaction_us / 100U;
            led.setColour(BLUE);
            player_b_history.push((uint16_t)(reaction_us / 1000U));
            enterState(State::Result);
            addPoint(player_b_score);
            Serial.printf("                                                BLUE wins! %u.%u ms, %u pts\n", (unsigned)(tenths / 10U), (unsigned) (tenths % 10U), (unsigned)player_b_score);
            Serial.printf("                                                ");
            printHistory("BLUE", player_b_history);
        }
        break;
    }
    case State::Result:
        if ((millis() - current_state_time) >= STATE_RESULT_DURATION) {
            if (player_a_score >= WINNER_SCORE) {
                celebrate("RED", RED);
                winner_colour = RED;
                enterState(State::Celebrate);
            } else if (player_b_score >= WINNER_SCORE) {
                celebrate("BLUE", BLUE);
                winner_colour = BLUE;
                enterState(State::Celebrate);
            } else {
                led.off();
                enterState(State::Waiting);
                round_number++;
                pause_ms = (uint32_t)random(STATE_WAIT_DURATION_MIN, STATE_WAIT_DURATION_MAX);
                Serial.printf("\nRound %u: wait... \n", (unsigned)round_number);
            }
        }
        break;
    case State::Celebrate: {
        const uint32_t elapsed = millis() - current_state_time;
        if (((elapsed / 300) % 2U) == 0U) {
            led.setColour(winner_colour);
        } else {
            led.off();
        }
        if ((millis() - current_state_time) >= STATE_CELEBRATE_DURATION || player_a_pressed || player_b_pressed) {
            led.off();
            enterState(State::Waiting);
            round_number++;
            pause_ms = (uint32_t)random(STATE_WAIT_DURATION_MIN, STATE_WAIT_DURATION_MAX);
            Serial.printf("\n=====================================================================\n");
            Serial.printf("RESTARTING THE GAME \n");
            Serial.printf("---------------------------------------------------------------------\n");
            Serial.printf("New match, both player histories cleared\n");
            Serial.printf("=====================================================================\n\n");
            Serial.printf("Round %u: wait... \n", (unsigned)round_number);
        }
        break;
    }
    default:
        Serial.println("This shouldn't have happened - we're in default switch state");
        break;
    }

}