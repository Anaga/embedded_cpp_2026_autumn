#include <gtest/gtest.h>

#include "LiveTone.h"
#include "ThereminController.h"

#include <limits>
#include <utility>

namespace {
using Controller = sounder::ThereminController;
using Mode = Controller::Mode;

TEST(ThereminControllerTest, PitchAndVolumeLandmarks) {
    Controller controller;
    for (const auto& row : {std::pair<float, float>{5, 880}, {27.5f, 440}, {50, 220}}) {
        controller.start(Mode::Pitch);
        const auto target = controller.update(100, {true, row.first, 100}, {});
        EXPECT_NEAR(target.hz, row.second, 0.001);
        EXPECT_FLOAT_EQ(target.drive, 1);
    }
    for (const auto& row : {std::pair<float, float>{2, 1}, {5, 1}, {27.5f, 0.5f}, {50, 0}}) {
        controller.start(Mode::Volume);
        const auto target = controller.update(100, {}, {true, row.first, 100});
        EXPECT_FLOAT_EQ(target.hz, 440);
        EXPECT_FLOAT_EQ(target.drive, row.second);
    }
}

TEST(ThereminControllerTest, BothInputsRequiredAndMissingReadingsExpire) {
    Controller controller;
    controller.start(Mode::Both);
    EXPECT_FLOAT_EQ(controller.update(0, {}, {}).drive, 0);
    EXPECT_FLOAT_EQ(controller.update(0, {true, 5, 0}, {}).drive, 0);
    EXPECT_FLOAT_EQ(controller.update(0, {}, {true, 5, 0}).drive, 1);
    const auto held = controller.update(249, {}, {});
    EXPECT_FLOAT_EQ(held.hz, 880);
    EXPECT_FLOAT_EQ(held.drive, 1);
    EXPECT_FLOAT_EQ(controller.update(250, {}, {}).drive, 0);
    // Recovering only one hand is insufficient after the other has expired.
    EXPECT_FLOAT_EQ(controller.update(350, {true, 27.5f, 350}, {}).drive, 0);
    const auto recovered = controller.update(450, {}, {true, 27.5f, 450});
    EXPECT_FLOAT_EQ(recovered.drive, 0.5f);
    EXPECT_FLOAT_EQ(recovered.hz, 440);
    controller.start(Mode::Both);
    EXPECT_FLOAT_EQ(controller.update(451, {}, {}).drive, 0);
    controller.stop();
    EXPECT_EQ(controller.mode(), Mode::Off);
    EXPECT_FLOAT_EQ(controller.update(452, {true, 5, 452}, {true, 5, 452}).drive, 0);
}

TEST(ThereminControllerTest, InvalidOrDistantTargetsDoNotRefreshHeldControls) {
    Controller controller;
    for (float distance : {1.0f, 51.0f, 400.0f, std::numeric_limits<float>::quiet_NaN(),
                           std::numeric_limits<float>::infinity()}) {
        controller.start(Mode::Pitch);
        controller.update(100, {true, 5, 100}, {});
        EXPECT_FLOAT_EQ(controller.update(349, {true, distance, 349}, {}).drive, 1);
        EXPECT_FLOAT_EQ(controller.update(350, {true, distance, 350}, {}).drive, 0);
    }
    controller.start(Mode::Pitch);
    const uint32_t measured = UINT32_MAX - 100;
    EXPECT_FLOAT_EQ(controller.update(measured, {true, 5, measured}, {}).drive, 1);
    EXPECT_FLOAT_EQ(controller.update(148, {}, {}).drive, 1);
    EXPECT_FLOAT_EQ(controller.update(149, {}, {}).drive, 0);
}

TEST(LiveToneTest, GlidesPitchAndFadesGainWithoutRestartingIdenticalTargets) {
    sounder::LiveTone tone;
    tone.setTarget({880, 1}, 100);
    EXPECT_FLOAT_EQ(tone.sample(100).drive, 0);
    EXPECT_FLOAT_EQ(tone.sample(105).hz, 550);
    EXPECT_FLOAT_EQ(tone.sample(105).drive, 1.0f / 3.0f);
    tone.setTarget({880, 1}, 110);
    EXPECT_FLOAT_EQ(tone.sample(115).drive, 1);
    EXPECT_FLOAT_EQ(tone.sample(120).hz, 880);
    EXPECT_FLOAT_EQ(tone.sample(120).drive, 1);
    tone.setTarget({220, 0}, 120);
    EXPECT_NEAR(tone.sample(125).drive, 2.0f / 3.0f, 0.000001f);
    EXPECT_FLOAT_EQ(tone.sample(135).drive, 0);
    EXPECT_FLOAT_EQ(tone.sample(140).hz, 220);
    tone.setTarget({std::numeric_limits<float>::quiet_NaN(), 1}, 140);
    EXPECT_FLOAT_EQ(tone.sample(160).hz, 440);
    EXPECT_FLOAT_EQ(tone.sample(160).drive, 0);
}

TEST(LiveToneTest, MidGlideReplacementAndClockRolloverRemainContinuous) {
    sounder::LiveTone tone;
    const uint32_t start = UINT32_MAX - 5;
    tone.setTarget({880, 1}, start);
    EXPECT_FLOAT_EQ(tone.sample(start + 5).drive, 1.0f / 3.0f);
    const auto before = tone.sample(start + 10);
    tone.setTarget({220, 0}, start + 10);
    EXPECT_FLOAT_EQ(tone.sample(start + 10).hz, before.hz);
    EXPECT_FLOAT_EQ(tone.sample(start + 10).drive, before.drive);
    EXPECT_FLOAT_EQ(tone.sample(start + 30).hz, 220);
    EXPECT_FLOAT_EQ(tone.sample(start + 30).drive, 0);
}

TEST(VibratoTest, UsesSameModulationAroundAnyCenterPitch) {
    for (float center : {220.0f, 440.0f, 880.0f}) {
        EXPECT_NEAR(sounder::Vibrato::frequency(center, 0), center, 0.001);
        EXPECT_NEAR(sounder::Vibrato::frequency(center, 50), center * 1.0293022f, 0.001);
        EXPECT_NEAR(sounder::Vibrato::frequency(center, 150), center * 0.9715319f, 0.001);
        EXPECT_NEAR(sounder::Vibrato::frequency(center, 200), center, 0.001);
    }
}
} // namespace
