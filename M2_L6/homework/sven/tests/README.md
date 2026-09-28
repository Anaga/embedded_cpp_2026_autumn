# Local tests

Google Test checks for `../src/main.cpp` using simulated Arduino hardware.

`MapToCelsius` and `Loop` expect 0–3850 ADC counts to map to −40–+140 °C,
with integer truncation and saturation above 3850 counts.

`ColourForTemperature` and `Loop` check TODO 1's five colour bands from the
[homework README](../../README.md#the-colour-code), including their boundaries.
The loop checks both Serial colour bits and the common-anode LED pin levels.
These two tests reject the original `COLOUR_OFF` stub. TODO 2's hysteresis is
not part of these expectations.

## Run with PlatformIO

From the `homework` directory:

```sh
~/.platformio/penv/bin/pio test -d sven/tests -e native
```

Or from this `tests` directory:

```sh
~/.platformio/penv/bin/pio test -e native
```

Use `pio` instead of the full path if it is on your PATH. The first run
downloads the dependencies.

## CLion

1. Open this directory's `CMakeLists.txt` as a project.
2. Use the **Debug** profile with build directory `cmake-build-clion-debug`.
3. Run or debug a **Google Test** configuration with both **Target** and
   **Executable** set to `firmware_tests`.

## Add tests

Edit `test/test_firmware/test_main.cpp`. Each function has one test containing
its input and expected-value table. Add rows there, including meaningful
extremes; add one `TEST_F` for each new function.

Use `fake_arduino` for inputs, simulated time and captured Serial output.
Reset hardware and firmware state between independent rows.

Native mocks do not verify physical hardware.
