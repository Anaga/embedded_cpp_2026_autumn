# PISI - automated microgreens controller (demo)

PISI waters a small growing tray and controls 12 V LED grow lights using an ESP32-C3 Super Mini.

This demonstration version is based on the user's `main(4).cpp`. It keeps pump PWM and the AHT10 temperature safety logic, but changes the timing and RGB status pattern.

## Demo schedule

| Device | Start | On time | Off time | Cycle |
|---|---|---|---|---|
| 12 V pump | immediately after setup | 30 seconds | 90 seconds | 120 seconds |
| 12 V grow LEDs | scheduled ON at startup | 120 seconds | 60 seconds | 180 seconds |

- Pump PWM: 1-second ramp from 65% to 100%, as in the latest source.
- Grow light PWM: 10-second fade in and 10-second fade out *inside* the 120-second ON period.
- The pump is stopped if it runs for 40 seconds without its normal stop. This is only a software limit.
- The program starts without a USB Serial Monitor or laptop.
- AHT10 safety rule remains: grow lights stay OFF if AHT10 is not working, if temperature is too high, or if there is a pump fault. Lights switch off at 28 C; after at least 300 seconds off AND only when temperature is below 24 C, they may resume during the scheduled ON period. AHT10 is sampled about every 2 seconds.
- The pump cycle continues if AHT10 fails. Red error LED shows this condition.
- During the demonstration the potentiometer reading is printed only; it does not change the fixed 30-second pump time.

**Important:** The grow-light schedule starts at power-on, but actual light power can be delayed by AHT10 sensor startup and by the 10-second sunrise fade. With no working AHT10, the grow lights will not turn on.

## RGB status

The RGB LED is common anode: its common pin goes to 3.3 V, and each color pin must have its own resistor. A LOW output turns that color ON.

| Situation | RGB output |
|---|---|
| Fault (pump, AHT10 or thermal pause) | RED 200 ms on, 200 ms off |
| Pump and grow lights ON | BLUE 200 ms on, 200 ms off, GREEN 200 ms on, 200 ms off; repeat (800-ms cycle) |
| Pump ON, grow lights OFF | BLUE steady |
| Pump OFF, grow lights ON | GREEN steady |
| Both OFF, no fault | GREEN 100 ms on every 1000 ms |

Fault has highest priority. If grow-light power is temporarily zero during its fade, the status uses the actual light output, not just the scheduled day phase.

## Hardware and pin budget

This is the pin map in the uploaded `main(4).cpp`, *not* the older homework pin map.

| Part | ESP32-C3 GPIO | Notes |
|---|---|---|
| Pump MOSFET control | GPIO3 | PWM, active-HIGH 3.3 V-compatible module |
| Grow-light MOSFET control | GPIO10 | PWM, active-HIGH 3.3 V-compatible module |
| Potentiometer center | GPIO4 | ADC input, original resistor network |
| RGB red | GPIO5 | Through its own resistor, active LOW |
| RGB green | GPIO6 | Through its own resistor, active LOW |
| RGB blue | GPIO7 | Through its own resistor, active LOW |
| AHT10 SDA | GPIO0 | I2C data |
| AHT10 SCL | GPIO1 | I2C clock |
| Light sensor digital DO (optional) | GPIO20 | Only if `LIGHT_SENSOR_INSTALLED = true` |
| USB | USB-C | 5 V supply and Serial Monitor |

GPIO2, GPIO8 and GPIO9 are boot strapping pins and are not used. GPIO21 is unused, so normal UART0 TX does not conflict with RGB.

Keep all 12 V load current outside the breadboard. Use suitable wire, fuses, splash protection and strain relief. The 12 V power supply must be suitable for the pump and LED load. ESP32 is powered from regulated USB 5 V, never directly from 12 V. Non-isolated drivers need a common low-voltage ground. Do not wire exposed 230 V near water.

## Files and classes

| File | What it does |
|---|---|
| `src/main.cpp` | Connects all parts and reads Serial commands |
| `include/Config.h` | GPIO pins, timing, PWM and safety settings |
| `include/PumpController.h` | Pump PWM ramp, 30/90 schedule and fault limit |
| `include/LightController.h` | Light 120/60 schedule, fade and safety override |
| `include/TemperatureMonitor.h` | AHT10 readings and overheat protection |
| `include/RgbLed.h` | Basic RGB output |
| `include/StatusIndicator.h` | Fault, idle, pump, lights and combined color logic |
| `include/Potentiometer.h` | Knob voltage for monitoring |
| `include/LoopStats.h` | Reports time between passes, including waits and task scheduling |

