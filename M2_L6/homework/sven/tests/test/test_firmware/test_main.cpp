#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Test Sven's firmware directly. Compile it only here.
#include "../../../src/main.cpp"

namespace {
namespace hw = fake_arduino;

class FirmwareTest : public ::testing::Test {
protected:
    void SetUp() override {
        hw::reset();
        Serial = FakeSerial{};
        g_last_update_ms = 0;
    }
};

TEST_F(FirmwareTest, ReadPotentiometer) {
    struct Row {
        const char* name;
        std::array<uint16_t, 16> samples;
        uint16_t expected;
    };
    const Row rows[] = {
        {"zero", {}, 0},
        {"calibrated maximum", {3850, 3850, 3850, 3850, 3850, 3850, 3850, 3850,
                                 3850, 3850, 3850, 3850, 3850, 3850, 3850, 3850}, 3850},
        {"12-bit maximum", {4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
                            4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095}, 4095},
        {"midpoint", {2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
                      2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048}, 2048},
        {"fraction truncates", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, 7},
        {"alternating extremes", {0, 4095, 0, 4095, 0, 4095, 0, 4095,
                                 0, 4095, 0, 4095, 0, 4095, 0, 4095}, 2047},
        {"below first whole count", {15}, 0},
        {"first whole count", {16}, 1},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(row.name);
        SetUp();
        hw::time_us = 10250;
        hw::analog_inputs.assign(row.samples.begin(), row.samples.end());
        EXPECT_EQ(readPotentiometer(), row.expected);
        EXPECT_EQ(hw::analog_read_pins, std::vector<uint8_t>(16, 4));
        ASSERT_EQ(hw::analog_read_times.size(), 16U);
        for (size_t i = 0; i < 16; ++i) {
            SCOPED_TRACE(i);
            EXPECT_EQ(hw::analog_read_times[i], 10250U + i * 200U);
        }
        EXPECT_EQ(hw::microsecond_delays, std::vector<uint32_t>(16, 200));
        EXPECT_EQ(hw::time_us, 13450U);  // Includes the delay after the last sample.
        EXPECT_TRUE(hw::millisecond_delays.empty());
        EXPECT_TRUE(hw::pin_writes.empty());
        EXPECT_TRUE(Serial.output.empty());
        EXPECT_EQ(g_last_update_ms, 0U);
    }
}

TEST_F(FirmwareTest, MapToCelsius) {
    struct Row { uint16_t counts; int16_t celsius; };
    // Literal checkpoints for the -40..140 range, including
    // truncation before the negative offset and saturation above 3850 counts.
    const Row rows[] = {
        {0, -40}, {1, -40}, {21, -40}, {22, -39}, {213, -31}, {214, -30},
        {834, -2}, {835, -1}, {855, -1}, {856, 0}, {857, 0},
        {1924, 49}, {1925, 50}, {1926, 50}, {2138, 59}, {2139, 60},
        {2780, 89}, {2781, 90}, {3208, 109}, {3209, 110},
        {3849, 139}, {3850, 140}, {3851, 140}, {4095, 140}, {65535, 140},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(row.counts);
        SetUp();
        EXPECT_EQ(mapToCelsius(row.counts), row.celsius);
        EXPECT_EQ(hw::time_us, 0U);
        EXPECT_TRUE(hw::analog_read_pins.empty());
        EXPECT_TRUE(hw::pin_writes.empty());
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, SetColour) {
    struct Row { uint8_t colour; uint8_t red; uint8_t green; uint8_t blue; };
    const Row rows[] = {
        {0x00, HIGH, HIGH, HIGH}, {0x01, LOW, HIGH, HIGH},
        {0x02, HIGH, LOW, HIGH}, {0x03, LOW, LOW, HIGH},
        {0x04, HIGH, HIGH, LOW}, {0x05, LOW, HIGH, LOW},
        {0x06, HIGH, LOW, LOW}, {0x07, LOW, LOW, LOW},
        {0x08, HIGH, HIGH, HIGH}, {0x80, HIGH, HIGH, HIGH},
        {0x81, LOW, HIGH, HIGH}, {0xFA, HIGH, LOW, HIGH},
        {0xF8, HIGH, HIGH, HIGH}, {0xFF, LOW, LOW, LOW},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(static_cast<unsigned>(row.colour));
        SetUp();
        setColour(row.colour);
        const std::vector<hw::PinWrite> expected = {
            {5, row.red, 0}, {6, row.green, 0}, {7, row.blue, 0},
        };
        EXPECT_EQ(hw::pin_writes, expected);
        EXPECT_EQ(hw::pin_levels[5], row.red);
        EXPECT_EQ(hw::pin_levels[6], row.green);
        EXPECT_EQ(hw::pin_levels[7], row.blue);
        EXPECT_TRUE(hw::pin_modes.empty());
        EXPECT_EQ(hw::time_us, 0U);
        EXPECT_TRUE(Serial.output.empty());
    }
}

TEST_F(FirmwareTest, PrintBinary) {
    struct Row { uint8_t value; const char* text; };
    const Row rows[] = {
        {0, "00000000"}, {1, "00000001"}, {2, "00000010"}, {3, "00000011"},
        {4, "00000100"}, {7, "00000111"}, {15, "00001111"}, {16, "00010000"},
        {31, "00011111"}, {63, "00111111"}, {64, "01000000"}, {85, "01010101"},
        {127, "01111111"}, {128, "10000000"}, {170, "10101010"},
        {254, "11111110"}, {255, "11111111"},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(static_cast<unsigned>(row.value));
        SetUp();
        Serial.output = "prefix:";
        printBinary(row.value);
        EXPECT_EQ(Serial.output, std::string("prefix:") + row.text);
        EXPECT_EQ(hw::time_us, 0U);
        EXPECT_TRUE(hw::pin_writes.empty());
    }
}

TEST_F(FirmwareTest, ColourForTemperature) {
    // TODO 1's README bands, including each boundary and the int16_t extremes.
    // Expected masks: blue 0x04, cyan 0x06, green 0x02, yellow 0x03, red 0x01.
    struct Row { int16_t celsius; uint8_t colour; };
    const Row rows[] = {
        {-32768, 0x04}, {-40, 0x04}, {-31, 0x04}, {-30, 0x04}, {-29, 0x04}, {-1, 0x04},
        {0, 0x06}, {1, 0x06}, {30, 0x06}, {59, 0x06},
        {60, 0x02}, {61, 0x02}, {75, 0x02}, {89, 0x02},
        {90, 0x03}, {91, 0x03}, {100, 0x03}, {109, 0x03}, {110, 0x01}, {111, 0x01},
        {119, 0x01}, {120, 0x01}, {121, 0x01}, {140, 0x01}, {32767, 0x01},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(row.celsius);
        SetUp();
        EXPECT_EQ(colourForTemperature(row.celsius), row.colour);
        EXPECT_TRUE(hw::pin_writes.empty());
        EXPECT_TRUE(hw::analog_read_pins.empty());
        EXPECT_TRUE(Serial.output.empty());
        EXPECT_EQ(hw::time_us, 0U);
        EXPECT_EQ(g_last_update_ms, 0U);
    }
}

TEST_F(FirmwareTest, Setup) {
    struct Row { uint64_t start_us; uint32_t last_update; const char* prefix; };
    const Row rows[] = {
        {0, 0, ""}, {777123, 37, "previous output\r\n"},
        {4294966795123ULL, 4294967285U, ""},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(row.start_us);
        SetUp();
        hw::time_us = row.start_us;
        hw::pin_levels.fill(LOW);
        g_last_update_ms = row.last_update;
        Serial.output = row.prefix;
        setup();
        const std::vector<std::pair<uint32_t, uint64_t>> expected_begins = {
            {115200, row.start_us},
        };
        EXPECT_EQ(Serial.begin_calls, expected_begins);
        EXPECT_EQ(hw::millisecond_delays, std::vector<uint32_t>{1500});
        EXPECT_TRUE(hw::microsecond_delays.empty());
        EXPECT_EQ(hw::time_us, row.start_us + 1500000);
        const std::vector<hw::PinSetting> expected_modes = {
            {5, OUTPUT}, {6, OUTPUT}, {7, OUTPUT},
        };
        EXPECT_EQ(hw::pin_modes, expected_modes);
        const std::vector<hw::PinWrite> expected_writes = {
            {5, HIGH, row.start_us + 1500000},
            {6, HIGH, row.start_us + 1500000},
            {7, HIGH, row.start_us + 1500000},
        };
        EXPECT_EQ(hw::pin_writes, expected_writes);
        EXPECT_EQ(hw::pin_levels[5], HIGH);
        EXPECT_EQ(hw::pin_levels[6], HIGH);
        EXPECT_EQ(hw::pin_levels[7], HIGH);
        EXPECT_EQ(hw::adc_resolutions, std::vector<uint8_t>{12});
        const std::vector<hw::PinSetting> expected_attenuations = {{4, ADC_11db}};
        EXPECT_EQ(hw::adc_attenuations, expected_attenuations);
        EXPECT_TRUE(hw::analog_read_pins.empty());
        EXPECT_EQ(g_last_update_ms, row.last_update);  // setup does not reset it.
        EXPECT_EQ(Serial.output, std::string(row.prefix) +
                  "\r\nLesson 06 homework - the colour thermometer\r\nTurn the knob.\r\n");
    }
}

TEST_F(FirmwareTest, Loop) {
    struct Row {
        uint32_t last;
        uint64_t now_ms;
        uint16_t counts;
        bool updates;
        const char* text;
        std::array<uint8_t, 3> levels;  // Red, green, blue; LOW switches a channel on.
    };
    const Row rows[] = {
        {0, 0, 4095, false, ""}, {0, 1, 4095, false, ""},
        {0, 249, 4095, false, ""},
        {0, 250, 0, true, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {0, 251, 4095, true, "C = +140   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        {1000, 1249, 0, false, ""},
        {1000, 1250, 1925, true, "C =  +50   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {1000, 1251, 3850, true, "C = +140   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        {0, 250, 1, true, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {0, 250, 21, true, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {0, 250, 22, true, "C =  -39   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {0, 250, 855, true, "C =   -1   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {0, 250, 856, true, "C =   +0   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {0, 250, 857, true, "C =   +0   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {0, 250, 1924, true, "C =  +49   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {0, 250, 1926, true, "C =  +50   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {0, 250, 2138, true, "C =  +59   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {0, 250, 2139, true, "C =  +60   colour = 00000010\r\n", {HIGH, LOW, HIGH}},
        {0, 250, 2780, true, "C =  +89   colour = 00000010\r\n", {HIGH, LOW, HIGH}},
        {0, 250, 2781, true, "C =  +90   colour = 00000011\r\n", {LOW, LOW, HIGH}},
        {0, 250, 3208, true, "C = +109   colour = 00000011\r\n", {LOW, LOW, HIGH}},
        {0, 250, 3209, true, "C = +110   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        {0, 250, 3849, true, "C = +139   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        {0, 250, 3851, true, "C = +140   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        // 4294967196 is 100 ms before rollover; these are elapsed 249/250/251.
        {4294967196U, 4294967445ULL, 0, false, ""},
        {4294967196U, 4294967446ULL, 0, true, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {4294967196U, 4294967447ULL, 4095, true, "C = +140   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        // Sampling itself crosses rollover; the saved timestamp is pre-sampling.
        {4294967000U, 4294967295ULL, 1925, true, "C =  +50   colour = 00000110\r\n", {HIGH, LOW, LOW}},
    };
    for (const auto& row : rows) {
        SCOPED_TRACE(::testing::Message() << "last=" << row.last
                     << " now=" << row.now_ms << " counts=" << row.counts);
        SetUp();
        hw::time_us = row.now_ms * 1000;
        g_last_update_ms = row.last;
        hw::analog_inputs.assign(16, row.counts);
        hw::pin_levels[5] = LOW;
        hw::pin_levels[6] = HIGH;
        hw::pin_levels[7] = LOW;
        Serial.output = "preserved\r\n";
        loop();
        EXPECT_EQ(Serial.output, std::string("preserved\r\n") + row.text);
        EXPECT_TRUE(Serial.begin_calls.empty());
        EXPECT_TRUE(hw::pin_modes.empty());
        EXPECT_TRUE(hw::adc_resolutions.empty());
        EXPECT_TRUE(hw::adc_attenuations.empty());
        EXPECT_TRUE(hw::millisecond_delays.empty());
        if (row.updates) {
            EXPECT_EQ(g_last_update_ms, static_cast<uint32_t>(row.now_ms));
            EXPECT_EQ(hw::time_us, row.now_ms * 1000 + 3200);
            EXPECT_EQ(hw::analog_read_pins, std::vector<uint8_t>(16, 4));
            ASSERT_EQ(hw::analog_read_times.size(), 16U);
            for (size_t i = 0; i < 16; ++i) {
                EXPECT_EQ(hw::analog_read_times[i], row.now_ms * 1000 + i * 200);
            }
            EXPECT_EQ(hw::microsecond_delays, std::vector<uint32_t>(16, 200));
            const std::vector<hw::PinWrite> expected_writes = {
                {5, row.levels[0], row.now_ms * 1000 + 3200},
                {6, row.levels[1], row.now_ms * 1000 + 3200},
                {7, row.levels[2], row.now_ms * 1000 + 3200},
            };
            EXPECT_EQ(hw::pin_writes, expected_writes);
            EXPECT_EQ(hw::pin_levels[5], row.levels[0]);
            EXPECT_EQ(hw::pin_levels[6], row.levels[1]);
            EXPECT_EQ(hw::pin_levels[7], row.levels[2]);
        } else {
            EXPECT_EQ(g_last_update_ms, row.last);
            EXPECT_EQ(hw::time_us, row.now_ms * 1000);
            EXPECT_TRUE(hw::analog_read_pins.empty());
            EXPECT_TRUE(hw::analog_read_times.empty());
            EXPECT_TRUE(hw::microsecond_delays.empty());
            EXPECT_TRUE(hw::pin_writes.empty());
            EXPECT_EQ(hw::pin_levels[5], LOW);
            EXPECT_EQ(hw::pin_levels[6], HIGH);
            EXPECT_EQ(hw::pin_levels[7], LOW);
        }
    }

    // Keep real setup and successive loop calls in one deliberate sequence.
    // The middle sample batch averages 1925.5, testing both integer divisions.
    SetUp();
    setup();
    hw::analog_inputs.assign(16, 0);
    for (size_t i = 0; i < 8; ++i) {
        hw::analog_inputs.push_back(0);
        hw::analog_inputs.push_back(3851);
    }
    hw::analog_inputs.insert(hw::analog_inputs.end(), 16, 4095);
    struct Step {
        uint64_t time_us;
        uint32_t last;
        size_t updates;
        bool updates_now;
        const char* cumulative_text;
        std::array<uint8_t, 3> levels;  // Red, green, blue.
    };
    const Step steps[] = {
        {1500000, 1500, 1, true, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {1503200, 1500, 1, false, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {1749999, 1500, 1, false, "C =  -40   colour = 00000100\r\n", {HIGH, HIGH, LOW}},
        {1750000, 1750, 2, true, "C =  -40   colour = 00000100\r\nC =  +50   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {1753200, 1750, 2, false, "C =  -40   colour = 00000100\r\nC =  +50   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {1999999, 1750, 2, false, "C =  -40   colour = 00000100\r\nC =  +50   colour = 00000110\r\n", {HIGH, LOW, LOW}},
        {2000000, 2000, 3, true, "C =  -40   colour = 00000100\r\nC =  +50   colour = 00000110\r\nC = +140   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
        {2003200, 2000, 3, false, "C =  -40   colour = 00000100\r\nC =  +50   colour = 00000110\r\nC = +140   colour = 00000001\r\n", {LOW, HIGH, HIGH}},
    };
    for (const auto& step : steps) {
        SCOPED_TRACE(step.time_us);
        hw::time_us = step.time_us;
        loop();
        EXPECT_EQ(g_last_update_ms, step.last);
        EXPECT_EQ(hw::time_us, step.time_us + (step.updates_now ? 3200 : 0));
        EXPECT_EQ(hw::analog_read_pins, std::vector<uint8_t>(step.updates * 16, 4));
        EXPECT_EQ(hw::microsecond_delays, std::vector<uint32_t>(step.updates * 16, 200));
        EXPECT_EQ(hw::pin_writes.size(), (step.updates + 1) * 3);
        EXPECT_EQ(hw::pin_levels[5], step.levels[0]);
        EXPECT_EQ(hw::pin_levels[6], step.levels[1]);
        EXPECT_EQ(hw::pin_levels[7], step.levels[2]);
        EXPECT_EQ(Serial.output, std::string(
            "\r\nLesson 06 homework - the colour thermometer\r\nTurn the knob.\r\n") +
            step.cumulative_text);
        EXPECT_EQ(Serial.begin_calls.size(), 1U);
        EXPECT_EQ(hw::millisecond_delays, std::vector<uint32_t>{1500});
        EXPECT_EQ(hw::pin_modes.size(), 3U);
        EXPECT_EQ(hw::adc_resolutions, std::vector<uint8_t>{12});
        EXPECT_EQ(hw::adc_attenuations.size(), 1U);
    }
}
}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
#ifdef PIO_UNIT_TESTING
    (void)result;
    return 0;  // PlatformIO determines failures from Google Test output.
#else
    return result;
#endif
}
