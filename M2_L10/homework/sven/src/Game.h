/*
 * Game.h - round state and scores for the two-player reaction game.
 */

#pragma once

#include <stdint.h>

class Button;
class RgbLed;

class Game {
public:
    Game(Button &player1, Button &player2, RgbLed &led);

    void begin(void);

    void update(void);

private:
    enum class State : uint8_t {
        Idle,
        WaitingForGreen,
        Ready,
        ShowingResult
    };

    enum class Result : uint8_t {
        Draw,
        Player1,
        Player2
    };

    void startRound(uint32_t now);

    void finishRound(Result result, uint32_t now);

    Button &m_player1;
    Button &m_player2;
    RgbLed &m_led;

    State m_state;
    uint32_t m_state_started_ms;
    uint32_t m_pause_ms;
    uint32_t m_round;
    uint32_t m_player1_score;
    uint32_t m_player2_score;
};
