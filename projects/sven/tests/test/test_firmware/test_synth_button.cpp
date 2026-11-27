#include <gtest/gtest.h>

#include "SynthButton.h"

namespace {
using Button = sounder::SynthButton;
using Event = Button::Event;

class SynthButtonTest : public testing::Test {
protected:
    void SetUp() override {
        fake_arduino::now = 0;
        fake_arduino::resetGpio();
        fake_arduino::delays.clear();
    }
};

TEST_F(SynthButtonTest, PullupAndDebounceProduceShortPressOnlyOnRelease) {
    Button button;
    EXPECT_EQ(button.poll(), Event::None);
    button.begin();
    EXPECT_EQ(fake_arduino::modes[25], INPUT_PULLUP);
    EXPECT_EQ(digitalRead(25), HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::edge(25, LOW);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 29;
    button.begin();
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 30;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 100;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 129;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 130;
    EXPECT_EQ(button.poll(), Event::ShortPress);
    EXPECT_EQ(button.poll(), Event::None);
    EXPECT_TRUE(fake_arduino::writes.empty());
    EXPECT_TRUE(fake_arduino::delays.empty());
}

TEST_F(SynthButtonTest, BounceRequiresStablePressAndRelease) {
    Button button;
    button.begin();
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::edge(25, LOW);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 10;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 15;
    fake_arduino::edge(25, LOW);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 45;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 100;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 110;
    fake_arduino::edge(25, LOW);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 500;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 600;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 629;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 630;
    EXPECT_EQ(button.poll(), Event::ShortPress);
    EXPECT_EQ(button.poll(), Event::None);
}

TEST_F(SynthButtonTest, LongPressFiresAtThreeSecondsOnceAndSuppressesReleaseSelection) {
    Button button;
    button.begin();
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::edge(25, LOW);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 30;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 2999;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 3000;
    EXPECT_EQ(button.poll(), Event::LongPress);
    fake_arduino::now = 10000;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 10030;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 10100;
    fake_arduino::edge(25, LOW);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 10130;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 10140;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 10170;
    EXPECT_EQ(button.poll(), Event::ShortPress);
}

TEST_F(SynthButtonTest, ReleaseBeforeThresholdStaysShortEvenWhenDebounceFinishesLater) {
    Button button;
    button.begin();
    button.poll();
    fake_arduino::edge(25, LOW);
    button.poll();
    fake_arduino::now = 30;
    button.poll();
    fake_arduino::now = 2999;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 3029;
    EXPECT_EQ(button.poll(), Event::ShortPress);
    EXPECT_EQ(button.poll(), Event::None);
}

TEST_F(SynthButtonTest, DelayedPollingRecognizesLongHoldOnReleaseExactlyOnce) {
    Button button;
    button.begin();
    button.poll();
    fake_arduino::edge(25, LOW);
    button.poll();
    fake_arduino::now = 30;
    button.poll();
    fake_arduino::now = 3000;
    fake_arduino::edge(25, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = 3030;
    EXPECT_EQ(button.poll(), Event::LongPress);
    EXPECT_EQ(button.poll(), Event::None);
}

TEST_F(SynthButtonTest, HeldBeforeFirstPollNeedsReleaseBeforeEitherEvent) {
    for (bool heldBeforeBegin : {false, true}) {
        SCOPED_TRACE(heldBeforeBegin);
        fake_arduino::now = 0;
        fake_arduino::resetGpio();
        if (heldBeforeBegin) fake_arduino::edge(25, LOW);
        Button button;
        button.begin();
        if (!heldBeforeBegin) fake_arduino::edge(25, LOW);
        fake_arduino::now = 2000;
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::now = 8000;
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::edge(25, HIGH);
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::now = 8030;
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::now = 8040;
        fake_arduino::edge(25, LOW);
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::now = 8070;
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::now = 8080;
        fake_arduino::edge(25, HIGH);
        EXPECT_EQ(button.poll(), Event::None);
        fake_arduino::now = 8110;
        EXPECT_EQ(button.poll(), Event::ShortPress);
    }
}

TEST_F(SynthButtonTest, CustomPinAndClockRolloverPreserveShortPress) {
    Button button(26);
    const uint32_t start = UINT32_MAX - 10;
    fake_arduino::now = start;
    button.begin();
    EXPECT_EQ(fake_arduino::modes[26], INPUT_PULLUP);
    button.poll();
    fake_arduino::edge(26, LOW);
    button.poll();
    fake_arduino::now = start + 30;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = start + 40;
    fake_arduino::edge(26, HIGH);
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = start + 70;
    EXPECT_EQ(button.poll(), Event::ShortPress);
}

TEST_F(SynthButtonTest, LongPressTimingHandlesClockRollover) {
    Button button;
    const uint32_t start = UINT32_MAX - 1500;
    fake_arduino::now = start;
    button.begin();
    button.poll();
    fake_arduino::edge(25, LOW);
    button.poll();
    fake_arduino::now = start + 30;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = start + 2999;
    EXPECT_EQ(button.poll(), Event::None);
    fake_arduino::now = start + 3000;
    EXPECT_EQ(button.poll(), Event::LongPress);
    EXPECT_EQ(button.poll(), Event::None);
}
} // namespace
