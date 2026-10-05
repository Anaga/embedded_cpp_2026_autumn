/*
 * Game.cpp - non-blocking rounds, false starts and draws.
 */

#include <Arduino.h>

#include "Button.h"
#include "Colour.h"
#include "Game.h"
#include "RgbLed.h"

static const uint32_t MIN_PAUSE_MS = 2000U;
static const uint32_t MAX_PAUSE_MS = 5000U;
static const uint32_t RESULT_MS = 2000U;

static const Colour GO_COLOUR = {0U, 255U, 0U};
static const Colour PLAYER_1_COLOUR = {255U, 0U, 0U};
static const Colour PLAYER_2_COLOUR = {0U, 0U, 255U};
static const Colour DRAW_COLOUR = {255U, 255U, 255U};

/* Change the caller's score through a reference parameter. */
static void addPoint(uint32_t &score) {
    score += 1U;
}

Game::Game(Button &player1, Button &player2, RgbLed &led)
    : m_player1(player1), m_player2(player2), m_led(led),
      m_state(State::Idle), m_state_started_ms(0U),
      m_pause_ms(0U), m_round(0U), m_player1_score(0U), m_player2_score(0U) {
}

void Game::begin(void) {
    m_led.begin();
    m_player1.begin();
    m_player2.begin();
}

void Game::update(void) {
    // Read both before deciding. Keep reading during the result as well,
    // so a button held into the next round does not become another press.
    bool pressed1 = false;
    bool pressed2 = false;
    // In Idle, leave press events for the first round to detect.
    if (m_state != State::Idle) {
        pressed1 = m_player1.wasPressed();
        pressed2 = m_player2.wasPressed();
    }
    const uint32_t now = millis();

    switch (m_state) {
        case State::Idle:
            startRound(now);
            break;

        case State::WaitingForGreen:
            // The LED is still off, even on the pass when the pause expires.
            if (pressed1 && pressed2) {
                finishRound(Result::Draw, now);
            } else if (pressed1 || pressed2) {
                Serial.printf("False start by player %u\n", pressed1 ? 1U : 2U);
                finishRound(pressed1 ? Result::Player2 : Result::Player1, now);
            } else if ((now - m_state_started_ms) >= m_pause_ms) {
                m_led.setColour(GO_COLOUR);
                m_state = State::Ready;
                m_state_started_ms = millis();
                Serial.println("GO!");
            }
            break;

        case State::Ready:
            if (pressed1 && pressed2) {
                finishRound(Result::Draw, now);
            } else if (pressed1 || pressed2) {
                finishRound(pressed1 ? Result::Player1 : Result::Player2, now);
            }
            break;

        case State::ShowingResult:
            if ((now - m_state_started_ms) >= RESULT_MS) {
                startRound(now);
            }
            break;
    }
}

void Game::startRound(uint32_t now) {
    m_led.off();
    m_state = State::WaitingForGreen;
    m_state_started_ms = now;
    m_pause_ms = random(MIN_PAUSE_MS, MAX_PAUSE_MS + 1U);
    m_round += 1U;

    Serial.printf("Round %lu: get ready...\n", m_round);
}

void Game::finishRound(Result result, uint32_t now) {
    if (result == Result::Draw) {
        m_led.setColour(DRAW_COLOUR);
        Serial.println("Draw");
    } else {
        const uint8_t winner = (result == Result::Player1) ? 1U : 2U;
        if (result == Result::Player1) {
            addPoint(m_player1_score);
            m_led.setColour(PLAYER_1_COLOUR);
        } else {
            addPoint(m_player2_score);
            m_led.setColour(PLAYER_2_COLOUR);
        }

        if (m_state == State::Ready) {
            Serial.printf("Player %u wins in %lu ms\n", winner,
                          now - m_state_started_ms);
        } else {
            Serial.printf("Player %u wins\n", winner);
        }
    }

    Serial.printf("Score: player 1 - %lu, player 2 - %lu\n\n",
                  m_player1_score, m_player2_score);
    m_state = State::ShowingResult;
    m_state_started_ms = now;
}
