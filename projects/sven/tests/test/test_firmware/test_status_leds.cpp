#include <gtest/gtest.h>

#include "StatusLeds.h"

namespace {
using Leds = sounder::StatusLeds;
using Voice = sounder::SynthVoice;

class StatusLedsTest : public testing::Test {
protected:
    void SetUp() override {
        fake_arduino::now = 0;
        fake_arduino::resetGpio();
        fake_arduino::delays.clear();
    }

    static void expectLevels(uint8_t red, uint8_t green) {
        EXPECT_EQ(digitalRead(27), red);
        EXPECT_EQ(digitalRead(26), green);
    }
};

TEST_F(StatusLedsTest, InitializesOnlyTheTwoPinsAndAvoidsRepeatedWrites) {
    Leds leds;
    leds.update(Voice::Sine, false);
    EXPECT_TRUE(fake_arduino::writes.empty());
    leds.begin();
    EXPECT_EQ(fake_arduino::modes[27], OUTPUT);
    EXPECT_EQ(fake_arduino::modes[26], OUTPUT);
    EXPECT_EQ(fake_arduino::modes[18], 0);
    ASSERT_EQ(fake_arduino::writes.size(), 2U);
    expectLevels(LOW, LOW);
    leds.begin();
    EXPECT_EQ(fake_arduino::writes.size(), 2U);
    leds.update(Voice::Sine, false);
    expectLevels(HIGH, HIGH);
    const auto writes = fake_arduino::writes.size();
    fake_arduino::now = 499;
    leds.update(Voice::Sine, false);
    leds.update(Voice::Sine, false);
    EXPECT_EQ(fake_arduino::writes.size(), writes);
    EXPECT_TRUE(fake_arduino::delays.empty());
    for (const auto& write : fake_arduino::writes) EXPECT_NE(write.pin, 18);
}

TEST_F(StatusLedsTest, EveryVoiceHasItsOwnGreenRateWithOrWithoutBluetooth) {
    struct Row { Voice voice; uint32_t halfPeriodMs; };
    const Row rows[] = {{Voice::Sine, 1000}, {Voice::Triangle, 500},
                        {Voice::Pulse, 250}, {Voice::Fm, 167}, {Voice::Organ, 125}};
    for (bool connected : {false, true}) {
        for (const auto& row : rows) {
            SCOPED_TRACE(static_cast<int>(row.voice));
            SCOPED_TRACE(connected);
            fake_arduino::now = 120;
            Leds leds;
            leds.begin();
            leds.update(row.voice, connected);
            EXPECT_EQ(digitalRead(26), HIGH);
            fake_arduino::now = 120 + row.halfPeriodMs - 1;
            leds.update(row.voice, connected);
            EXPECT_EQ(digitalRead(26), HIGH);
            fake_arduino::now = 120 + row.halfPeriodMs;
            leds.update(row.voice, connected);
            EXPECT_EQ(digitalRead(26), LOW);
            fake_arduino::now = 120 + 2 * row.halfPeriodMs - 1;
            leds.update(row.voice, connected);
            EXPECT_EQ(digitalRead(26), LOW);
            fake_arduino::now = 120 + 2 * row.halfPeriodMs;
            leds.update(row.voice, connected);
            EXPECT_EQ(digitalRead(26), HIGH);
        }
    }
}

TEST_F(StatusLedsTest, BluetoothRedBlinksOfflineAndTurnsOffImmediatelyWhenConnected) {
    Leds leds;
    leds.begin();
    leds.update(Voice::Sine, false);
    EXPECT_EQ(digitalRead(27), HIGH);
    fake_arduino::now = 500;
    leds.update(Voice::Sine, false);
    EXPECT_EQ(digitalRead(27), LOW);
    fake_arduino::now = 1000;
    leds.update(Voice::Sine, false);
    EXPECT_EQ(digitalRead(27), HIGH);
    fake_arduino::now = 1001;
    leds.update(Voice::Sine, true);
    EXPECT_EQ(digitalRead(27), LOW);
    fake_arduino::now = 6500;
    leds.update(Voice::Sine, true);
    EXPECT_EQ(digitalRead(27), LOW);
    leds.update(Voice::Sine, false);
    EXPECT_EQ(digitalRead(27), HIGH);
    fake_arduino::now = 6999;
    leds.update(Voice::Sine, false);
    EXPECT_EQ(digitalRead(27), HIGH);
    fake_arduino::now = 7000;
    leds.update(Voice::Sine, false);
    EXPECT_EQ(digitalRead(27), LOW);
}

TEST_F(StatusLedsTest, VoiceChangesRestartGreenWithoutResettingBluetoothBlink) {
    Leds leds;
    leds.begin();
    leds.update(Voice::Pulse, false);
    fake_arduino::now = 750;
    leds.update(Voice::Pulse, false);
    expectLevels(LOW, LOW);
    leds.update(Voice::Triangle, false);
    expectLevels(LOW, HIGH);
    fake_arduino::now = 1000;
    leds.update(Voice::Triangle, false);
    expectLevels(HIGH, HIGH);
    fake_arduino::now = 1250;
    leds.update(Voice::Triangle, false);
    expectLevels(HIGH, LOW);
}

TEST_F(StatusLedsTest, ConnectionChangesPreserveTheGreenBlinkPhase) {
    Leds leds;
    leds.begin();
    leds.update(Voice::Pulse, false);
    fake_arduino::now = 250;
    leds.update(Voice::Pulse, false);
    expectLevels(HIGH, LOW);
    fake_arduino::now = 251;
    leds.update(Voice::Pulse, true);
    expectLevels(LOW, LOW);
    fake_arduino::now = 500;
    leds.update(Voice::Pulse, true);
    expectLevels(LOW, HIGH);
    fake_arduino::now = 751;
    leds.update(Voice::Pulse, false);
    expectLevels(HIGH, LOW);
    fake_arduino::now = 1000;
    leds.update(Voice::Pulse, false);
    expectLevels(HIGH, HIGH);
}

TEST_F(StatusLedsTest, DelayedLoopsAndClockRolloverPreserveBothBlinkRates) {
    Leds leds;
    leds.begin();
    const uint32_t start = UINT32_MAX - 100;
    fake_arduino::now = start;
    leds.update(Voice::Pulse, false);
    expectLevels(HIGH, HIGH);
    fake_arduino::now = start + 250;
    leds.update(Voice::Pulse, false);
    expectLevels(HIGH, LOW);
    fake_arduino::now = start + 500;
    leds.update(Voice::Pulse, false);
    expectLevels(LOW, HIGH);
    const auto writes = fake_arduino::writes.size();
    fake_arduino::now = start + 2750;
    leds.update(Voice::Pulse, false);
    expectLevels(LOW, LOW);
    EXPECT_EQ(fake_arduino::writes.size(), writes + 1);
    fake_arduino::now = start + 3000;
    leds.update(Voice::Pulse, false);
    expectLevels(HIGH, HIGH);
    EXPECT_TRUE(fake_arduino::delays.empty());
}

TEST_F(StatusLedsTest, CustomPinsSwitchOffAndReleaseOnDestruction) {
    {
        Leds leds(19, 21);
        leds.begin();
        leds.update(Voice::Organ, false);
        EXPECT_EQ(fake_arduino::modes[19], OUTPUT);
        EXPECT_EQ(fake_arduino::modes[21], OUTPUT);
        EXPECT_EQ(digitalRead(19), HIGH);
        EXPECT_EQ(digitalRead(21), HIGH);
        EXPECT_EQ(fake_arduino::modes[27], 0);
        EXPECT_EQ(fake_arduino::modes[26], 0);
    }
    EXPECT_EQ(digitalRead(19), LOW);
    EXPECT_EQ(digitalRead(21), LOW);
    EXPECT_EQ(fake_arduino::modes[19], INPUT);
    EXPECT_EQ(fake_arduino::modes[21], INPUT);
}

TEST_F(StatusLedsTest, EndIsIdempotentAndRebeginStartsFreshBlinkPhases) {
    Leds leds;
    leds.begin();
    leds.update(Voice::Organ, false);
    leds.end();
    expectLevels(LOW, LOW);
    EXPECT_EQ(fake_arduino::modes[27], INPUT);
    EXPECT_EQ(fake_arduino::modes[26], INPUT);
    const auto writes = fake_arduino::writes.size();
    leds.end();
    leds.update(Voice::Triangle, true);
    EXPECT_EQ(fake_arduino::writes.size(), writes);
    fake_arduino::now = 100;
    leds.begin();
    leds.update(Voice::Sine, false);
    expectLevels(HIGH, HIGH);
    fake_arduino::now = 600;
    leds.update(Voice::Sine, false);
    expectLevels(LOW, HIGH);
}
} // namespace
