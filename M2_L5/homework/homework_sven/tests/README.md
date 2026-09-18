# Local tests

Google Test checks for `../src/main.cpp` using simulated Arduino hardware.

## Run with PlatformIO

From the parent `homework` directory:

```sh
~/.platformio/penv/bin/pio test -d homework_sven/tests -e native
```

Or from this `tests` directory:

```sh
~/.platformio/penv/bin/pio test -e native
```

Use `pio` instead of the full path if it is on your PATH. PlatformIO installs
Google Test 1.17.0 in this project's `.pio` directory on the first run.

To run only conversion cases from `tests`:

```sh
~/.platformio/penv/bin/pio test -e native -a '--gtest_filter=Temperatures/Celsius*.*'
```

## CLion

1. Open this directory's `CMakeLists.txt` as a project.
2. Use the **Debug** profile with build directory `cmake-build-clion-debug`.
3. Run or debug the **Google Test** configuration with both **Target** and
   **Executable** set to `temperature_tests`.

If missing, create it under **Run > Edit Configurations > + > Google Test** and
select all tests. For an incompatible generator error, use a new, unused
`cmake-build-*` directory.

The first build downloads Google Test.

## Add tests

Edit `test/test_temperature/test_main.cpp`:

- Whole degrees: add `{fahrenheit, expected_celsius}` rows to `CELSIUS_CASES`.
- Tenths: add `{fahrenheit, expected_tenths}` rows to `CELSIUS_TENTHS_CASES`.
  Both conversions truncate toward zero; 39 F gives 3 C or 38 tenths C.
- Serial output: add `{adc_counts, expected_output}` rows to `LOOP_OUTPUT_CASES`.
  Checks require signed F/C/Ctenths values and one decimal digit for Ctenths;
  padding and line breaks are flexible, including a second Ctenths line.
- Other checks: add a `TEST_F(HomeworkTest, DescriptiveName)`. Use `fake_arduino`
  for ADC readings, simulated time and captured serial output.

Each test starts with fresh simulated hardware and firmware state. Actual ADC,
USB and hardware timing still need a board check.
