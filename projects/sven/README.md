# Sounder

Sounder is a theremin-style instrument in development. The current firmware
runs on the Freenove ESP32-WROVER and sends synthesized audio to a JBL
Flip 4 over Bluetooth Classic A2DP. Both-hand theremin mode starts automatically
once the speaker connects and audio streams; USB serial provides manual controls.
Two ultrasonic modules provide live pitch and volume control and can also be
tested independently over serial. See the
[sensor wiring and capability guide](docs/ultrasonic-sensors.md) for the pinout,
3.3 V connections, and `sensors on` demo. The
[project proposal](final-project-proposal.md) describes the instrument.
An optional [GPIO25 button](docs/synth-button.md) cycles voices:
**sine → triangle → pulse → FM → organ → sine**, once per short press/release while
unmuted. While muted, a short click unmutes and preserves the current synth.
Hold for **3 seconds** to toggle mute/unmute. Each hold toggles once and preserves
the current voice, hand controls, and master volume.
Two [status LEDs](docs/status-leds.md) use GPIO27 (red) and GPIO26 (green), each
with its own 200 Ω resistor. Red blinks while Bluetooth is disconnected and turns
off when connected. Green identifies sine, triangle, pulse, FM, and organ by
blinking at 0.5, 1, 2, approximately 3, and 4 times per second, respectively.

## Build and try it

Connect the Freenove board over USB and run these PlatformIO commands from the
repository root.

```sh
pio run
pio run --target upload
pio device monitor
```

Use `~/.platformio/penv/bin/pio` if `pio` is unavailable on your PATH.
In CLion, click **Reload PlatformIO Project**, then select
**freenove_esp32_wrover** in the environment dropdown in the main toolbar.

If several boards are connected, add `--upload-port /dev/cu.YOUR_ESP32` to the
upload command and `--port /dev/cu.YOUR_ESP32` to the monitor command. You may
need to hold BOOT as the upload starts. Close the serial monitor with Ctrl-C
before uploading again.

1. Power on the Flip 4 and press its **Bluetooth** button to enter pairing mode.
   Disconnect phones or computers that are already using the speaker.
2. Boot the ESP32 and open its 115200-baud serial monitor. It searches for the
   exact name `JBL Flip 4`. If your speaker was renamed, edit `kSpeakerName` in
   `src/main.cpp`, rebuild, and upload. Each boot discovers the configured
   speaker name.
3. Wait for `Bluetooth connected` and `audio streaming` messages. The ESP32
   automatically starts `theremin on` once per boot and waits for an initial
   reading from both hands before sounding. A serial monitor is optional.
   The speaker may play its own connection notification; its buttons set its volume.
4. Send `tone 440 1500` for a first test, then `demo` for the full tour.

The configured monitor sends each line when Enter is pressed. `!` is recognized
without a newline by the firmware, but this monitor still buffers it until Enter.

## Volume from the monitor or terminal

Change or query the synthesized volume in the serial monitor.

```text
volume
volume 20
volume 0
volume 35
status
```

`volume` queries the current value; `volume 0` mutes without stopping playback,
and `volume 35` restores the default. `level 20` remains an alias for
`volume 20`. Volume changes apply to running playback at the next audio callback and also work while
disconnected. They control generated-audio amplitude; the Flip 4's own volume
buttons remain an additional control.

Run `pio device list` to find the Freenove's USB serial port. Close an existing
monitor before opening another session on the same serial port. Volume resets
to 35% on board restart. The GPIO25 button's mute is independent of this master
level: volume changes do not clear button mute, and unmuting preserves the level.
Playback and hand tracking continue while button-muted; `status` shows `muted/`
before the playback mode. Reboot clears button mute.

## Live pulse vibrato with your hands

Upload the firmware, open the monitor, and wait for the Flip 4 to connect and
start streaming. Use the wiring in the [sensor guide](docs/ultrasonic-sensors.md).
Start with the pulse voice and pitch sensor alone:

```text
synth pulse
volume 20
theremin pitch
```

