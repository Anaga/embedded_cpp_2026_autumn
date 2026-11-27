#include <gtest/gtest.h>

#include "HandDistanceFilter.h"

#include <limits>

namespace {
using Filter = sounder::HandDistanceFilter;

TEST(HandDistanceFilterTest, RejectsAnIsolatedSpikeAndDoesNotConfirmACachedReading) {
    Filter filter;
    // Distances from the supplied log; timestamps simulate consecutive 70 ms
    // samples. The actual attachment contains only one-second snapshots.
    filter.observe(true, 18.12f, 0);
    filter.observe(true, 39.24f, 70);
    for (uint32_t now = 70; now < 140; ++now) {
        filter.observe(true, 39.24f, 70);
        const auto hand = filter.sample(now);
        EXPECT_TRUE(hand.valid);
        EXPECT_FLOAT_EQ(hand.distanceCm, 18.12f);
        EXPECT_EQ(hand.acceptedAtMs, 0U);
        EXPECT_EQ(hand.heldJumps, 1U);
    }
    filter.observe(true, 18.46f, 140);
    const auto resumed = filter.sample(170);
    EXPECT_EQ(resumed.acceptedAtMs, 140U);
    EXPECT_GT(resumed.distanceCm, 18.12f);
    EXPECT_LT(resumed.distanceCm, 18.46f);
    EXPECT_EQ(resumed.heldJumps, 1U);
}

TEST(HandDistanceFilterTest, ConfirmsPersistentChangesAndLimitsOutputSpeed) {
    Filter filter;
    filter.observe(true, 18, 0);
    filter.observe(true, 39, 70);
    filter.observe(true, 39.5f, 140);
    EXPECT_FLOAT_EQ(filter.sample(140).distanceCm, 18);
    EXPECT_EQ(filter.sample(140).acceptedAtMs, 140U);
    float previous = 18;
    for (uint32_t now = 141; now <= 2000; ++now) {
        if (now % 70 == 0) filter.observe(true, 39.5f, now);
        const auto hand = filter.sample(now);
        ASSERT_TRUE(hand.valid);
        EXPECT_GE(hand.distanceCm, previous);
        EXPECT_LE(hand.distanceCm - previous, 0.0301f);
        previous = hand.distanceCm;
    }
    EXPECT_NEAR(previous, 39.5f, 0.01f);
}

TEST(HandDistanceFilterTest, SuppressesSmallJitterAndFollowsGentleMovement) {
    Filter filter;
    filter.observe(true, 20, 0);
    filter.observe(true, 20.10f, 70);
    filter.observe(true, 20.12f, 140);
    EXPECT_FLOAT_EQ(filter.sample(200).distanceCm, 20);
    EXPECT_EQ(filter.sample(200).acceptedAtMs, 140U);
    filter.reset();
    filter.observe(true, 20, 0);
    float latest = 20;
    for (uint32_t now = 1; now <= 1000; ++now) {
        if (now % 70 == 0) {
            latest = 20 + now * 0.01f; // A steady 10 cm/s hand movement.
            filter.observe(true, latest, now);
        }
        const auto hand = filter.sample(now);
        EXPECT_TRUE(hand.valid);
        EXPECT_LE(hand.distanceCm, latest);
        EXPECT_GT(hand.distanceCm, latest - 1.0f);
    }
}

TEST(HandDistanceFilterTest, InvalidOrDistantReadingsHoldAndReturnAdoptsANewAbsolutePosition) {
    Filter filter;
    filter.observe(true, 18, 0);
    filter.observe(true, 57.32f, 70);
    filter.observe(true, 113.16f, 140);
    EXPECT_TRUE(filter.sample(249).valid);
    EXPECT_TRUE(filter.sample(250).valid);
    EXPECT_TRUE(filter.sample(250).holding);
    EXPECT_FLOAT_EQ(filter.sample(25000).distanceCm, 18);
    filter.observe(true, 39, 25070);
    filter.observe(true, 39, 25070); // Reading the same result twice is insufficient.
    EXPECT_TRUE(filter.sample(25070).holding);
    EXPECT_FLOAT_EQ(filter.sample(25070).distanceCm, 18);
    filter.observe(true, 39.2f, 25140);
    const auto recovered = filter.sample(25140);
    EXPECT_TRUE(recovered.valid);
    EXPECT_FALSE(recovered.holding);
    EXPECT_FLOAT_EQ(recovered.distanceCm, 39.2f);
    filter.observe(true, 40, 210); // Late old data must not rewind the tracker.
    EXPECT_EQ(filter.sample(25140).acceptedAtMs, 25140U);
    filter.observe(false, 0, 25210);
    EXPECT_TRUE(filter.sample(30000).valid);
    EXPECT_TRUE(filter.sample(30000).holding);
    EXPECT_FLOAT_EQ(filter.sample(30000).distanceCm, 39.2f);
}

TEST(HandDistanceFilterTest, InvalidSamplesBreakJumpConfirmationAndNeverInitialize) {
    Filter filter;
    uint32_t now = 0;
    for (float cm : {0.0f, 1.0f, 51.0f, std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity()}) {
        filter.observe(true, cm, now);
        EXPECT_FALSE(filter.sample(now).valid);
        now += 70;
    }
    filter.reset();
    filter.observe(true, 18, 0);
    filter.observe(true, 39, 70);
    filter.observe(false, 0, 140);
    filter.observe(true, 39, 210);
    EXPECT_FLOAT_EQ(filter.sample(210).distanceCm, 18);
    EXPECT_EQ(filter.sample(210).acceptedAtMs, 0U);
}

TEST(HandDistanceFilterTest, ClockRolloverAndCounterResetPreserveTheHeldValue) {
    Filter filter;
    const uint32_t start = UINT32_MAX - 100;
    filter.observe(true, 18, start);
    filter.observe(true, 39, start + 70);
    EXPECT_EQ(filter.sample(start + 70).heldJumps, 1U);
    filter.resetStatistics();
    EXPECT_TRUE(filter.sample(148).valid);
    EXPECT_EQ(filter.sample(148).heldJumps, 0U);
    EXPECT_FLOAT_EQ(filter.sample(148).distanceCm, 18);
    EXPECT_TRUE(filter.sample(149).valid);
    EXPECT_TRUE(filter.sample(149).holding);
    filter.reset();
    EXPECT_FALSE(filter.sample(200).valid);
}

TEST(HandDistanceFilterTest, LeavingFreezesTheCurrentOutputInsteadOfFinishingAnOldRamp) {
    Filter filter;
    filter.observe(true, 20, 0);
    filter.observe(true, 22, 70);
    filter.sample(100);
    filter.observe(false, 0, 105);
    const auto left = filter.sample(105);
    ASSERT_TRUE(left.holding);
    EXPECT_GT(left.distanceCm, 20);
    EXPECT_LT(left.distanceCm, 22);
    EXPECT_FLOAT_EQ(filter.sample(60000).distanceCm, left.distanceCm);
    filter.observe(true, 8, 60070);
    EXPECT_FLOAT_EQ(filter.sample(60070).distanceCm, left.distanceCm);
    filter.observe(true, 8.1f, 60140);
    EXPECT_FLOAT_EQ(filter.sample(60140).distanceCm, 8.1f);
    EXPECT_FALSE(filter.sample(60140).holding);
}
} // namespace
