# Local tests

Google Test checks for `../src/main.cpp` using simulated Arduino hardware.

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
