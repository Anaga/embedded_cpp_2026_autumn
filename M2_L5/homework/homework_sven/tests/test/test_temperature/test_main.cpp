#include <gtest/gtest.h>
#include <ostream>
#include <sstream>
#include <string>

// Include once so the assignment's static functions can be tested unchanged.
// Arduino.h is supplied by test/support in the native environment only.
#include "../../../src/main.cpp"

// A fresh firmware and simulated hardware state for every test and input row.
class HomeworkTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake_arduino::reset();
        g_last_print_ms = 0U;
        g_last_counts = 0U;
        g_last_fahrenheit = 0;
        g_printed_once = false;
    }
};

TEST_F(HomeworkTest, AdcEndpoints) {
    EXPECT_EQ(-40, mapToFahrenheit(0U));
    EXPECT_EQ(140, mapToFahrenheit(3850U));
}

TEST_F(HomeworkTest, AdcFreezingPoint) {
    EXPECT_EQ(32, mapToFahrenheit(1540U));
}

TEST_F(HomeworkTest, AdcClampsAboveCalibratedMaximum) {
    EXPECT_EQ(140, mapToFahrenheit(4095U));
    EXPECT_EQ(140, mapToFahrenheit(UINT16_MAX));
}

TEST_F(HomeworkTest, AdcMappingAtDegreeBoundaries) {
    EXPECT_EQ(-40, mapToFahrenheit(21U));
    EXPECT_EQ(-39, mapToFahrenheit(22U));
    EXPECT_EQ(31, mapToFahrenheit(1539U));
    EXPECT_EQ(32, mapToFahrenheit(1540U));
    EXPECT_EQ(50, mapToFahrenheit(1925U));
    EXPECT_EQ(139, mapToFahrenheit(3849U));
}

TEST_F(HomeworkTest, PotentiometerAveragesNoisySamplesAndTruncates) {
    // The mean is 1001.5 counts; the integer result must be 1001.
    fake_arduino::analog_samples = {
        1000U, 1003U, 1000U, 1003U, 1000U, 1003U, 1000U, 1003U,
        1000U, 1003U, 1000U, 1003U, 1000U, 1003U, 1000U, 1003U,
    };
    EXPECT_EQ(1001U, readPotentiometer());
    EXPECT_EQ(16U, fake_arduino::analog_read_count);
}

TEST_F(HomeworkTest, PotentiometerAveragesMaximum12BitReadings) {
    fake_arduino::analog_counts = 4095U;
    EXPECT_EQ(4095U, readPotentiometer());
    EXPECT_EQ(16U, fake_arduino::analog_read_count);
}

struct CelsiusCase {
    int16_t fahrenheit;
    int16_t expected_celsius;
};

// Hardcoded expected answers; fractional results truncate toward zero.
// Add another row to exercise another input without changing the runner.
static constexpr CelsiusCase CELSIUS_CASES[] = {
    // Arithmetic limits of the input type (not physical temperature limits).
    {INT16_MIN, -18222},
    {-32767, -18221},

    // Assignment's lower endpoint and intermediate cold temperatures.
    {-40, -40},
    {-39, -39},
    {-31, -35},
    {-22, -30},
    {-4, -20},
    {0, -17},
    {14, -10},
    {23, -5},

    // Around freezing: check truncation on both sides of zero Celsius.
    {29, -1},
    {30, -1},
    {31, 0},
    {32, 0},
    {33, 0},
    {34, 1},
    {35, 1},
    {36, 2},

    // Intermediate warm temperatures, including fractional Celsius results.
    {50, 10},
    {59, 15},
    {68, 20},
    {72, 22},
    {77, 25},
    {86, 30},
    {95, 35},
    {104, 40},
    {122, 50},
    {131, 55},

    // Assignment's upper endpoint and nearby/out-of-range inputs.
    {139, 59},
    {140, 60},
    {141, 60},
    {212, 100}, // Water's boiling-point reference at standard pressure.

    // Arithmetic limits at the positive end of int16_t.
    {32766, 18185},
    {INT16_MAX, 18186},
};

// Google Test prints these fields when a parameterized case fails.
void PrintTo(const CelsiusCase& value, std::ostream* output) {
    *output << value.fahrenheit << " F -> " << value.expected_celsius << " C";
}

class CelsiusConversion : public HomeworkTest,
                          public ::testing::WithParamInterface<CelsiusCase> {};

TEST_P(CelsiusConversion, ConvertsToWholeDegrees) {
    const CelsiusCase& value = GetParam();
    EXPECT_EQ(value.expected_celsius, fahrenheitToCelsius(value.fahrenheit));
}

// Parameter names must be alphanumeric. Keep negative inputs readable in CLion.
static std::string temperatureName(int16_t value) {
    const int32_t wide_value = value;
    return wide_value < 0 ? "Minus" + std::to_string(-wide_value)
                          : std::to_string(wide_value);
}