Hold a hand 5–50 cm in front of the **pitch sensor (Trig GPIO32 / Echo GPIO34)**. Moving closer
raises the center pitch from 220 Hz at 50 cm to 880 Hz at 5 cm; 27.5 cm gives
440 Hz. The existing vibrato effect runs around that center at 5 Hz, ±50 cents.
The output level stays at the master `volume` setting for this pitch-only test.

Then test only the **volume sensor (Trig GPIO33 / Echo GPIO35)**:

```text
theremin volume
```

The center pitch stays at 440 Hz with vibrato. Moving closer from 50 to 5 cm raises
amplitude from silence to the master volume ceiling. `volume 0` still mutes;
`volume 20` restores the 20% ceiling. The speaker's own volume remains separate.

Use both hands together:

```text
theremin on
theremin status
```

Both sensors must first see a hand within range before both-hands mode sounds.
Readings from 2–5 cm clamp to the near endpoint; readings beyond 50 cm are
ignored as background. A dedicated
`HandDistanceFilter` tracks each hand independently before musical mapping:

- Changes over 4 cm require a second new reading within 2 cm of the candidate.
  An isolated spike holds the previous position rather than changing the sound.
- A 0.15 cm deadband reduces small stationary variations. Accepted movement
  uses a 50 ms smoothing time constant and a 30 cm/s output speed limit, so even
  confirmed large changes approach their new position gradually.
- When a hand leaves the beam or its readings become unusable, that control
  freezes at its last output indefinitely. A 250 ms gap in accepted readings
  also freezes tracking. The other hand can continue changing its own control.
- A returning hand needs two consistent readings, then its new absolute position
  takes over directly, even if it differs substantially from where it left.
  There is no need to move back through the old position or wait for the slow
  movement limit to traverse the gap. The short audio glide still avoids clicks.
  `sensors reset` clears counters without resetting these held positions.

Leaving both sensors continues the last center pitch and amplitude, with vibrato.
Moving through the valid range still changes the sound before leaving it; the
value held is the last one reached, rather than the value before starting that
movement. Move sideways out of the beam to leave at a particular level.

The audio stage also glides pitch over 20 ms and fades hand amplitude over 15 ms.
Live modes sample each sensor every 70 ms (about 14 attempts/second), alternating
modules 35 ms apart. Diagnostic `sensors` modes retain their 200 ms interval
(five attempts/second). Both keep the 200 ms startup wait and throttled logging.

Normal gentle movements use the fast sampling schedule without waiting for a
moving-average window. Confirmation adds one sensor interval for large changes,
and smoothing deliberately trades some response time for stability. Bluetooth
and the speaker still buffer audio; their delay has not been measured here. Compare
`theremin pitch` and `theremin volume` with `theremin on` to check crosstalk
and responsiveness on the actual modules.

`theremin status` shows center pitch and hand volume targets; `status` shows
the last rendered, vibrato-modulated pitch. Logging shows pitch and volume on
one line at most once per second. In live mode, `raw` is the measured distance,
`hand` is the filtered distance used by the instrument. `(hold)` marks a frozen
position; `n/a` means no initial position has been accepted yet. `held` counts
unconfirmed large changes. Raw pulse counts and min/max still
include rejected spikes; `ok` only means a complete pulse in the 2–450 cm range.
`synth` can change voices during the live test.

Use `stop`, `!`, or `theremin off` to silence live audio. Sensor diagnostics
continue until `sensors off`. A normal tone/pattern replaces live audio when
consumed; changing sensor acquisition mode also ends live audio. Queries and
`sensors reset` keep it running. Disconnect/suspension cancels the live test;
start it explicitly again after reconnecting.

