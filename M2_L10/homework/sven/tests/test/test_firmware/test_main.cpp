#include <gtest/gtest.h>

#include <array>
#include <new>
#include <string>
#include <vector>

#include <Arduino.h>
#include "../../../src/RingBuffer.h"
// Include each unchanged firmware source once to exercise static helpers too.
#include "../../../src/Button.cpp"
#include "../../../src/RgbLed.cpp"
#include "../../../src/Game.cpp"
#include "../../../src/main.cpp"

using History = RingBuffer<uint16_t, 10>;
using Write = std::pair<uint8_t, uint32_t>;
using RandomRange = std::pair<long, long>;

static void expectHistory(const History &history, const std::vector<uint16_t> &expected) {
    ASSERT_EQ(history.count(), expected.size());
    for (uint8_t index = 0U; index < history.count(); ++index) {
        SCOPED_TRACE(index);
        EXPECT_EQ(history.at(index), expected[index]);
    }
}

TEST(RingBufferTest, Push) {
    History history;
    History other;
    other.push(777U);
    const uint16_t values[] = {0U, 65535U, 1U, 32768U, 231U, 254U, 301U, 287U, 99U, 500U};
    for (uint16_t value : values) {
        history.push(value);
    }
    expectHistory(history, {0U, 65535U, 1U, 32768U, 231U, 254U, 301U, 287U, 99U, 500U});
    history.push(42U);
    expectHistory(history, {65535U, 1U, 32768U, 231U, 254U, 301U, 287U, 99U, 500U, 42U});
    history.push(65000U);
    expectHistory(history, {1U, 32768U, 231U, 254U, 301U, 287U, 99U, 500U, 42U, 65000U});
    for (uint16_t value = 13U; value <= 40U; ++value) {
        history.push(value);
    }
    expectHistory(history, {31U, 32U, 33U, 34U, 35U, 36U, 37U, 38U, 39U, 40U});
    expectHistory(other, {777U});
}

TEST(RingBufferTest, Count) {
    struct Row { uint16_t pushes; uint8_t count; };
    const Row rows[] = {{0, 0}, {1, 1}, {5, 5}, {9, 9}, {10, 10}, {11, 10}, {255, 10}, {256, 10}, {300, 10}};
    for (const auto &row : rows) {
        SCOPED_TRACE(row.pushes);
        History history;
        for (uint16_t i = 0; i < row.pushes; ++i) history.push(i);
        const History &view = history;
        EXPECT_EQ(view.count(), row.count);
    }
}

TEST(RingBufferTest, IsFull) {
    struct Row { uint16_t pushes; bool full; };
    const Row rows[] = {{0, false}, {1, false}, {5, false}, {9, false}, {10, true}, {11, true}, {300, true}};
    for (const auto &row : rows) {
        SCOPED_TRACE(row.pushes);
        History history;
        for (uint16_t i = 0; i < row.pushes; ++i) history.push(i);
        const History &view = history;
        EXPECT_EQ(view.isFull(), row.full);
    }
}

