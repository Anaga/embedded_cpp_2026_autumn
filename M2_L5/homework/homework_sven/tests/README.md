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
~/.platformio/penv/bin/pio test -e native -a '--gtest_filter=Temperatures/CelsiusConversion.*'
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

- Conversion cases: add `{fahrenheit, expected_celsius}` rows to `CELSIUS_CASES`.
  Expected whole-degree results truncate toward zero.
- Other checks: add a `TEST_F(HomeworkTest, DescriptiveName)`. Use `fake_arduino`
  for ADC readings, simulated time and captured serial output.

Each test starts with fresh simulated hardware and firmware state. Actual ADC,
USB and hardware timing still need a board check.
