# Lesson 14 homework - climate dashboard

Two parts. Both are due before session 15.

1. A dashboard on the Nokia 5110: your own display driver from the lab and
   your own `Aht10` class from lesson 13.
2. PulseView installed and your logic analyzer detected, so session 15 starts
   with measurements instead of drivers.

## Part 1 - the dashboard

### What it shows

The screen is 14 characters x 6 lines. Use this layout:

```
CLIMATE
T     23.4 C
RH    41.7 %
T  21.9..24.1
RH 38.0..55.2
up 12 min
```

| Line | Content |
|---|---|
| 0 | a title of your choice |
| 1 | temperature now, one decimal |
| 2 | relative humidity now, one decimal |
| 3 | lowest and highest temperature since start |
| 4 | lowest and highest humidity since start |
| 5 | status: uptime while the sensor works, `SENSOR MISSING` while it does not |

You may change the wording and spacing, as long as every line fits in 14
characters and all six items are there.

### How it must behave

- A new reading every 2 s.
- No `delay()` in `loop()`, and no waiting for the 75 ms conversion. Your
  `Aht10` class already works this way; keep it like that.
- Unplug the sensor: within one reading period the status line shows
  `SENSOR MISSING`. The last good values may stay on lines 1-4, or be
  replaced by dashes - your choice, but say which in your notes.
- Plug the sensor back in: readings resume by themselves. No reset button,
  no re-upload.
- Min / max only ever use good readings. A failed reading must not become a
  new minimum.
- Every SPI transaction goes through your `ChipSelect` guard. No
  `digitalWrite` on the CS pin anywhere else.
- Redraw a line only when its text changed.
- Serial: one line per reading, and one line when the sensor goes missing or
  comes back (not one per second while it is missing).

### Code structure

- Your display driver in its own files, e.g. `Pcd8544.h` / `Pcd8544.cpp`,
  with at least: `begin()`, `clear()`, `gotoXY(col, line)`,
  `drawString(const char*)`.
- `ChipSelect` in its own header or inside the driver files.
- `font5x7.h` copied from `lesson-14/lab/` into your own folder.
- Your `Aht10` class from lesson 13, unchanged if it already worked.
- `main.cpp` only ties them together.
- Course style: `stdint.h` types, `U` suffixes, named constants instead of
  magic numbers, `printf`-style formatting with the correct format for each
  type.

### Hints

- Fixed point keeps formatting simple: store tenths of a degree in an
  `int32_t` (234 = 23.4 C). Mind the sign for values between -1.0 and 0.0.
- `snprintf` into a `char line[15]` never writes past the buffer. Check what
  it returns if the text is too long.
- Keep the previous text of each line to know whether it changed.

### Wiring

Same as in class. All of it can stay wired at once.

| Device | Pin | Pico |
|---|---|---|
| 5110 | RST / CE / DC / DIN / CLK | GP21 / GP17 / GP20 / GP19 / GP18 |
| 5110 | VCC / GND | 3V3 OUT (pin 36) / GND |
| AHT10 | SDA / SCL | GP4 / GP5 |
| AHT10 | VIN / GND | 3V3 OUT (pin 36) / GND |

Never VBUS for either module.

## Part 2 - PulseView and the analyzer

1. Install PulseView from sigrok.org (the stable release for your OS).
2. Windows only: plug in the analyzer and run Zadig. Choose the analyzer
   (it may show up as "Unknown Device" or with an FX2 name), select WinUSB,
   press Replace Driver. Make sure you picked the analyzer and not another
   USB device.
3. In PulseView choose the driver "fx2lafw (generic driver for FX2 based
   LAs)", interface USB, press Scan. The analyzer must appear in the list.
4. Take one capture of anything: for example GP25 blinking, with the analyzer
   GND on a Pico GND pin and channel D0 on GP25. Use 1 MHz and a few
   seconds.
5. Save a screenshot as `pulseview.png` in your folder.

No analyzer at home? Install PulseView, start it with the "Demo device"
driver, take a capture and save that screenshot instead. Bring your laptop to
session 15 with PulseView installed.

## What to commit

In your own folder only:

```
<name>/
  src/            your code, including font5x7.h
  notes.md        what went wrong and how you fixed it (dashboard and PulseView)
  pulseview.png   the screenshot from part 2
```

Do not commit `platformio.ini`.

## Check before you commit

- [ ] Builds without warnings from your own files.
- [ ] Unplug and replug the sensor twice while it runs: it recovers both
      times, min / max stay sensible.
- [ ] Leave it running for 10 minutes: nothing freezes, the uptime keeps
      counting.
- [ ] `grep -rPn '[^\x00-\x7F]' <name>/` finds nothing (ASCII only, in code
      and in Serial output).
