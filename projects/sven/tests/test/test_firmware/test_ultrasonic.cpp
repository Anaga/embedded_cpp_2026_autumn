#include <gtest/gtest.h>

#include "UltrasonicDemo.h"

#include <algorithm>
#include <limits>

namespace {
using Sensor = sounder::UltrasonicSensor;
using Demo = sounder::UltrasonicDemo;
using Status = Sensor::Status;

class UltrasonicTest : public testing::Test {
protected:
    void SetUp() override {
        fake_arduino::now = 0;
        fake_arduino::resetGpio();
        fake_arduino::delays.clear();
        Serial = {};
        Serial.connected = true;
    }
    void TearDown() override { EXPECT_EQ(fake_rtos::lockDepth, 0); }

    static void echo(uint8_t pin, uint32_t pulseUs) {
        delayMicroseconds(100);
        fake_arduino::edge(pin, HIGH);
        delayMicroseconds(pulseUs);
        fake_arduino::edge(pin, LOW);
    }

    static size_t triggers(uint8_t pin) {
        return std::count_if(fake_arduino::writes.begin(), fake_arduino::writes.end(),
                             [pin](const auto& write) { return write.pin == pin && write.value == HIGH; });
    }
};

TEST_F(UltrasonicTest, CapturesPulseWithoutBlockingAndRejectsBusyTrigger) {
    Sensor sensor(32, 34);
    EXPECT_FALSE(sensor.trigger());
    sensor.begin();
    EXPECT_EQ(fake_arduino::modes[32], OUTPUT);
    EXPECT_EQ(fake_arduino::modes[34], INPUT);
    ASSERT_TRUE(sensor.trigger());
    EXPECT_EQ(micros(), 12U);
    EXPECT_EQ(fake_arduino::writes.back().atUs - fake_arduino::writes[2].atUs, 10U);
    EXPECT_FALSE(sensor.trigger());
    EXPECT_FALSE(sensor.poll());
    echo(34, 1000);
    ASSERT_TRUE(sensor.poll());
    EXPECT_EQ(sensor.reading().status, Status::Valid);
    EXPECT_EQ(sensor.reading().pulseUs, 1000U);
    EXPECT_NEAR(sensor.reading().distanceCm, 17.171, 0.001);
    EXPECT_FALSE(sensor.poll());
    EXPECT_FALSE(sensor.busy());
    EXPECT_TRUE(fake_arduino::delays.empty());
}

TEST_F(UltrasonicTest, MissingAndIncompleteEchoesExpireWithoutReusingDistance) {
    Sensor sensor(32, 34);
    sensor.begin();
    sensor.trigger();
    echo(34, 2000);
    ASSERT_TRUE(sensor.poll());
    EXPECT_EQ(sensor.reading().status, Status::Valid);
    for (bool risingEdge : {false, true}) {
        ASSERT_TRUE(sensor.trigger());
        if (risingEdge) fake_arduino::edge(34, HIGH);
        delayMicroseconds(Sensor::kTimeoutUs - 13);
        EXPECT_FALSE(sensor.poll());
        delayMicroseconds(1);
        ASSERT_TRUE(sensor.poll());
        EXPECT_EQ(sensor.reading().status, Status::Timeout);
        EXPECT_FLOAT_EQ(sensor.reading().distanceCm, 0);
        EXPECT_EQ(sensor.reading().pulseUs, 0U);
        fake_arduino::edge(34, LOW);
        EXPECT_FALSE(sensor.poll());
    }
}

TEST_F(UltrasonicTest, RejectsStuckHighAndOutOfRangePulses) {
    Sensor sensor(32, 34);
    sensor.begin();
    fake_arduino::edge(34, HIGH);
    ASSERT_TRUE(sensor.trigger());
    ASSERT_TRUE(sensor.poll());
    EXPECT_EQ(sensor.reading().status, Status::EchoHigh);
    EXPECT_EQ(triggers(32), 0U);
    fake_arduino::edge(34, LOW);
    for (uint32_t pulse : {50U, 27000U}) {
        sensor.trigger();
        echo(34, pulse);
        ASSERT_TRUE(sensor.poll());
        EXPECT_EQ(sensor.reading().status, Status::OutOfRange);
        EXPECT_EQ(sensor.reading().pulseUs, pulse);
    }
    // An echo arriving after the deadline is ignored even if poll was delayed.
    sensor.trigger();
    echo(34, 31000);
    ASSERT_TRUE(sensor.poll());
    EXPECT_EQ(sensor.reading().status, Status::Timeout);
}

TEST_F(UltrasonicTest, TemperatureCompensationAndMicrosRollover) {
    Sensor sensor(32, 34);
    sensor.begin();
    EXPECT_TRUE(sensor.setTemperatureC(0));
    EXPECT_FALSE(sensor.setTemperatureC(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(sensor.setTemperatureC(61));
    // Place a measurement across the 32-bit micros rollover.
    fake_arduino::now = UINT32_MAX / 1000;
    fake_arduino::microsecondRemainder = UINT32_MAX % 1000 - 200;
    sensor.trigger();
    echo(34, 1000);
    ASSERT_TRUE(sensor.poll());
    EXPECT_EQ(sensor.reading().status, Status::Valid);
    EXPECT_EQ(sensor.reading().pulseUs, 1000U);
    EXPECT_NEAR(sensor.reading().distanceCm, 16.565, 0.001);
    sensor.trigger();
    delayMicroseconds(30000);
    EXPECT_TRUE(sensor.poll());
    EXPECT_EQ(sensor.reading().status, Status::Timeout);
    sensor.end();
    EXPECT_EQ(fake_arduino::interrupts[34].handler, nullptr);
    EXPECT_FALSE(sensor.trigger());
}

TEST_F(UltrasonicTest, AlternatesSensorsAndTracksOnlyValidExtrema) {
    Demo demo;
    demo.update();
    EXPECT_TRUE(fake_arduino::writes.empty());
    demo.start();
    EXPECT_EQ(fake_arduino::modes[32], OUTPUT);
    EXPECT_EQ(fake_arduino::modes[33], OUTPUT);
    EXPECT_EQ(fake_arduino::modes[34], INPUT);
    EXPECT_EQ(fake_arduino::modes[35], INPUT);
    EXPECT_NE(fake_arduino::interrupts[34].handler, nullptr);
    EXPECT_NE(fake_arduino::interrupts[35].handler, nullptr);
    EXPECT_EQ(fake_arduino::interrupts[32].handler, nullptr);
    EXPECT_EQ(fake_arduino::interrupts[33].handler, nullptr);
    EXPECT_EQ(fake_arduino::interrupts[14].handler, nullptr);
    fake_arduino::now = 199;
    demo.update();
    EXPECT_EQ(triggers(32), 0U);
    fake_arduino::now = 200;
    demo.update();
    EXPECT_EQ(triggers(32), 1U);
    EXPECT_EQ(triggers(33), 0U);
    echo(34, 1000);
    demo.update();
    EXPECT_EQ(demo.pitchStatistics().valid, 1U);
    EXPECT_FALSE(demo.pitchHandReading().valid); // Raw diagnostics do not run hand tracking.
    fake_arduino::now = 300;
    demo.update();
    EXPECT_EQ(triggers(33), 1U);
    delayMicroseconds(30000);
    demo.update();
    EXPECT_EQ(demo.volumeStatistics().attempts, 1U);
    EXPECT_EQ(demo.volumeStatistics().valid, 0U);
    EXPECT_NE(Serial.output.find("volume timeout | echo 0 us | distance n/a"), std::string::npos);
    fake_arduino::now = 400;
    demo.update();
    echo(34, 2000);
    demo.update();
    EXPECT_EQ(demo.pitchStatistics().attempts, 2U);
    EXPECT_NEAR(demo.pitchStatistics().minCm, 17.171, 0.001);
    EXPECT_NEAR(demo.pitchStatistics().maxCm, 34.342, 0.001);
    demo.handleCommand("sensors", "reset", nullptr);
    EXPECT_EQ(demo.pitchStatistics().attempts, 0U);
    EXPECT_TRUE(demo.active());
    demo.stop();
    EXPECT_EQ(fake_arduino::interrupts[34].handler, nullptr);
    EXPECT_EQ(fake_arduino::interrupts[35].handler, nullptr);
    EXPECT_EQ(std::count_if(fake_arduino::writes.begin(), fake_arduino::writes.end(),
                           [](const auto& write) { return write.pin == 34 || write.pin == 35; }), 0);
    fake_arduino::now = 10000;
    demo.update();
    EXPECT_EQ(triggers(32), 2U);
}

TEST_F(UltrasonicTest, SingleSensorAndDelayedLoopsDoNotBurstOrOverlap) {
    for (auto selection : {Demo::Selection::Pitch, Demo::Selection::Volume}) {
        Demo demo;
        fake_arduino::resetGpio();
        fake_arduino::now = 0;
        demo.start(selection);
        Serial.output.clear();
        fake_arduino::now = 200;
        demo.update();
        fake_arduino::now = 300;
        demo.update(); // First attempt times out; this slot remains quiet.
        EXPECT_EQ(std::count(Serial.output.begin(), Serial.output.end(), '\n'), 1);
        EXPECT_NE(Serial.output.find(selection == Demo::Selection::Pitch ? "pitch timeout" : "volume timeout"),
                  std::string::npos);
        EXPECT_EQ(Serial.output.find(selection == Demo::Selection::Pitch ? "volume" : "pitch"), std::string::npos);
        const uint8_t pin = selection == Demo::Selection::Pitch ? 32 : 33;
        EXPECT_EQ(triggers(pin), 1U);
        fake_arduino::now = 400;
        demo.update();
        EXPECT_EQ(triggers(pin), 2U);
        fake_arduino::now = 10000;
        demo.update();
        demo.update();
        EXPECT_LE(triggers(pin), 3U);
        EXPECT_EQ(triggers(pin == 32 ? 33 : 32), 0U);
    }
    // Millisecond rollover must not stall the schedule.
    Demo demo;
    fake_arduino::now = UINT32_MAX - 50;
    demo.start();
    const auto before = triggers(32);
    fake_arduino::now = 149;
    demo.update();
    EXPECT_EQ(triggers(32), before + 1);
}

TEST_F(UltrasonicTest, LimitsLoggingWhileCountingEveryAttemptAndExtremum) {
    Demo demo;
    demo.start();
    Serial.output.clear();
    for (uint32_t slotMs = 200; slotMs <= 1300; slotMs += 100) {
        fake_arduino::now = slotMs;
        fake_arduino::microsecondRemainder = 0;
        demo.update();
        if (slotMs == 500) {
            delayMicroseconds(30000); // A suppressed failure still counts.
        } else {
            const bool pitch = slotMs % 200 == 0;
            echo(pitch ? 34 : 35, slotMs == 400 ? 2000 : slotMs == 600 ? 500 : 1000);
        }
        demo.update();
        if (slotMs == 200) EXPECT_TRUE(Serial.output.empty()); // Wait for volume before the first line.
        if (slotMs == 1100) {
            EXPECT_EQ(std::count(Serial.output.begin(), Serial.output.end(), '\n'), 1);
            const auto end = Serial.output.find('\n');
            EXPECT_LT(Serial.output.find("pitch ok"), end);
            EXPECT_LT(Serial.output.find(" || volume ok"), end);
            EXPECT_EQ(demo.pitchStatistics().attempts, 5U);
            EXPECT_EQ(demo.volumeStatistics().attempts, 5U);
        }
    }
    EXPECT_EQ(std::count(Serial.output.begin(), Serial.output.end(), '\n'), 2);
    EXPECT_EQ(demo.pitchStatistics().attempts, 6U);
    EXPECT_EQ(demo.pitchStatistics().valid, 6U);
    EXPECT_EQ(demo.volumeStatistics().attempts, 6U);
    EXPECT_EQ(demo.volumeStatistics().valid, 5U);
    EXPECT_NEAR(demo.pitchStatistics().minCm, 8.5855, 0.001);
    EXPECT_NEAR(demo.pitchStatistics().maxCm, 34.342, 0.001);
    EXPECT_NE(Serial.output.find("valid 6/6 | min 8.59 max 34.34 cm"), std::string::npos);
    EXPECT_NE(Serial.output.find("valid 5/6"), std::string::npos);
}

TEST_F(UltrasonicTest, LiveTimingSamplesEachSensorEverySeventyMilliseconds) {
    for (auto selection : {Demo::Selection::Both, Demo::Selection::Pitch, Demo::Selection::Volume}) {
        SCOPED_TRACE(static_cast<int>(selection));
        Demo demo;
        fake_arduino::resetGpio();
        fake_arduino::now = 0;
        demo.start(selection, Demo::Timing::Live);
        ASSERT_EQ(demo.measurementIntervalMs(), 70U);
        fake_arduino::now = 199;
        demo.update();
        EXPECT_EQ(triggers(32) + triggers(33), 0U);
        for (uint32_t slot = 0; slot < 6; ++slot) {
            fake_arduino::now = 200 + 35 * slot;
            fake_arduino::microsecondRemainder = 0;
            const auto before = triggers(32) + triggers(33);
            demo.update();
            const bool pitch = selection == Demo::Selection::Pitch ||
                (selection == Demo::Selection::Both && slot % 2 == 0);
            if (selection == Demo::Selection::Both || slot % 2 == 0) {
                ASSERT_EQ(triggers(32) + triggers(33), before + 1);
                echo(pitch ? 34 : 35, 1000);
                demo.update();
            } else {
                EXPECT_EQ(triggers(32) + triggers(33), before);
            }
            // Finishing early does not permit a second ping before the slot.
            fake_arduino::now = 200 + 35 * slot + 34;
            demo.update();
            EXPECT_EQ(triggers(32) + triggers(33), before +
                      (selection == Demo::Selection::Both || slot % 2 == 0 ? 1 : 0));
        }
        EXPECT_EQ(triggers(32), selection == Demo::Selection::Volume ? 0U : 3U);
        EXPECT_EQ(triggers(33), selection == Demo::Selection::Pitch ? 0U : 3U);
        EXPECT_EQ(demo.pitchHandReading().valid, selection != Demo::Selection::Volume);
        EXPECT_EQ(demo.volumeHandReading().valid, selection != Demo::Selection::Pitch);
        demo.handleCommand("sensors", "status", nullptr);
        EXPECT_NE(Serial.output.find("70 ms/sensor"), std::string::npos);
        demo.start(); // Diagnostic commands restore the conservative schedule.
        EXPECT_EQ(demo.measurementIntervalMs(), 200U);
        EXPECT_FALSE(demo.pitchHandReading().valid);
        EXPECT_FALSE(demo.volumeHandReading().valid);
    }
}
} // namespace
