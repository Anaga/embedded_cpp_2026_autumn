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
`test/test_firmware/test_theremin.cpp` covers hand mapping, independently held inputs,
pitch/gain ramps, and shared vibrato. Firmware tests drive simulated Echo pulses
through the controller into the real playback/synth classes, including pulse
vibrato, master volume, stop/replacement, and disconnect/suspension behavior.
Boot tests cover automatic both-hand startup without a serial monitor, waiting
for connection and streaming, initialization failures, manual overrides, and
remaining stopped after disconnect/suspension or an explicit stop.
`test/test_firmware/test_synth_button.cpp` checks pull-up initialization,
debounce, bounce during press/release, short selection on release, one-shot
3-second holds, release timing at the threshold, held-at-boot behavior, custom
pins, and clock rollover. Firmware tests verify the full voice cycle,
serial/button selection sharing, offline selection, initialization failures,
and preserving automatic startup and live controls when pressing or holding.
Mute tests check zero stereo samples, output restoration, unchanged master level,
no voice change on long holds, mute before connection, serial commands while
muted, and explicit stop/disconnect/suspension preventing playback resurrection.
Short-click tests cover unmuting without changing voice or volume across every
synth, both connected and offline, then cycling on the following unmuted click.
Playback tests compare oscillator phase and sample time with an unmuted reference
for every voice, and verify finite tones can finish while muted.
`test/test_firmware/test_status_leds.cpp` checks the two GPIO outputs, all five
green blink rates, red Bluetooth indication, independent timing, delayed loops,
clock rollover, cached writes, custom pins, and cleanup/reinitialization.
Firmware tests verify button/serial rate changes, Bluetooth connection independent
of audio state, showcase voice indication, visibility before Bluetooth initialization
without a serial monitor, and unchanged audio/automatic startup. Queue/audio
initialization failures keep the LEDs operational.
`test/test_firmware/test_hand_distance_filter.cpp` checks isolated spikes,
confirmation, gentle motion, output speed limits, freezing on invalid/missing
inputs, return at a different absolute position, cached samples, and clock rollover.
Firmware tests distinguish held hands from a stalled-main-task watchdog and
verify that stop/disconnection still silence held notes. The log-derived distances
use synthetic full-rate timestamps because the attachment contains only
one-second snapshots. Firmware tests also verify raw-vs-filtered readings
reach the actual musical control path.
Add input and expected-value cases to the corresponding function's test, including
meaningful boundary values.

Use `fake_arduino`, `fake_rtos`, and the test-only audio/Bluetooth headers for
simulated inputs and captured output. Reset state between independent cases.

Native mocks do not verify physical hardware.
