# Two ultrasonic sensors

## Identification and capabilities

The photos show an **HC-SR04-compatible ultrasonic ranging module**, with
`Vcc`, `Trig`, `Echo`, and `Gnd` pins. Its back also labels `Trig/Rx/SCL`,
`Echo/Tx/SDA`, `R4/IIC`, `R5/UART`, and `R4+R5/1-WIRE`. These identify a
multi-interface variant, but do not establish its manufacturer or exact
specification. The two metal cans are the transmitter (`T`) and receiver (`R`)
of **one** sensor. This setup uses two complete modules.

| Capability | What to expect |
| --- | --- |
| Measurement | Distance to a reflecting object in front of the module. |
| Trig/Echo output | A digital high pulse representing the sound's round-trip time. The demo records its width in microseconds. |
| Conversion | `distance_cm = echo_us * (331.3 + 0.606 * temperature_C) / 20000`. At 20 °C, 1000 µs is about 17.17 cm. |
| Nominal range | Comparable multi-interface modules advertise 2–450 cm. Test the useful range of your particular boards. |
| Precision | A related module specifies 1 mm resolution and ±2% accuracy. Resolution and displayed decimal places do not establish actual accuracy. |
| Ultrasound | Comparable modules use a 40 kHz carrier. The exposed interfaces provide ranging information, without raw microphone audio, echo amplitude, or a confidence score. |
| Other interfaces | The PCB advertises UART, I²C, and single-wire modes. This implementation uses Trig/Echo only. |
| Diagnostic rate | `sensors` commands use five attempts/second/sensor, alternating modules 100 ms apart. |
| Live rate | `theremin` commands use about 14 attempts/second/sensor: one ping every 35 ms, 70 ms per module. This is a firmware setting, rather than a measured maximum. |
| Logging rate | One combined pitch/volume line at most once/second, starting after each selected sensor has a result. Statistics still include every attempt. |