Classes have `begin()` and `update()` methods as appropriate. There is no network task. The loop normally uses no delays. AHT10 I2C reads can still take a short time.

## How to build and run

1. Open this folder as a PlatformIO project in VS Code.
2. Use the board setting and the `platformio.ini` included here. Install Adafruit AHTX0 and its dependencies via PlatformIO.
3. Connect the hardware only after you verify the pin map and MOSFET control polarity.
4. Keep the pump under water and ensure water can return safely to the tank.
5. Upload the program. Open Serial Monitor at 115200 baud.
6. Read `s` for status, `h` for help, `w` for an extra watering start, or `p` to stop the pump. A manual `w` starts a new 120-second pump cycle.
7. Test cold start with no Serial Monitor attached.

Example PlatformIO commands: `pio run` (build); `pio run -t upload` (`-t` selects the upload task); `pio device monitor -b 115200` (`-b` sets baud rate).

## How it works / rhythm table

| Part | Period | Owner |
|---|---|---|
| Pump schedule and cutoff | Every loop | Main loop |
| RGB pattern | Every loop | Main loop |
| Light schedule | Every loop | Main loop |
| AHT10 temperature and humidity | Every 2 s, retry 5 s | Main loop |
| Potentiometer voltage | Every 100 ms | Main loop |
| Serial commands | Every loop; max 12 chars/pass | Main loop |
| Loop statistics | Every loop, report every 5 s | Main loop |

## Safety and known problems

- This code has not been tested on your physical board by the assistant. Compile and test it with water before connecting the pump to the plant tray.
- The JT-160 is brushless. You say pump PWM works. Stop the test if the pump stalls, clicks, gets hot or restarts. The 65% to 100% ramp remains enabled.
- If AHT10 is unplugged, lights are OFF and red flashes. This is expected. Temperature protection is more important than forcing lights ON.
- The 40-second hard cutoff cannot protect against frozen firmware, MOSFET failure, dry running or a short circuit. Use an independent fuse and water-level protection in a later version.
- The light sensor on GPIO20 is optional, and uses the module's DO output, not AO. It is only reported in Serial and does not control the lights.
- Pump and grow-light times are fixed for the demo. The knob is read but does not control the 30-second pump ON period.
- The LED strips are not verified plant grow lamps; evaluate plant light later.
- If the AHT10 sensor reports 28 C or more, the grow lights turn OFF and the RGB LED flashes red. They can only resume after at least 5 minutes of thermal pause and a measured temperature strictly below 24 C, provided it is a scheduled light-ON period and no other fault is active. This may interrupt the intended 120-second light demonstration.

## Before the live demonstration

- Check water flow and return; keep the pump submerged.
- Verify the pump and grow-light MOSFETs really switch with 3.3 V active-HIGH signals.
- Verify RGB colors on GPIO5, GPIO6 and GPIO7.
- Confirm AHT10 is connected to GPIO0/1 and reports reasonable temperature.
- Watch at least two full pump cycles and two full light cycles.
- Check both-running alternating pattern, blue-only, green-only and idle heartbeat.
- Test the red fault pattern by safely disconnecting AHT10 while power is OFF, then restarting with the sensor missing. Restore after the test.
- Capture Serial output and a short backup video.
- Prepare a photo, pin table, known problems and the measured longest loop interval for the class demo.

## Next steps

Add a water-level sensor, a calibrated grow-light sensor, an aquarium filter plan and a normal growing mode with longer timings. Keep the demonstrated version stable for Friday.

## Final presentation documents

- [Wiring and safety](docs/WIRING_AND_SAFETY.md) and [functional wiring diagram](docs/WIRING_DIAGRAM.svg)
- [Software classes and timing](docs/PROJECT_ARCHITECTURE.md)
- [10-minute presentation notes](docs/DEMO_SCRIPT_10_MIN.md)
- [Test checklist](docs/TEST_PLAN.md) and [test results to fill](docs/TEST_RESULTS_TO_FILL.md)
- [Debugging history](docs/DEBUG_LOG.md)
- [Photos and evidence](docs/PHOTOS_AND_EVIDENCE.md)
- [Final submission checklist](docs/SUBMISSION_CHECKLIST.md)

**Submission note:** The documents and code are included, but measured test results, a final post-repair hardware photo, a backup video and proof of a GitHub push are **not** provided. Add your own real evidence before submitting.