A traditional theremin continuously responds to its antenna fields: closer to
the pitch antenna raises pitch, and closer to the volume loop reduces volume.
See the [Moog playing guide](https://cdn.inmusicbrands.com/Moog/Theremini/Theremini%20Manual.pdf),
pages 23–24. This project uses the requested hold-on-exit interaction and
closer-means-louder volume mapping.

## Serial commands

Commands are case-insensitive. Playback commands require a connected speaker;
commands sent while disconnected are rejected instead of saved for later.
Sensor commands work independently of Bluetooth. At boot, sensors wait for
Bluetooth audio, then start automatically in both-hand live mode. `stop`, `!`,
`theremin off`, a sensor acquisition command, or an accepted playback/theremin command
cancels the pending startup. Queries, volume/synth settings, `sensors reset`, and
rejected commands preserve it.

| Command | Behavior |
| --- | --- |
| `demo` | Play the complete 45.2-second tour once. |
| `showcase` | Play all nine demos on each of the five synth voices. Takes 3 minutes 46 seconds. |
| `1` or `scale` | C4 to C5 and back. |
| `2` or `sweep` | 100 to 8000 Hz and back. |
| `3` or `levels` | Seven amplitude steps at 1000 Hz. |
| `4` or `vibrato` | 440 Hz with ±50 cents pitch modulation at 5 Hz. |
| `5` or `tremolo` | Amplitude modulation at 4 Hz. |
| `6` or `envelope` | Four shaped 880 Hz notes. |
| `7` or `arp` | Fast C major arpeggio. |
| `8` or `siren` | Glide between 400 and 1600 Hz. |
| `9` or `echo` | One hit and four weaker repeats. |
| `tone 440` | Continuous tone. Range: 100–8000 Hz. |
| `tone 440 1500` | Timed tone. Duration range: 10–30000 ms; fractional ms are truncated. |
| `note 69` | Continuous MIDI note. Integer range: 45–119. |
| `synth` | Show the selected synth voice. |
| `synth sine` / `synth triangle` | Choose a simple waveform. The board starts with sine. |
| `synth pulse` | Arcade Pulse. Try `arp` for a retro game sound. |
| `synth fm` | FM Robot. Try `siren` for a metallic glide. |
| `synth organ` | Organ. Try `scale` for a layered, steady tone. |
| `volume` | Query the current PCM amplitude. |
| `volume 20` | Set amplitude to 0–100%, default 35%. Works while disconnected; 0 mutes. |
| `level 20` | Alias for setting amplitude, with the same range as `volume`. |
| `stop` or `!` | Silence future samples and invalidate pending playback commands. |
| `sensors on` / `sensors demo` | Measure both modules at 5 Hz each; log both on one line at most once/second. No speaker needed. |
| `sensors pitch` / `sensors volume` | Test one ultrasonic module at a time. |
| `sensors` / `sensors status` | Show sensor mode and configured timing/range. |
| `sensors reset` / `sensors off` | Clear measurement statistics / stop measurements. |
| `theremin pitch` | Live vibrato: pitch hand controls 220–880 Hz center; fixed master volume. |
| `theremin volume` | Live vibrato at a 440 Hz center; volume hand controls amplitude. |
| `theremin on` | Live vibrato with both pitch and volume hands. Boot default after Bluetooth audio is ready. Uses the selected synth voice. |
| `theremin` / `theremin status` | Show live control mode, center pitch, and volume targets. |
| `theremin off` | End live audio; sensor diagnostics continue. |
| `status` | Speaker connection, streaming state, synth voice, rendered mode/pitch, and amplitude. |
| `help` | Command menu. |

A new tone or pattern replaces the current one at the next audio callback.
Acknowledgments say `queued`; `status` reports the last rendered buffer, so it
can briefly show the previous mode. Eight playback commands can be pending;
additional commands are rejected explicitly. Stop works even when that queue
is full. Malformed numbers, non-finite values, extra arguments, and overlong
lines leave playback unchanged.

Every voice works with every demo and tone. To compare voices, replay the same
pattern with a different voice or switch during `demo`. The selected voice
takes effect on the next audio callback, and the tour keeps its place. You can
select a voice before connecting the speaker.

`showcase` plays the full tour on sine, triangle, pulse, FM, then organ.
The serial monitor shows the current voice and demo. Use `stop` or `!` to
end it early. The voice you choose with `synth` applies to regular playback
after the showcase.

On disconnect or remote suspension, pending playback is invalidated. After a
successful connection the firmware arms three reconnect attempts, then falls
back to discovery. Once the boot default has started or been overridden,
reconnection stays silent until a new playback command.
If pairing fails, confirm the speaker name and pairing mode, keep other audio
sources disconnected, and restart the board. `status` distinguishes discovery,
connection, and streaming.

## Audio and dependency choices

- **AudioTools 1.2.6** supplies `FastIntSineGenerator<int16_t>`. Sounder also
  implements triangle, 25% duty pulse with smoothed edges, 2:1 FM, and an
  additive organ with three harmonics. All voices share the demo pitch and
  amplitude controls and retain phase across pitch changes. Samples are
  generated at 44.1 kHz and duplicated into the two signed 16-bit channels
  expected by **ESP32-A2DP 1.8.11**, which encodes SBC.
- The musical control clock advances from generated stereo frames, retaining
  the fractional samples per millisecond. Effects update every 5 ms, independent
  of serial activity and Bluetooth callback size. The audio callback owns
  the demo engine and synthesizer. FreeRTOS queues and short critical sections
  transfer commands and status between tasks. AudioTools logging is disabled
  in the callback path.
- `HandDistanceFilter` rejects isolated range jumps and produces a bounded,
  smoothed hand position. `ThereminController` maps accepted readings to musical targets.
  `ThereminStartup` requests the boot default once audio is ready and yields to manual modes.
  `LiveTone` glides pitch and amplitude in the audio task. `Vibrato` supplies
  the same 5 Hz/±50-cent modulation to the original pattern and live controls.
  Live targets use a shared snapshot rather than filling the command queue;
  stop, disconnect, and suspension invalidate the live session.
- The demo engine supplies pitch and amplitude for each pattern. The echo
  pattern plays a note followed by four quieter repeats. The speaker's own
  volume buttons control the output level independently.
- Espressif32 remains pinned to **7.1.3 / Arduino ESP32 2.0.17**. The build adds
  the A2DP audio-state compatibility definition needed by this older SDK.
  Both audio libraries are pinned to exact release commits.
- `huge_app.csv` gives the 4 MB board a 3 MB application partition for USB
  uploads.

Bluetooth buffers and compresses audio. Commands and stop therefore have
audible latency. Test how it feels on the actual board and speaker.

## Verification

Run the native suite from the repository root.

```sh
pio test -d tests -e native
```

The tests compile the real firmware and demo engine with simulated
Arduino, FreeRTOS, Bluetooth, and oscillator APIs. They check the adapter's
clock, channel layout, commands, lifecycle, and stop/reconnect races. See
[the local test setup](tests/README.md) for CLion and CMake.

For hardware acceptance, play a timed tone and the complete tour; exercise
`volume`, `volume 0`, `volume 35`, `volume 100`, `level 20`, invalid commands,
and stop from both the attached monitor and a terminal session. Power-cycle the
speaker during a tone and verify reconnection remains silent until commanded.
Verify sweeps and vibrato remain smooth and that the serial monitor stays
responsive during playback. These steps require an original ESP32 and Flip 4.
For live acceptance, try `synth pulse`, `volume 20`, then `theremin pitch`,
`theremin volume`, and `theremin on`. Verify the 5/27.5/50 cm landmarks, held
sound after removing either hand, return at a different position, `volume 0`,
stop, replacement by a normal
pattern, and silence after reconnect. Firmware/native tests simulate these
controls but cannot verify physical sensor readings or the sound of the speaker.

## Research references

- [PlatformIO: Freenove ESP32-WROVER target and upload protocol](https://docs.platformio.org/en/latest/boards/espressif32/freenove_esp32_wrover.html).
- [Freenove ESP32-WROVER board resources](https://github.com/Freenove/Freenove_ESP32_WROVER_Board).
- [JBL Flip 4 specifications, including A2DP](https://support.jbl.com/on/demandware.static/-/Sites-masterCatalog_Harman/default/dw2b1c7c46/pdfs/JBL%20Flip4%20Spec%20Sheet%20English.pdf).
- [AudioTools generated-audio Bluetooth example](https://github.com/pschatzmann/arduino-audio-tools/tree/v1.2.6/examples/examples-communication/a2dp/streams-generator-a2dp).
- [AudioTools 1.2.6 release](https://github.com/pschatzmann/arduino-audio-tools/releases/tag/v1.2.6).
- [ESP32-A2DP source documentation](https://github.com/pschatzmann/ESP32-A2DP/wiki/A2DP-Source).
