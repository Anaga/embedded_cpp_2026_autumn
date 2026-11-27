# Synth voice button

Connect a normally open momentary pushbutton between **GPIO25** and **board GND**.
The firmware enables the internal pull-up, so only a button and two wires are needed.

```text
ESP32 3.3 V ── internal pull-up ──┬── GPIO25
                                │
                          pushbutton (NO)
                                │
Board GND ───────────────────────┘
```

For a four-leg tactile switch, choose two legs that are disconnected when released
and connected only when pressed. Use one leg from each internally connected pair.
Disconnect USB while wiring. Refer to the
[board pinout](https://github.com/Freenove/Freenove_ESP32_WROVER_Board/blob/main/ESP32_Pinout_1.6.png)
for the GPIO25 header location and
[Espressif's GPIO documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/gpio.html#internal-pullup-and-pulldown)
for the internal pull-up.

A **short press and release while unmuted** advances one voice:

**sine → triangle → pulse → FM → organ → sine**

While muted, a **single short press and release unmutes**, preserving the selected
synth. The following short press can then advance the voice.

Hold for **3 seconds** to toggle mute/unmute. Each hold toggles once, even if kept
pressed longer. Releasing after a long hold does not select a different synth.

Both press and release use **30 ms debounce**. Short presses select on release,
so a long hold can be recognized without also changing voice. A button held
during boot is ignored until released and pressed again.

The current selection is shared with serial `synth` commands, so selecting
`synth pulse` makes the next short press while unmuted select FM. The monitor logs
a voice change once, or `Sound muted.` / `Sound on.` once when mute changes.

Voice changes work while playing or disconnected and preserve pitch, hand
positions, master volume, and the automatic theremin startup. A short press while
stopped and unmuted selects a voice for the next playback. During `showcase`, the scheduled
voices keep playing, and the button changes the selection used after the showcase.

Mute preserves the synth, master volume, and automatic startup request. Hand
tracking and playback keep running silently, so unmuting uses the current hand
positions and playback position. Finite tones/demos can finish while muted.
Serial playback and volume commands also respect button mute; `volume 0` still
keeps output silent after unmuting. `status` prefixes the playback mode with
`muted/` and reports zero output drive while muted.

Explicit stops, Bluetooth disconnects, and remote audio suspension still stop
playback. Unmuting does not restart stopped or completed playback. A board restart
clears mute and keeps the normal automatic theremin startup.

The red/green LED indications continue to show Bluetooth and synth choice while
muted. Button wiring remains GPIO25 to GND.

[`SynthButton`](../include/SynthButton.h) encapsulates GPIO initialization,
nonblocking debounce, short/long events, and voice order. The main task publishes
voice or mute changes under the existing lock. [`Playback`](../include/Playback.h)
applies mute to the generated samples in the audio task without resetting the
oscillators or sequencing.