The [Joy-IT SEN-US01 specifications](https://joy-it.net/en/products/SEN-US01)
and [related module manual](https://www.lextronic.fr/lextronic_doc/joy-it/SEN-US01.pdf)
provide reference figures for similar hardware, rather than certification of
these boards. The manual also describes mode-selection resistors. Leave both
R4 and R5 **unpopulated** for Trig/Echo; they appear unpopulated in the photo.
Do not change them for this demo: revisions differ, and a printed label does
not establish the required resistor value or protocol.

UART/I²C would still return ranging data. Some related I²C variants share a
fixed address, so do not assume two can coexist on one bus. The single-wire
marking does not establish Dallas/Maxim 1-Wire compatibility.

Soft, small, or tilted targets and reflections off furniture can produce missing
or misleading ranges. Start with a flat book perpendicular to one module, then
try hands. A timeout cannot distinguish an absent target from poor reflections,
an excessive distance, or incorrect wiring.

## Wiring to the Freenove ESP32-WROVER

The current setup powers **both modules from the board's 3V3 (3.3 V) pin**,
with Trig and Echo connected directly. No Echo voltage dividers are used.
The GPIO assignments follow [the proposal](../final-project-proposal.md),
which originally planned a 5 V supply and Echo dividers. Numbers below are
**GPIO labels**, rather than physical header positions. Disconnect USB while
wiring, then power the Freenove through USB.

| Module connection | Pitch module | Volume module |
| --- | --- | --- |
| Vcc | Board 3V3 (3.3 V) | Board 3V3 (3.3 V) |
| Gnd | Board GND | Board GND |
| Trig / Rx / SCL | GPIO32 directly | GPIO13 directly |
| Echo / Tx / SDA | GPIO33 directly | GPIO14 directly |

In the **front photo**, with cans facing you and pins pointing down, the order
is **Vcc, Trig, Echo, Gnd** from left to right. It reverses when viewed from the
back. Follow the actual printed labels on each module.

Connect both modules as follows:

```text
Freenove ESP32-WROVER         Pitch HC-SR04
3V3 (3.3 V) ──────────────── Vcc
GND ─────────────────────── Gnd
GPIO32 ──────────────────── Trig
GPIO33 ◀─────────────────── Echo

                             Volume HC-SR04
3V3 (3.3 V) ──────────────── Vcc
GND ─────────────────────── Gnd
GPIO13 ──────────────────── Trig
GPIO14 ◀─────────────────── Echo
```

Both modules share the board's 3.3 V supply and common ground. Keep their Trig
and Echo lines separate, using the four GPIOs shown above. The firmware uses
the same pins and timing with this supply; no voltage-specific code change is
required. This diagram documents the modules used in this project; 3.3 V supply
support should not be assumed for every HC-SR04 revision.

GPIO32/33 and GPIO13/14 avoid WROVER flash/PSRAM and boot-strapping pins.
GPIO13/14 also serve JTAG, so reserve them for the sensors during this demo.
The [official board pinout](https://github.com/Freenove/Freenove_ESP32_WROVER_Board/blob/main/ESP32_Pinout_1.6.png)
shows their header locations.

Point the modules at different hand areas and avoid facing them at each other.
Alternating pings reduces crosstalk, but reflections can still cause it. Compare
individual and combined operation.

## Run the demo

Build/upload and open the 115200 baud monitor from the repository root:

```sh
~/.platformio/penv/bin/pio run
~/.platformio/penv/bin/pio run --target upload
~/.platformio/penv/bin/pio device monitor
```

Sensors are **off at boot**. Enter these commands in the serial monitor:

```text
sensors pitch
sensors volume
sensors on
sensors status
sensors reset
sensors off
```

- `sensors pitch` / `sensors volume` test only the named module.
- `sensors on` (alias `sensors demo`) alternates both modules.
- Starting a mode resets statistics and waits 200 ms before the first ping.
- `sensors` / `sensors status` report the mode, measurement/logging intervals, and range.
- `sensors reset` clears counts and extrema without stopping measurements.
- `sensors off` cancels pending attempts and detaches Echo interrupts.

No speaker connection is required. Existing `stop` / `!` commands control audio;
use `sensors off` to stop measurements. The sensor demo reports readings.
The separate [live pulse vibrato test](../README.md#live-pulse-vibrato-with-your-hands)
uses `synth pulse` followed by `theremin pitch`, `theremin volume`, or
`theremin on` to turn these readings into sound. It uses the same GPIO wiring.
Its 5–50 cm hand range is narrower than the distance diagnostics' 2–450 cm filter.
Live control uses a faster schedule: 70 ms between triggers of the same module,
with the two modules staggered 35 ms apart. The
[HC-SR04 datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
recommends measurement cycles longer than 60 ms. The driver retains a 30 ms
echo deadline, and the scheduler waits for completion/timeout before another
trigger. The faster schedule still needs checking for crosstalk on your modules;
compare each hand individually with both hands together. Diagnostic commands
restore the conservative 200 ms schedule. Logging stays at most once per second
in either mode, with both sensors on the same line when both are selected.

Illustrative output, rather than a recording from hardware:

```text
[330 ms] pitch ok | echo 1000 us | raw 17.17 cm | valid 1/1 | min 17.17 max 17.17 cm || volume timeout | echo 0 us | distance n/a | valid 0/1 | min/max n/a
```

Each line is a snapshot of the latest completed result for each selected sensor.
The timestamp is when the line was reported; the alternating measurements finish
at different times. The first line waits until every selected sensor has a
result, including failures. Logging is limited to one combined line per second;
attempts between lines still update the counts and min/max. Single-sensor modes
show only that sensor. Status meanings:

| Status | Meaning |
| --- | --- |
| `ok` | Complete pulse converts to 2–450 cm. |
| `out-of-range` | Complete pulse converts outside 2–450 cm; width and calculated distance are still shown. |
| `timeout` | No complete pulse within 30 ms of starting the attempt. Check power, target, wiring, and mode selection. |
| `echo-high` | Echo was high before triggering. No ping was emitted. Check wiring or a stuck module. |

Only valid readings contribute to min/max. Invalid attempts count toward the
denominator and do not reuse an earlier distance as a current reading. The demo
assumes 20 °C; the driver exposes `setTemperatureC()` for later compensation.

## Hand filtering and misleading echoes

`ok` confirms a complete pulse corresponding to 2–450 cm; it does not establish
that the echo came from a hand. The live instrument only accepts hand distances
from 2–50 cm, with 2–5 cm clamped to the near endpoint.

Each sensor has an independent [`HandDistanceFilter`](../include/HandDistanceFilter.h).
It holds changes larger than 4 cm until two consecutive usable readings agree
within 2 cm (at most 200 ms apart). A return to the old position discards an
isolated candidate. A 0.15 cm deadband suppresses small variations; confirmed
movement is smoothed with a 50 ms time constant and limited to 30 cm/s. These
named constants live in the filter class for later tuning. The first usable
reading establishes the initial position; after the 250 ms timeout, two
consistent readings are needed to reacquire a hand. Cached readings, invalid
pulses, out-of-range distances, and unconfirmed jumps never refresh freshness.

In live-mode log lines, each sensor includes both `raw ... cm` and `hand ... cm`,
plus `held N`, the number of large changes held for confirmation. `hand n/a`
means the tracker has no fresh accepted position. Diagnostics remain raw and
omit the hand fields. All attempts still update the raw counts/min/max, including
spikes rejected by the hand tracker. `sensors reset` clears these counters while
preserving the current hand position and smoothing state.

The supplied 54-second log excerpt illustrates two different problems:

- Volume briefly reads 39.24 cm between 17.77 and 18.46 cm snapshots. An isolated
  excursion at the sampling level should be held rather than sent to amplitude.
- Pitch alternates between plateaus around 57 and 113 cm; 34 of its 55 snapshots
  exceed the 50 cm control limit. Those distances are already excluded from the
  instrument even though the raw line says `ok`. Volume later stays near 94 cm,
  which is also outside the control range.

Both channels' valid/attempt counters increase equally during this excerpt;
the low cumulative valid percentages also include earlier history. The logs
show one snapshot/second, rather than every approximately 14 Hz reading, so
they cannot prove whether an excursion was one isolated sample or persisted.
The regression tests use the observed distances with explicitly synthetic
70 ms sample timing, rather than claiming to replay unseen measurements.

Persistent alternate plateaus can be consistent with reflections or interference,
but the log alone cannot identify the physical cause. Ultrasonic manufacturers
describe both [sensor interference](https://maxbotix.com/pages/faq) and
[target/noise effects](https://maxbotix.com/pages/sensor-selection-set-up-and-troubleshooting-overview).
A filter cannot identify the true hand when a wrong echo is consistently plausible.
Compare `theremin pitch` and `theremin volume` separately with `theremin on`,
holding a flat target 15–20 cm in front of each sensor. If jumps mainly appear
with both running, investigate crosstalk and mounting. If they persist with one
sensor, check the target angle, surrounding reflectors, connections, and the
supply requirements of the particular module revision. Keep the configured
3.3 V wiring unless that revision's electrical requirements justify changing it.

To characterize the modules, test each alone at measured 5, 10, 20, 50, 100,
and 200 cm with a flat target. Hold it still to observe min/max spread, resetting
statistics at each position. Compare both sensors together, then repeat with
hands and an audio tone. Record useful hand range, jitter, failed-attempt
fraction, and crosstalk before choosing instrument controls.

## Reusable classes and verification

- [`UltrasonicSensor`](../include/UltrasonicSensor.h) owns one Trig/Echo pair,
  interrupt capture, a 30 ms deadline, temperature conversion, and typed results.
  Call its public API from one task: `begin()`, schedule `trigger()` with the
  module's required recovery time (70 ms for this project's live mode, 200 ms
  for diagnostics), and call `poll()` regularly. `poll()` returns true once per result; `reading()`
  includes the completion timestamp. `end()` cancels capture. Instances cannot
  be copied or moved because the interrupt stores their address.
- [`UltrasonicDemo`](../include/UltrasonicDemo.h) owns two drivers, configurable
  pins, scheduling, statistics, hand filters, and serial reporting. `pitchReading()` /
  `volumeReading()` expose raw results. `pitchHandReading()` / `volumeHandReading()`
  expose filtered positions with accepted-source timestamps; the instrument uses
  these rather than raw distances.
- [`ThereminController`](../include/ThereminController.h) maps accepted hand
  positions to center pitch and amplitude, preserving the
  250 ms accepted-source timeout before muting stale inputs.
  [`LiveTone`](../include/LiveTone.h) ramps pitch over 20 ms and hand amplitude
  over 15 ms in the audio task;
  [`Vibrato`](../include/Vibrato.h) modulates pitch in both the original demo
  pattern and live audio. The selected existing synth voice renders the result.
- Firmware calls `sensors.update()` from `loop()`. Interrupts capture timestamps;
  conversion and reporting run in the main task. A ping uses a 12 µs trigger
  sequence without a blocking wait for Echo. Sensor work stays outside the audio
  callback. IRQ scheduling can still affect timing under Bluetooth load.

Run `~/.platformio/penv/bin/pio test -d tests -e native` for conversion, missing
and incomplete echoes, stuck-high handling, range limits, temperature, clock
rollover, scheduling, statistics, and firmware command integration. Run
`~/.platformio/penv/bin/pio run` for the ESP32 build. Physical accuracy, wiring,
and crosstalk still need verification on the actual hardware.
