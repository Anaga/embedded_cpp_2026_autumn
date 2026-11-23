#include <gtest/gtest.h>

#include "DemoEngine.h"

#include <limits>

namespace {
using Engine = sounder::DemoEngine;
using Mode = Engine::Mode;

void expectFrameBounds(sounder::Frame frame) {
    EXPECT_TRUE(std::isfinite(frame.hz));
    EXPECT_TRUE(std::isfinite(frame.drive));
    EXPECT_GE(frame.drive, 0.0f);
    EXPECT_LE(frame.drive, 1.000001f);
    if (frame.drive > 0.0f) {
        EXPECT_GE(frame.hz, 100.0f);
        EXPECT_LE(frame.hz, 8000.0f);
    }
}

TEST(DemoEngineTest, MidiToHz) {
    struct Row { unsigned note; float hz; };
    const Row rows[] = {{0, 8.175799f}, {45, 110.0f}, {60, 261.625565f},
                        {69, 440.0f}, {81, 880.0f}, {119, 7902.13282f}};
    for (const auto& row : rows) {
        SCOPED_TRACE(row.note);
        EXPECT_NEAR(sounder::midiToHz(row.note), row.hz, 0.01f);
    }
}

TEST(DemoEngineTest, RenderPattern) {
    // Numeric expectations describe the audible landmarks independently of the formulas.
    struct Row { size_t pattern; uint32_t ms; float hz; float drive; };
    const Row rows[] = {
        {0, 0, 261.6256f, 1}, {0, 329, 261.6256f, 1}, {0, 330, 261.6256f, 0},
        {0, 400, 293.6648f, 1}, {0, 2800, 523.2511f, 1}, {0, 5600, 261.6256f, 1},
        {1, 0, 100, 1}, {1, 1500, 894.4272f, 1}, {1, 3000, 8000, 1}, {1, 4500, 894.4272f, 1},
        {2, 0, 1000, 0.05f}, {2, 649, 1000, 0.05f}, {2, 650, 1000, 0},
        {2, 800, 1000, 0.10f}, {2, 4800, 1000, 1},
        {3, 0, 440, 1}, {3, 50, 452.893f, 1}, {3, 150, 427.4741f, 1},
        {4, 0, 440, 0.55f}, {4, 62, 440, 1}, {4, 125, 440, 0.55f}, {4, 187, 440, 0.10f},
        {5, 0, 880, 0}, {5, 50, 880, 0.5f}, {5, 100, 880, 1},
        {5, 200, 880, 0.7f}, {5, 300, 880, 0.4f}, {5, 649, 880, 0.4f},
        {5, 650, 880, 0.4f}, {5, 850, 880, 0.2f}, {5, 1050, 880, 0}, {5, 1200, 880, 0},
        {6, 0, 261.6256f, 1}, {6, 79, 261.6256f, 1}, {6, 80, 261.6256f, 0},
        {6, 100, 329.6276f, 1}, {6, 300, 523.2511f, 1}, {6, 600, 261.6256f, 1},
        {7, 0, 800, 1}, {8, 0, 440, 1}, {8, 199, 440, 1}, {8, 200, 440, 0},
        {8, 500, 440, 0.55f}, {8, 1000, 440, 0.30f}, {8, 1500, 440, 0.16f},
        {8, 2000, 440, 0.08f}, {8, 2500, 440, 0},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(testing::Message() << row.pattern << " at " << row.ms);
        const auto frame = sounder::renderPattern(row.pattern, row.ms);
        EXPECT_NEAR(frame.hz, row.hz, 0.02f);
        EXPECT_NEAR(frame.drive, row.drive, 0.0001f);
    }
    for (size_t i = 0; i < sounder::kPatternCount; ++i) {
        SCOPED_TRACE(i);
        for (uint32_t ms = 0; ms < sounder::kPatterns[i].durationMs; ++ms) {
            expectFrameBounds(sounder::renderPattern(i, ms));
        }
        for (uint32_t ms : {sounder::kPatterns[i].durationMs, UINT32_MAX}) {
            const auto frame = sounder::renderPattern(i, ms);
            EXPECT_FLOAT_EQ(frame.hz, 0);
            EXPECT_FLOAT_EQ(frame.drive, 0);
        }
    }
    EXPECT_FLOAT_EQ(sounder::renderPattern(9, 0).drive, 0);
    EXPECT_FLOAT_EQ(sounder::renderPattern(SIZE_MAX, 0).drive, 0);
}

TEST(DemoEngineTest, Stop) {
    Engine local;
    for (unsigned mode = 0; mode < 4; ++mode) {
        SCOPED_TRACE(mode);
        if (mode == 1) local.startTone(100, 440);
        if (mode == 2) local.startPattern(100, 0);
        if (mode == 3) local.startTour(100);
        local.stop();
        EXPECT_EQ(local.mode(), Mode::Idle);
        EXPECT_FLOAT_EQ(local.sample(200).drive, 0);
        local.stop();
        EXPECT_EQ(local.mode(), Mode::Idle);
    }
}

TEST(DemoEngineTest, StartTone) {
    const float frequencies[] = {0, -440, 99.999f, 100, 440, 8000, 8000.1f,
                                std::numeric_limits<float>::quiet_NaN(),
                                std::numeric_limits<float>::infinity()};
    for (float hz : frequencies) {
        SCOPED_TRACE(hz);
        Engine local;
        local.startTour(0);
        local.startTone(200, hz, 1000);
        const bool valid = std::isfinite(hz) && hz >= 100 && hz <= 8000;
        EXPECT_EQ(local.mode(), valid ? Mode::Tone : Mode::Idle);
        const auto frame = local.sample(200);
        EXPECT_FLOAT_EQ(frame.drive, valid ? 1 : 0);
        if (valid) EXPECT_FLOAT_EQ(frame.hz, hz);
    }
    Engine local;
    local.startTone(100, 440);
    EXPECT_FLOAT_EQ(local.sample(60100).drive, 1);
    local.startTone(60100, 880, 10);
    EXPECT_FLOAT_EQ(local.sample(60109).hz, 880);
    EXPECT_FLOAT_EQ(local.sample(60110).drive, 0);
}

TEST(DemoEngineTest, StartPattern) {
    for (size_t index : {size_t{0}, size_t{4}, size_t{8}, size_t{9}, SIZE_MAX}) {
        SCOPED_TRACE(index);
        Engine local;
        local.startTone(0, 440);
        local.startPattern(100, index);
        EXPECT_EQ(local.mode(), index < 9 ? Mode::Pattern : Mode::Idle);
        if (index < 9) {
            EXPECT_EQ(local.pattern(), index);
            EXPECT_FLOAT_EQ(local.sample(100).drive, sounder::renderPattern(index, 0).drive);
        } else {
            EXPECT_FLOAT_EQ(local.sample(100).drive, 0);
        }
    }
}

TEST(DemoEngineTest, StartTour) {
    Engine local;
    local.startPattern(0, 8);
    local.startTour(100);
    EXPECT_EQ(local.mode(), Mode::Tour);
    EXPECT_EQ(local.pattern(), 0U);
    EXPECT_NEAR(local.sample(100).hz, 261.6256f, 0.01f);
    local.sample(13100);
    ASSERT_EQ(local.pattern(), 2U);
    local.startTour(20000);
    EXPECT_EQ(local.pattern(), 0U);
    EXPECT_FLOAT_EQ(local.sample(20000).drive, 1);

    const uint32_t start = UINT32_MAX - 100;
    local.startTour(start, 5);
    EXPECT_EQ(local.mode(), Mode::Showcase);
    ASSERT_EQ(local.tourIndex(), 0U);
    EXPECT_NEAR(local.sample(start + 45200).hz, 261.6256f, 0.01f);
    EXPECT_EQ(local.tourIndex(), 1U);
    EXPECT_EQ(local.pattern(), 0U);
    // A delayed sample skips expired patterns and tours, including across rollover.
    EXPECT_FLOAT_EQ(local.sample(start + 4 * 45200 + 6000).hz, 100);
    EXPECT_EQ(local.tourIndex(), 4U);
    EXPECT_EQ(local.pattern(), 1U);
    EXPECT_FLOAT_EQ(local.sample(start + 5 * 45200).drive, 0);
    EXPECT_EQ(local.mode(), Mode::Idle);
}

TEST(DemoEngineTest, Sample) {
    Engine local;
    EXPECT_FLOAT_EQ(local.sample(0).hz, 0);
    EXPECT_FLOAT_EQ(local.sample(UINT32_MAX).drive, 0);
    const uint32_t starts[] = {0, 1000, UINT32_MAX - 100};
    const uint32_t durations[] = {6000, 6000, 5600, 5000, 5000, 4800, 4800, 5000, 3000};
    for (uint32_t start : starts) {
        SCOPED_TRACE(start);
        for (size_t i = 0; i < 9; ++i) {
            local.startPattern(start, i);
            expectFrameBounds(local.sample(start + durations[i] - 1));
            EXPECT_EQ(local.mode(), Mode::Pattern);
            EXPECT_FLOAT_EQ(local.sample(start + durations[i]).drive, 0);
            EXPECT_EQ(local.mode(), Mode::Idle);
            EXPECT_FLOAT_EQ(local.sample(start + durations[i] + 1).drive, 0);
        }
        local.startTone(start, 440, 1000);
        EXPECT_FLOAT_EQ(local.sample(start + 999).drive, 1);
        EXPECT_FLOAT_EQ(local.sample(start + 1000).drive, 0);
        local.startTone(start, 440);
        EXPECT_FLOAT_EQ(local.sample(start + 60000).drive, 1);

        local.startTour(start);
        uint32_t elapsed = 0;
        for (size_t i = 0; i < 9; ++i) {
            expectFrameBounds(local.sample(start + elapsed));
            EXPECT_EQ(local.pattern(), i);
            EXPECT_EQ(local.mode(), Mode::Tour);
            expectFrameBounds(local.sample(start + elapsed + durations[i] - 1));
            EXPECT_EQ(local.pattern(), i);
            elapsed += durations[i];
        }
        EXPECT_EQ(elapsed, 45200U);
        EXPECT_FLOAT_EQ(local.sample(start + 45200).drive, 0);
        EXPECT_EQ(local.mode(), Mode::Idle);
        local.startTour(start);
        expectFrameBounds(local.sample(start + 13000));
        EXPECT_EQ(local.pattern(), 2U); // A late sample skips expired sections.
        EXPECT_FLOAT_EQ(local.sample(start + 45201).drive, 0);
    }
}

TEST(DemoEngineTest, Mode) {
    Engine local;
    EXPECT_EQ(local.mode(), Mode::Idle);
    local.startTone(0, 440);
    EXPECT_EQ(local.mode(), Mode::Tone);
    local.startPattern(0, 8);
    EXPECT_EQ(local.mode(), Mode::Pattern);
    local.startTour(0);
    EXPECT_EQ(local.mode(), Mode::Tour);
    local.stop();
    EXPECT_EQ(local.mode(), Mode::Idle);
}

TEST(DemoEngineTest, Pattern) {
    Engine local;
    EXPECT_EQ(local.pattern(), 0U);
    for (size_t i : {size_t{0}, size_t{4}, size_t{8}}) {
        local.startPattern(0, i);
        EXPECT_EQ(local.pattern(), i);
        local.stop();
        EXPECT_EQ(local.pattern(), i);
    }
    local.startTour(0);
    EXPECT_EQ(local.pattern(), 0U);
    local.sample(6000);
    EXPECT_EQ(local.pattern(), 1U);
}

} // namespace
