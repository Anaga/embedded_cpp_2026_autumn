# Local tests

Google Test exercises the demo engine, playback controller, and Bluetooth
firmware in `../src/main.cpp` using simulated hardware.

## Run with PlatformIO

From the repository root:

```sh
~/.platformio/penv/bin/pio test -d tests -e native
```

Or from this `tests` directory:

```sh
~/.platformio/penv/bin/pio test -e native
```

Use `pio` instead of the full path if it is on your PATH. The first run
downloads the dependencies.

## CLion

1. Open this directory's `CMakeLists.txt` as a project.
2. Use a **Debug** profile with build directory `cmake-build-clion-debug`.
3. Run or debug a **Google Test** configuration with both **Target** and
   **Executable** set to `bluetooth_audio_tests`.

## Add tests

The Bluetooth and playback tests are in `test/test_firmware/test_main.cpp`.
The demo engine tests are in `test/test_firmware/test_demo_engine.cpp`.
Ultrasonic driver and scheduling tests are in `test/test_firmware/test_ultrasonic.cpp`;
the firmware tests also cover sensor commands without Bluetooth or audio initialization.
`test/test_firmware/test_theremin.cpp` covers hand mapping, missing/stale inputs,
pitch/gain ramps, and shared vibrato. Firmware tests drive simulated Echo pulses
through the controller into the real playback/synth classes, including pulse
vibrato, master volume, stop/replacement, and disconnect/suspension behavior.
`test/test_firmware/test_hand_distance_filter.cpp` checks isolated spikes,
confirmation, gentle motion, output speed limits, invalid/stale inputs,
reacquisition, cached samples, and clock rollover. The log-derived distances
use synthetic full-rate timestamps because the attachment contains only
one-second snapshots. Firmware tests also verify raw-vs-filtered readings
reach the actual musical control path.
Add input and expected-value cases to the corresponding function's test, including
meaningful boundary values.

Use `fake_arduino`, `fake_rtos`, and the test-only audio/Bluetooth headers for
simulated inputs and captured output. Reset state between independent cases.

Native mocks do not verify physical hardware.