TEST(RingBufferTest, At) {
    struct Row { uint16_t pushes; std::vector<uint16_t> expected; };
    const Row rows[] = {
        {1, {1}}, {5, {1, 2, 3, 4, 5}}, {9, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
        {10, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
        {11, {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}},
        {12, {3, 4, 5, 6, 7, 8, 9, 10, 11, 12}},
        {21, {12, 13, 14, 15, 16, 17, 18, 19, 20, 21}},
        {31, {22, 23, 24, 25, 26, 27, 28, 29, 30, 31}}
    };
    for (const auto &row : rows) {
        SCOPED_TRACE(row.pushes);
        History history;
        for (uint16_t i = 1; i <= row.pushes; ++i) history.push(i);
        expectHistory(history, row.expected);
    }
}

TEST(RingBufferTest, Clear) {
    for (uint16_t pushes : {0, 1, 9, 10, 11, 32}) {
        SCOPED_TRACE(pushes);
        History history;
        for (uint16_t i = 0; i < pushes; ++i) history.push(i);
        history.clear();
        EXPECT_EQ(history.count(), 0U);
        EXPECT_FALSE(history.isFull());
        history.clear();
        history.push(65535U);
        history.push(0U);
        expectHistory(history, {65535U, 0U});
        for (uint16_t i = 1; i <= 10; ++i) history.push(i);
        expectHistory(history, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    }
}

class FirmwareTest : public testing::Test {
protected:
    void SetUp() override { reset(); }

    void reset() {
        fake_arduino::reset();
        Serial = FakeSerial{};
        led = RgbLed(PIN_RED, PIN_GREEN, PIN_BLUE);
        player1 = Button(PIN_BUTTON_A);
        player2 = Button(PIN_BUTTON_B);
        game.~Game();
        new (&game) Game(player1, player2, led);
    }

    void expectColour(uint8_t red, uint8_t green, uint8_t blue) {
        const std::array<uint32_t, 3> expected = {
            static_cast<uint32_t>(255U - red),
            static_cast<uint32_t>(255U - green),
            static_cast<uint32_t>(255U - blue)
        };
        EXPECT_EQ(fake_arduino::duties, expected);
    }

    void expectInitialized() {
        EXPECT_EQ(fake_arduino::pin_modes[PIN_BUTTON_A], INPUT_PULLUP);
        EXPECT_EQ(fake_arduino::pin_modes[PIN_BUTTON_B], INPUT_PULLUP);
        EXPECT_EQ(fake_arduino::attached_channels[PIN_RED], 0);
        EXPECT_EQ(fake_arduino::attached_channels[PIN_GREEN], 1);
        EXPECT_EQ(fake_arduino::attached_channels[PIN_BLUE], 2);
        EXPECT_EQ(fake_arduino::frequencies, (std::array<uint32_t, 3>{5000U, 5000U, 5000U}));
        EXPECT_EQ(fake_arduino::resolutions, (std::array<uint8_t, 3>{8U, 8U, 8U}));
        expectColour(0U, 0U, 0U);
    }

    void press(uint8_t mask) {
        fake_arduino::inputs[PIN_BUTTON_A] = (mask & 1U) ? LOW : HIGH;
        fake_arduino::inputs[PIN_BUTTON_B] = (mask & 2U) ? LOW : HIGH;
    }
};

TEST_F(FirmwareTest, ButtonConstructor) {
    for (uint8_t pin : {0, 1, 42, 255}) {
        SCOPED_TRACE(pin);
        reset();
        Button button(pin);
        EXPECT_TRUE(fake_arduino::reads.empty());
        EXPECT_EQ(fake_arduino::pin_modes[pin], -1);
        EXPECT_FALSE(button.wasPressed());
        fake_arduino::now_ms = 30U;
        fake_arduino::inputs[pin] = LOW;
        EXPECT_TRUE(button.wasPressed());
        EXPECT_EQ(fake_arduino::reads, (std::vector<uint8_t>{pin, pin}));
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, ButtonBegin) {
    for (uint8_t pin : {0, 1, 42, 255}) {
        SCOPED_TRACE(pin);
        reset();
        Button button(pin);
        button.begin();
        button.begin();
        for (std::size_t i = 0; i < fake_arduino::pin_modes.size(); ++i) {
            EXPECT_EQ(fake_arduino::pin_modes[i], i == pin ? INPUT_PULLUP : -1);
        }
        EXPECT_TRUE(fake_arduino::reads.empty());
        EXPECT_TRUE(fake_arduino::delays.empty());
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, ButtonWasPressed) {
    struct Step { uint32_t time; uint8_t level; bool pressed; };
    const std::vector<std::vector<Step>> rows = {
        {{0, HIGH, false}, {29, LOW, false}, {30, LOW, false}, {100, LOW, false}},
        {{30, LOW, true}, {31, LOW, false}, {100, LOW, false}},
        {{31, LOW, true}, {32, HIGH, false}, {33, LOW, false}, {34, HIGH, false},
         {63, LOW, false}, {64, HIGH, false}, {94, LOW, true}},
        {{30, LOW, true}, {60, HIGH, false}, {91, LOW, true}},
        {{UINT32_MAX - 20U, LOW, true}, {UINT32_MAX - 10U, HIGH, false}, {18, LOW, false}},
        {{UINT32_MAX - 20U, LOW, true}, {UINT32_MAX - 10U, HIGH, false}, {19, LOW, true}},
        {{UINT32_MAX - 20U, LOW, true}, {UINT32_MAX - 10U, HIGH, false}, {20, LOW, true}}
    };
    for (std::size_t row = 0; row < rows.size(); ++row) {
        SCOPED_TRACE(row);
        reset();
        Button button(42U);
        for (const auto &step : rows[row]) {
            SCOPED_TRACE(step.time);
            fake_arduino::now_ms = step.time;
            fake_arduino::inputs[42] = step.level;
            EXPECT_EQ(button.wasPressed(), step.pressed);
            EXPECT_EQ(fake_arduino::now_ms, step.time);
        }
        EXPECT_EQ(fake_arduino::reads, std::vector<uint8_t>(rows[row].size(), 42U));
        EXPECT_TRUE(fake_arduino::writes.empty());
        EXPECT_TRUE(fake_arduino::delays.empty());
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, RgbLedConstructor) {
    for (const auto &pins : {std::array<uint8_t, 3>{0, 1, 2}, {5, 6, 7}, {253, 254, 255}}) {
        SCOPED_TRACE(pins[0]);
        reset();
        RgbLed rgb(pins[0], pins[1], pins[2]);
        EXPECT_TRUE(fake_arduino::writes.empty());
        EXPECT_EQ(fake_arduino::attached_channels[pins[0]], -1);
        rgb.begin();
        for (uint8_t channel = 0; channel < 3; ++channel) {
            EXPECT_EQ(fake_arduino::attached_channels[pins[channel]], channel);
        }
        expectColour(0, 0, 0);
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, RgbLedBegin) {
    for (unsigned calls : {1U, 2U}) {
        SCOPED_TRACE(calls);
        reset();
        for (unsigned i = 0; i < calls; ++i) led.begin();
        EXPECT_EQ(fake_arduino::frequencies, (std::array<uint32_t, 3>{5000, 5000, 5000}));
        EXPECT_EQ(fake_arduino::resolutions, (std::array<uint8_t, 3>{8, 8, 8}));
        EXPECT_EQ(fake_arduino::attached_channels[5], 0);
        EXPECT_EQ(fake_arduino::attached_channels[6], 1);
        EXPECT_EQ(fake_arduino::attached_channels[7], 2);
        EXPECT_EQ(fake_arduino::writes.size(), 3U * calls);
        expectColour(0, 0, 0);
        EXPECT_TRUE(fake_arduino::delays.empty());
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, RgbLedSetColourReference) {
    struct Row { Colour colour; std::array<uint32_t, 3> duties; };
    const Row rows[] = {{{0, 0, 0}, {255, 255, 255}}, {{255, 255, 255}, {0, 0, 0}},
                        {{1, 127, 254}, {254, 128, 1}}, {{255, 0, 128}, {0, 255, 127}}};
    for (const auto &row : rows) {
        SCOPED_TRACE(row.colour.red);
        reset();
        led.setColour(row.colour);
        EXPECT_EQ(fake_arduino::duties, row.duties);
        EXPECT_EQ(fake_arduino::writes.size(), 3U);
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, RgbLedSetColourChannels) {
    struct Row { uint8_t red, green, blue; std::array<uint32_t, 3> duties; };
    const Row rows[] = {{0, 0, 0, {255, 255, 255}}, {255, 255, 255, {0, 0, 0}},
                        {1, 127, 254, {254, 128, 1}}, {255, 0, 128, {0, 255, 127}}};
    for (const auto &row : rows) {
        SCOPED_TRACE(row.red);
        reset();
        led.setColour(row.red, row.green, row.blue);
        EXPECT_EQ(fake_arduino::duties, row.duties);
        EXPECT_EQ(fake_arduino::writes.size(), 3U);
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, RgbLedOff) {
    for (const Colour colour : {Colour{0, 0, 0}, {1, 127, 254}, {255, 255, 255}}) {
        SCOPED_TRACE(colour.red);
        reset();
        led.setColour(colour);
        fake_arduino::writes.clear();
        led.off();
        EXPECT_EQ(fake_arduino::writes, (std::vector<Write>{{0, 255}, {1, 255}, {2, 255}}));
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, RgbLedWriteChannel) {
    // Exercise the private helper through setColour, including each channel.
    struct Row { uint8_t level; uint32_t duty; };
    const Row rows[] = {{0, 255}, {1, 254}, {127, 128}, {128, 127}, {254, 1}, {255, 0}};
    for (const auto &row : rows) {
        for (uint8_t channel = 0; channel < 3; ++channel) {
            SCOPED_TRACE(row.level);
            SCOPED_TRACE(channel);
            reset();
            uint8_t levels[] = {0, 0, 0};
            levels[channel] = row.level;
            led.setColour(levels[0], levels[1], levels[2]);
            ASSERT_EQ(fake_arduino::writes.size(), 3U);
            EXPECT_EQ(fake_arduino::writes[channel], Write(channel, row.duty));
            EXPECT_TRUE(Serial.output.empty());
        }
    }
}

TEST_F(FirmwareTest, AddPoint) {
    struct Row { uint32_t before, after; };
    const Row rows[] = {{0, 1}, {1, 2}, {42, 43}, {UINT32_MAX - 1U, UINT32_MAX}, {UINT32_MAX, 0}};
    for (const auto &row : rows) {
        SCOPED_TRACE(row.before);
        reset();
        uint32_t score = row.before;
        addPoint(score);
        EXPECT_EQ(score, row.after);
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, GameConstructor) {
    EXPECT_TRUE(fake_arduino::writes.empty());
    EXPECT_TRUE(fake_arduino::reads.empty());
    EXPECT_TRUE(Serial.output.empty());
    fake_arduino::now_ms = 100U;
    game.update();
    EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
    EXPECT_TRUE(fake_arduino::reads.empty());
    expectColour(0, 0, 0);
    Serial.output.clear();
    fake_arduino::now_ms = 2100U;
    game.update();
    press(2U);
    fake_arduino::now_ms = 2331U;
    game.update();
    EXPECT_EQ(Serial.output,
              "GO!\nPlayer 2 wins in 231 ms\nScore: player 1 - 0, player 2 - 1\n\n"
              "Player 1   last 0: -   best -   avg -\n"
              "Player 2   last 1: 231   best 231   avg 231\n\n");
}

TEST_F(FirmwareTest, GameBegin) {
    for (unsigned calls : {1U, 2U}) {
        SCOPED_TRACE(calls);
        reset();
        for (unsigned i = 0; i < calls; ++i) game.begin();
        expectInitialized();
        EXPECT_EQ(fake_arduino::writes.size(), 3U * calls);
        EXPECT_TRUE(fake_arduino::reads.empty());
        EXPECT_TRUE(fake_arduino::delays.empty());
        EXPECT_TRUE(fake_arduino::random_ranges.empty());
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, GameStartRound) {
    struct Row { uint32_t start; long pause; };
    const Row rows[] = {{0, 2000}, {100, 3500}, {UINT32_MAX - 1000U, 5000}};
    for (const auto &row : rows) {
        SCOPED_TRACE(row.start);
        reset();
        fake_arduino::now_ms = row.start;
        fake_arduino::random_value = row.pause;
        game.update();  // Idle calls the private startRound.
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
        EXPECT_EQ(fake_arduino::random_ranges, (std::vector<RandomRange>{{2000, 5001}}));
        expectColour(0, 0, 0);
        Serial.output.clear();
        fake_arduino::now_ms = row.start + static_cast<uint32_t>(row.pause) - 1U;
        game.update();
        EXPECT_TRUE(Serial.output.empty());
        expectColour(0, 0, 0);
        fake_arduino::now_ms += 1U;
        game.update();
        EXPECT_EQ(Serial.output, "GO!\n");
        expectColour(0, 255, 0);
        EXPECT_TRUE(fake_arduino::delays.empty());
    }
}

TEST_F(FirmwareTest, GameFinishRound) {
    struct Row { bool ready; uint8_t pressed; const char *text; Colour colour; const char *histories; };
    const Row rows[] = {
        {false, 1, "False start by player 1\nPlayer 2 wins\nScore: player 1 - 0, player 2 - 1\n\n", {0, 0, 255},
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {false, 2, "False start by player 2\nPlayer 1 wins\nScore: player 1 - 1, player 2 - 0\n\n", {255, 0, 0},
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {false, 3, "Draw\nScore: player 1 - 0, player 2 - 0\n\n", {255, 255, 255},
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {true, 1, "Player 1 wins in 231 ms\nScore: player 1 - 1, player 2 - 0\n\n", {255, 0, 0},
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {true, 2, "Player 2 wins in 231 ms\nScore: player 1 - 0, player 2 - 1\n\n", {0, 0, 255},
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 1: 231   best 231   avg 231\n\n"},
        {true, 3, "Draw\nScore: player 1 - 0, player 2 - 0\n\n", {255, 255, 255},
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 1: 231   best 231   avg 231\n\n"}
    };
    for (const auto &row : rows) {
        SCOPED_TRACE(row.text);
        reset();
        fake_arduino::now_ms = 100U;
        game.update();
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
        Serial.output.clear();
        if (row.ready) {
            fake_arduino::now_ms = 2100U;
            game.update();
            EXPECT_EQ(Serial.output, "GO!\n");
            Serial.output.clear();
        }
        fake_arduino::now_ms += 231U;
        press(row.pressed);
        game.update();  // Calls the private finishRound through every outcome.
        EXPECT_EQ(Serial.output, std::string(row.text) + row.histories);
        expectColour(row.colour.red, row.colour.green, row.colour.blue);
        Serial.output.clear();
        const uint32_t result_time = fake_arduino::now_ms;
        game.update();
        EXPECT_TRUE(Serial.output.empty());
        fake_arduino::now_ms = result_time + 1999U;
        game.update();
        EXPECT_TRUE(Serial.output.empty());
        expectColour(row.colour.red, row.colour.green, row.colour.blue);
        fake_arduino::now_ms = result_time + 2000U;
        game.update();
        EXPECT_EQ(Serial.output, "Round 2: get ready...\n");
        expectColour(0, 0, 0);
        EXPECT_TRUE(fake_arduino::delays.empty());
    }
}

TEST_F(FirmwareTest, GameUpdate) {
    // The full decision table is also exercised through loop below.
    for (uint32_t start : {100U, UINT32_MAX - 1000U}) {
        SCOPED_TRACE(start);
        reset();
        fake_arduino::now_ms = start;
        game.update();
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
        EXPECT_TRUE(fake_arduino::reads.empty());
        Serial.output.clear();
        for (uint32_t elapsed : {0U, 1U, 1999U, 2000U, 2001U}) {
            SCOPED_TRACE(elapsed);
            fake_arduino::now_ms = start + elapsed;
            game.update();
            EXPECT_EQ(Serial.output, elapsed < 2000U ? "" : "GO!\n");
            expectColour(0, elapsed < 2000U ? 0 : 255, 0);
        }
        EXPECT_EQ(fake_arduino::reads.size(), 10U);
        EXPECT_EQ(fake_arduino::random_ranges.size(), 1U);
        EXPECT_TRUE(fake_arduino::delays.empty());
    }
}

TEST_F(FirmwareTest, Setup) {
    for (uint32_t start : {0U, 123U, UINT32_MAX - 1000U}) {
        SCOPED_TRACE(start);
        reset();
        fake_arduino::now_ms = start;
        setup();
        EXPECT_EQ(Serial.baud, 115200U);
        EXPECT_EQ(fake_arduino::now_ms, static_cast<uint32_t>(start + 1500U));
        EXPECT_EQ(fake_arduino::delays, (std::vector<uint32_t>{1500U}));
        EXPECT_EQ(Serial.output, "\nThe Reaction Game\n");
        expectInitialized();
        EXPECT_EQ(fake_arduino::writes, (std::vector<Write>{{0, 255}, {1, 255}, {2, 255}}));
        EXPECT_TRUE(fake_arduino::reads.empty());
        EXPECT_TRUE(fake_arduino::random_ranges.empty());
        Serial.output.clear();
        loop();
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
    }
}

TEST_F(FirmwareTest, Loop) {
    struct Row {
        uint32_t start;
        uint32_t pause;
        bool ready;
        uint32_t elapsed;  // Since GO for ready rows, since round start otherwise.
        uint8_t pressed;
        const char *result;
        Colour colour;
        const char *next_score;
        const char *histories;
        const char *next_histories;
    };
    const Row rows[] = {
        {1500, 2000, true, 0, 1, "Player 1 wins in 0 ms\nScore: player 1 - 1, player 2 - 0\n\n", {255, 0, 0}, "Score: player 1 - 2, player 2 - 0\n\n",
         "Player 1   last 1: 0   best 0   avg 0\nPlayer 2   last 0: -   best -   avg -\n\n",
         "Player 1   last 2: 0 231   best 0   avg 115\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {1500, 3500, true, 1, 2, "Player 2 wins in 1 ms\nScore: player 1 - 0, player 2 - 1\n\n", {0, 0, 255}, "Score: player 1 - 1, player 2 - 1\n\n",
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 1: 1   best 1   avg 1\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 1: 1   best 1   avg 1\n\n"},
        {1500, 5000, true, 231, 3, "Draw\nScore: player 1 - 0, player 2 - 0\n\n", {255, 255, 255}, "Score: player 1 - 1, player 2 - 0\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 1: 231   best 231   avg 231\n\n",
         "Player 1   last 2: 231 231   best 231   avg 231\nPlayer 2   last 1: 231   best 231   avg 231\n\n"},
        {1500, 2000, true, 65535, 1, "Player 1 wins in 65535 ms\nScore: player 1 - 1, player 2 - 0\n\n", {255, 0, 0}, "Score: player 1 - 2, player 2 - 0\n\n",
         "Player 1   last 1: 65535   best 65535   avg 65535\nPlayer 2   last 0: -   best -   avg -\n\n",
         "Player 1   last 2: 65535 231   best 231   avg 32883\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {1500, 2000, true, UINT32_MAX, 2, "Player 2 wins in 4294967295 ms\nScore: player 1 - 0, player 2 - 1\n\n", {0, 0, 255}, "Score: player 1 - 1, player 2 - 1\n\n",
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 1: 65535   best 65535   avg 65535\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 1: 65535   best 65535   avg 65535\n\n"},
        {UINT32_MAX - 1000U, 2000, true, 231, 1, "Player 1 wins in 231 ms\nScore: player 1 - 1, player 2 - 0\n\n", {255, 0, 0}, "Score: player 1 - 2, player 2 - 0\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 0: -   best -   avg -\n\n",
         "Player 1   last 2: 231 231   best 231   avg 231\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {UINT32_MAX - 2400U, 2000, true, 231, 2, "Player 2 wins in 231 ms\nScore: player 1 - 0, player 2 - 1\n\n", {0, 0, 255}, "Score: player 1 - 1, player 2 - 1\n\n",
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 1: 231   best 231   avg 231\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 1: 231   best 231   avg 231\n\n"},
        {1500, 2000, false, 1999, 1, "False start by player 1\nPlayer 2 wins\nScore: player 1 - 0, player 2 - 1\n\n", {0, 0, 255}, "Score: player 1 - 1, player 2 - 1\n\n",
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 0: -   best -   avg -\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {1500, 2000, false, 2000, 2, "False start by player 2\nPlayer 1 wins\nScore: player 1 - 1, player 2 - 0\n\n", {255, 0, 0}, "Score: player 1 - 2, player 2 - 0\n\n",
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 0: -   best -   avg -\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 0: -   best -   avg -\n\n"},
        {1500, 2000, false, 2001, 3, "Draw\nScore: player 1 - 0, player 2 - 0\n\n", {255, 255, 255}, "Score: player 1 - 1, player 2 - 0\n\n",
         "Player 1   last 0: -   best -   avg -\nPlayer 2   last 0: -   best -   avg -\n\n",
         "Player 1   last 1: 231   best 231   avg 231\nPlayer 2   last 0: -   best -   avg -\n\n"}
    };
    for (const auto &row : rows) {
        SCOPED_TRACE(row.start);
        SCOPED_TRACE(row.result);
        reset();
        setup();
        EXPECT_EQ(Serial.output, "\nThe Reaction Game\n");
        Serial.output.clear();
        fake_arduino::now_ms = row.start;
        fake_arduino::random_value = row.pause;
        loop();
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
        EXPECT_TRUE(fake_arduino::reads.empty());  // Idle leaves inputs untouched.
        expectColour(0, 0, 0);
        Serial.output.clear();
        loop();
        EXPECT_TRUE(Serial.output.empty());
        EXPECT_EQ(fake_arduino::reads, (std::vector<uint8_t>{0, 1}));
        if (row.ready) {
            fake_arduino::now_ms = row.start + row.pause - 1U;
            loop();
            EXPECT_TRUE(Serial.output.empty());
            expectColour(0, 0, 0);
            fake_arduino::now_ms += 1U;
            loop();
            EXPECT_EQ(Serial.output, "GO!\n");
            expectColour(0, 255, 0);
            Serial.output.clear();
            loop();  // Ready without a press preserves state and is silent.
            EXPECT_TRUE(Serial.output.empty());
            if (row.elapsed > 0U) {
                fake_arduino::now_ms += 1U;
                loop();
                EXPECT_TRUE(Serial.output.empty());
                expectColour(0, 255, 0);
            }
        }
        const uint32_t result_time = row.start + (row.ready ? row.pause : 0U) + row.elapsed;
        fake_arduino::now_ms = result_time;
        press(row.pressed);
        loop();
        EXPECT_EQ(Serial.output, std::string(row.result) + row.histories);
        expectColour(row.colour.red, row.colour.green, row.colour.blue);
        Serial.output.clear();
        for (uint32_t elapsed : {0U, 1U, 1999U}) {
            SCOPED_TRACE(elapsed);
            fake_arduino::now_ms = result_time + elapsed;
            loop();
            EXPECT_TRUE(Serial.output.empty());
            expectColour(row.colour.red, row.colour.green, row.colour.blue);
            EXPECT_EQ(fake_arduino::random_ranges.size(), 1U);
        }
        fake_arduino::now_ms = result_time + 2000U;
        loop();
        EXPECT_EQ(Serial.output, "Round 2: get ready...\n");
        expectColour(0, 0, 0);
        EXPECT_EQ(fake_arduino::random_ranges, (std::vector<RandomRange>{{2000, 5001}, {2000, 5001}}));
        Serial.output.clear();
        fake_arduino::now_ms += 1U;
        loop();  // Holding into the new round is not another press.
        EXPECT_TRUE(Serial.output.empty());
        expectColour(0, 0, 0);
        press(0U);
        fake_arduino::now_ms += 1U;
        loop();
        EXPECT_TRUE(Serial.output.empty());
        fake_arduino::now_ms = result_time + 2000U + row.pause;
        loop();
        EXPECT_EQ(Serial.output, "GO!\n");
        Serial.output.clear();
        press(1U);
        fake_arduino::now_ms += 231U;
        loop();
        EXPECT_EQ(Serial.output, std::string("Player 1 wins in 231 ms\n") + row.next_score + row.next_histories);
        expectColour(255, 0, 0);
        EXPECT_EQ(fake_arduino::delays, (std::vector<uint32_t>{1500U}));
        EXPECT_EQ(fake_arduino::now_ms, static_cast<uint32_t>(result_time + 2000U + row.pause + 231U));
    }

    // Debounce rejection in loop must preserve the waiting state and score.
    struct DebounceRow { uint32_t time; bool accepted; };
    const DebounceRow debounce_rows[] = {{29, false}, {30, true}, {31, true}};
    for (const auto &row : debounce_rows) {
        SCOPED_TRACE(row.time);
        reset();
        loop();
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
        Serial.output.clear();
        fake_arduino::now_ms = row.time;
        press(1U);
        loop();
        if (row.accepted) {
            EXPECT_EQ(Serial.output,
                      "False start by player 1\nPlayer 2 wins\nScore: player 1 - 0, player 2 - 1\n\n"
                      "Player 1   last 0: -   best -   avg -\n"
                      "Player 2   last 0: -   best -   avg -\n\n");
            expectColour(0, 0, 255);
        } else {
            EXPECT_TRUE(Serial.output.empty());
            expectColour(0, 0, 0);
            fake_arduino::now_ms = 2000U;
            loop();
            EXPECT_EQ(Serial.output, "GO!\n");
            expectColour(0, 255, 0);
            Serial.output.clear();
            press(3U);  // Only player 2 has a fresh edge.
            fake_arduino::now_ms = 2231U;
            loop();
            EXPECT_EQ(Serial.output,
                      "Player 2 wins in 231 ms\nScore: player 1 - 0, player 2 - 1\n\n"
                      "Player 1   last 0: -   best -   avg -\n"
                      "Player 2   last 1: 231   best 231   avg 231\n\n");
        }
        EXPECT_TRUE(fake_arduino::delays.empty());
    }

    // A low input on the very first call must reach the first round's decision.
    for (uint8_t mask : {1, 2, 3}) {
        SCOPED_TRACE(mask);
        reset();
        fake_arduino::now_ms = 1500U;
        press(mask);
        loop();
        EXPECT_EQ(Serial.output, "Round 1: get ready...\n");
        EXPECT_TRUE(fake_arduino::reads.empty());
        Serial.output.clear();
        loop();
        const char *expected[] = {
            "False start by player 1\nPlayer 2 wins\nScore: player 1 - 0, player 2 - 1\n\n",
            "False start by player 2\nPlayer 1 wins\nScore: player 1 - 1, player 2 - 0\n\n",
            "Draw\nScore: player 1 - 0, player 2 - 0\n\n"
        };
        EXPECT_EQ(Serial.output, std::string(expected[mask - 1U]) +
                  "Player 1   last 0: -   best -   avg -\n"
                  "Player 2   last 0: -   best -   avg -\n\n");
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
#ifdef PIO_UNIT_TESTING
    (void)result;
    return 0;
#else
    return result;
#endif
}