INSTANTIATE_TEST_SUITE_P(
    Temperatures,
    CelsiusConversion,
    ::testing::ValuesIn(CELSIUS_CASES),
    [](const ::testing::TestParamInfo<CelsiusCase>& info) {
        return "F" + temperatureName(info.param.fahrenheit) + "ToC" +
               temperatureName(info.param.expected_celsius);
    });

struct CelsiusTenthsCase {
    int16_t fahrenheit;
    int16_t expected_tenths;
};

// Expected results truncate toward zero at one decimal place. The full
// int16_t Fahrenheit extremes would need a wider return type for tenths.
static constexpr CelsiusTenthsCase CELSIUS_TENTHS_CASES[] = {
    {-40, -400},
    {-39, -394},
    {-31, -350},
    {-22, -300},
    {-4, -200},
    {0, -177},
    {14, -100},
    {23, -50},
    {29, -16},
    {30, -11},
    {31, -5},
    {32, 0},
    {33, 5},
    {34, 11},
    {35, 16},
    {36, 22},
    {39, 38},
    {50, 100},
    {59, 150},
    {68, 200},
    {72, 222},
    {77, 250},
    {86, 300},
    {95, 350},
    {104, 400},
    {122, 500},
    {131, 550},
    {139, 594},
    {140, 600},
    {141, 605},
    {212, 1000},
};

void PrintTo(const CelsiusTenthsCase& value, std::ostream* output) {
    *output << value.fahrenheit << " F -> " << value.expected_tenths << " tenths C";
}

class CelsiusTenthsConversion : public HomeworkTest,
                                public ::testing::WithParamInterface<CelsiusTenthsCase> {};

TEST_P(CelsiusTenthsConversion, ConvertsToTenthsOfADegree) {
    const CelsiusTenthsCase& value = GetParam();
    EXPECT_EQ(value.expected_tenths, fahrenheitToCelsiusInTenths(value.fahrenheit));
}

INSTANTIATE_TEST_SUITE_P(
    Temperatures,
    CelsiusTenthsConversion,
    ::testing::ValuesIn(CELSIUS_TENTHS_CASES),
    [](const ::testing::TestParamInfo<CelsiusTenthsCase>& info) {
        return "F" + temperatureName(info.param.fahrenheit) + "ToTenths" +
               temperatureName(info.param.expected_tenths);
    });

static void pollAtNextPeriod(uint16_t counts) {
    fake_arduino::serial_output.clear();
    fake_arduino::analog_counts = counts;
    fake_arduino::advanceMilliseconds(PRINT_PERIOD_MS);
    loop();
}

struct LoopOutputCase {
    uint16_t adc_counts;
    const char* expected_output;
};

// Padding and line breaks are flexible; values, signs, labels and the single
// decimal digit must match. Each row starts with freshly reset firmware state.
static const LoopOutputCase LOOP_OUTPUT_CASES[] = {
    {0U,    "F = -40 C = -40 Ctenths = -40.0"},
    {856U,  "F = +0 C = -17 Ctenths = -17.7"},
    {1348U, "F = +23 C = -5 Ctenths = -5.0"},
    {1498U, "F = +30 C = -1 Ctenths = -1.1"},
    {1519U, "F = +31 C = +0 Ctenths = -0.5"},
    {1540U, "F = +32 C = +0 Ctenths = +0.0"},
    {1562U, "F = +33 C = +0 Ctenths = +0.5"},
    {1583U, "F = +34 C = +1 Ctenths = +1.1"},
    {1690U, "F = +39 C = +3 Ctenths = +3.8"},
    {1925U, "F = +50 C = +10 Ctenths = +10.0"},
    {2400U, "F = +72 C = +22 Ctenths = +22.2"},
    {3829U, "F = +139 C = +59 Ctenths = +59.4"},
    {3850U, "F = +140 C = +60 Ctenths = +60.0"},
    {4095U, "F = +140 C = +60 Ctenths = +60.0"},
};

void PrintTo(const LoopOutputCase& value, std::ostream* output) {
    *output << value.adc_counts << " ADC counts -> " << value.expected_output;
}

class LoopOutput : public HomeworkTest,
                   public ::testing::WithParamInterface<LoopOutputCase> {};

TEST_P(LoopOutput, PrintsSignedTemperaturesWithOneDecimalDigit) {
    const LoopOutputCase& value = GetParam();
    pollAtNextPeriod(value.adc_counts);

    // Treat spaces, tabs and newlines as separators so either a single line
    // or a second Ctenths line is accepted, without fixing column widths.
    std::istringstream output(fake_arduino::serial_output);
    std::string actual;
    std::string token;
    while (output >> token) {
        if (!actual.empty()) {
            actual += ' ';
        }
        actual += token;
    }
    EXPECT_EQ(value.expected_output, actual);
}

INSTANTIATE_TEST_SUITE_P(
    Readings,
    LoopOutput,
    ::testing::ValuesIn(LOOP_OUTPUT_CASES),
    [](const ::testing::TestParamInfo<LoopOutputCase>& info) {
        return "Adc" + std::to_string(info.param.adc_counts);
    });

TEST_F(HomeworkTest, LoopFirstZeroReadingPrintsAtExactPeriod) {
    // Test boot-relative times without setup()'s startup delay.
    fake_arduino::advanceMilliseconds(249U);
    loop();
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    EXPECT_EQ(0U, fake_arduino::analog_read_count);

    fake_arduino::advanceMilliseconds(1U);
    loop();
    EXPECT_TRUE(fake_arduino::serial_output.find("F =  -40") == 0U);
    EXPECT_EQ(16U, fake_arduino::analog_read_count);
}

TEST_F(HomeworkTest, LoopDeadbandAccumulatesFromLastPrintWhenIncreasing) {
    pollAtNextPeriod(1539U); // 31 F; establish the last printed position.
    EXPECT_FALSE(fake_arduino::serial_output.empty());

    pollAtNextPeriod(1540U); // 32 F, but only one count of movement.
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    pollAtNextPeriod(1546U); // Seven counts from the last printed position.
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    pollAtNextPeriod(1547U); // Eight counts: print even after smaller steps.
    EXPECT_TRUE(fake_arduino::serial_output.find("F =  +32") == 0U);
}

TEST_F(HomeworkTest, LoopDeadbandAccumulatesFromLastPrintWhenDecreasing) {
    pollAtNextPeriod(1540U); // 32 F.
    EXPECT_FALSE(fake_arduino::serial_output.empty());

    pollAtNextPeriod(1539U); // 31 F, but only one count of movement.
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    pollAtNextPeriod(1533U); // Seven counts of movement.
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    pollAtNextPeriod(1532U); // Eight counts of movement in the other direction.
    EXPECT_TRUE(fake_arduino::serial_output.find("F =  +31") == 0U);
}

TEST_F(HomeworkTest, LoopSuppressesMovementWithoutAFahrenheitChange) {
    pollAtNextPeriod(1540U); // 32 F.
    pollAtNextPeriod(1548U); // Eight counts, still 32 F.
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    pollAtNextPeriod(1556U); // Sixteen counts, still 32 F.
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    pollAtNextPeriod(1562U); // Now 33 F.
    EXPECT_TRUE(fake_arduino::serial_output.find("F =  +33") == 0U);
}

TEST_F(HomeworkTest, LoopPrintPeriodSurvivesMillisRollover) {
    const uint64_t start_ms = static_cast<uint64_t>(UINT32_MAX) - 100U;
    fake_arduino::elapsed_us = start_ms * 1000U;
    fake_arduino::analog_counts = 1540U;
    loop();
    EXPECT_FALSE(fake_arduino::serial_output.empty());

    fake_arduino::serial_output.clear();
    fake_arduino::analog_counts = 3850U;
    // The clock is now past UINT32_MAX and millis() has wrapped to 148.
    fake_arduino::elapsed_us = (start_ms + 249U) * 1000U;
    loop();
    EXPECT_TRUE(fake_arduino::serial_output.empty());
    EXPECT_EQ(16U, fake_arduino::analog_read_count);

    fake_arduino::advanceMilliseconds(1U);
    loop();
    EXPECT_TRUE(fake_arduino::serial_output.find("F = +140") == 0U);
    EXPECT_EQ(32U, fake_arduino::analog_read_count);
}

TEST_F(HomeworkTest, LoopPrintsTheConvertedCelsiusValue) {
    pollAtNextPeriod(1925U);
    // Allow the additional Ctenths field; LoopOutput checks the full reading.
    EXPECT_EQ(0U, fake_arduino::serial_output.find("F =  +50   C =  +10"));
}

TEST_F(HomeworkTest, LoopWaitsBetweenPrintsAndSuppressesUnchangedReadings) {
    setup();
    fake_arduino::serial_output.clear();
    fake_arduino::analog_counts = 1540U;
    loop();
    EXPECT_EQ(0U, fake_arduino::serial_output.find("F =  +32   C =   +0"));

    fake_arduino::serial_output.clear();
    fake_arduino::analog_counts = 3850U;
    loop();
    EXPECT_TRUE(fake_arduino::serial_output.empty());

    fake_arduino::advanceMilliseconds(250U);
    loop();
    // Check Fahrenheit only here so the timing test is independent of TODO 1.
    EXPECT_TRUE(fake_arduino::serial_output.find("F = +140") == 0U);

    fake_arduino::serial_output.clear();
    fake_arduino::advanceMilliseconds(250U);
    loop();
    EXPECT_TRUE(fake_arduino::serial_output.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
#ifdef PIO_UNIT_TESTING
    // PlatformIO parses Google Test's results and sets its own failure status.
    // Its native runner can misinterpret a nonzero program exit as a signal.
    (void)result;
    return 0;
#else
    // CLion/CTest and direct runs need the normal process exit status.
    return result;
#endif
}
