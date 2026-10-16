# Lesson 14 homework - two screens and a button

Two parts. Both are due before session 15.

1. Start from the lesson 14 demo and turn it into a small climate station
   with two screens, switched by a button. This time the point is to use
   other people's libraries well: Adafruit PCD8544, Adafruit GFX and
   Adafruit AHTX0.
2. Install PulseView and check that it sees your logic analyzer, so session
   15 starts with measurements instead of drivers.

## Part 1 - the climate station

### Hardware

The setup from class plus one push button:

| Device | Pin | Pico |
|---|---|---|
| 5110 | RST / CE / DC | GP21 / GP17 / GP20 |
| 5110 | DIN / CLK | GP19 / GP18 |
| 5110 | VCC / GND | 3V3 OUT (pin 36) / GND |
| AHT10 | SDA / SCL | GP4 / GP5 |
| AHT10 | VIN / GND | 3V3 OUT (pin 36) / GND |
| Button | one leg / other leg | GP14 (pin 19) / GND |

Use `INPUT_PULLUP` on GP14: not pressed reads HIGH, pressed reads LOW. Never
VBUS for either module.

### Screen 1 - now

The current temperature and humidity, one decimal each, updated every
second. Layout and wording are up to you.

Draw the labels once, when the screen is shown. After that each update
redraws only the two values: clear their area with `fillRect()` in white,
print the new value, call `display()`. Do not clear and redraw the whole
screen every second.

### Screen 2 - temperature history

A bar chart of temperature, drawn with `fillRect()`. Each bar is the
average of 5 readings (5 s), so the chart is calmer than the raw values.
The chart shows the last 14 averages, about the last 70 s:

```
+------------------------------------------+
| T 21.9 .. 24.1                           |  <- text line: lowest and
|                                     ##   |     highest value shown
|              ##                     ##   |
|         ##   ##   ##               ##   |
|    ##   ##   ##   ##   ##          ##   |
|    ##   ##   ##   ##   ##   ##     ##   |
+------------------------------------------+
     oldest                       newest
```

- One bar per 5-reading average, 6 pixels per bar (5 pixels wide + 1 pixel
  gap): 14 bars fill the 84-pixel width.
- A new bar every 5 s. The newest bar is on the right. When all 14 places
  are taken, the oldest average drops off on the left.
- Before 14 averages exist, draw only the bars you have.
- Automatic scale: the lowest value shown gets the shortest bar, the highest
  value the full height of the chart area (everything under the text line).
  Give the shortest bar a minimum height of 2 pixels so it stays visible.
- Show the lowest and highest value on the text line, so the bars have a
  meaning.

### Behaviour

- A new reading every second. Every reading goes into the running sum for
  the current 5-second average; every fifth reading closes the average and
  adds a bar to the history. This happens on both screens: switching to
  screen 2 shows the history collected in the meantime.
- Screen 1 shows every reading. Screen 2 redraws when a new bar arrives.
- One press = one switch, now -> history -> now. Holding the button down
  must not keep switching.
- The screen switches right away when the button is pressed, not at the
  next reading.
- No `delay()` in `loop()`. Timing with `millis()`.
- How you debounce the button is your choice: your `Button` class from K3,
  a library, or your own code. Say which in `notes.md`.

### Things to watch

- What happens to the scale when all 14 averages are the same? Your code
  must not divide by zero.
- `aht.getEvent()` waits for the conversion (about 80 ms) before it
  returns. Once a second that is acceptable, but notice that the button is
  not read during that time.
- `lcd.display()` sends the buffer to the display. Drawing without it shows
  nothing; calling it after every `fillRect()` works but is slow.
- Your `RingBuffer<T, N>` from K3 is a good home for the 14 averages.
- `platformio.ini` already has the three Adafruit libraries in `lib_deps`.
  Start from a copy of `lesson-14/demo/src/main.cpp`.

### Code style

Course style as usual: `stdint.h` types, `U` suffixes, named constants at
the top instead of magic numbers (14 bars, 6 pixels, 1000 ms, 5 readings
per bar, GP14 ...).

## Part 2 - PulseView and the analyzer

1. Download PulseView 0.4.2 from https://sigrok.org/wiki/Downloads (the
   stable release, not a nightly).
2. Install it for your system:
   - **macOS:** open the `.dmg` and drag PulseView into Applications. The
     build is for Intel Macs; on an Apple Silicon Mac (M1 and later) macOS
     offers to install Rosetta 2 on the first start - accept. macOS will
     block the first start because the app is not notarized: System
     Settings -> Privacy & Security -> "Open Anyway". No driver is needed.
   - **Windows:** run the installer. Then plug in the analyzer and run
     Zadig: choose the analyzer (it may show up as "Unknown Device" or with
     an FX2 name), select WinUSB, press Replace Driver. Make sure you picked
     the analyzer and not another USB device.
3. The analyzer has a USB-A cable. A Mac with only USB-C ports needs an
   adapter or a hub.
4. In PulseView choose the driver "fx2lafw (generic driver for FX2 based
   LAs)", interface USB, press Scan. The analyzer must appear in the list.
5. Take one capture: analyzer GND on a Pico GND pin, channel D0 on GP25 (the
   on-board LED, blinking in the demo). 1 MHz, a few seconds.
6. Save a screenshot as `pulseview.png` in your folder.

If it does not work, do not spend the evening on it. Write down exactly what
happened (error text, what the analyzer LED did) in `notes.md` and bring the
laptop and the analyzer to session 15.

## What to commit

In your own folder only:

```
<name>/
  src/            your code
  notes.md        how you debounced the button, what went wrong and how
                  you fixed it, how the PulseView install went
  pulseview.png   the screenshot from part 2 (if it worked)
```

Do not commit `platformio.ini`.

## Check before you commit

- [ ] Builds without warnings from your own files.
- [ ] Press the button ten times quickly: the screen switches exactly ten
      times.
- [ ] Hold the button for 3 s: exactly one switch.
- [ ] Leave it on screen 1 for a minute, then switch: the history is there.
- [ ] Screen 1: the numbers change every second, the labels do not
      flicker.
- [ ] Breathe on the sensor: on screen 1 the value jumps at once; on screen
      2 the next bars grow more gently and the scale follows.
- [ ] `grep -rPn '[^\x00-\x7F]' <name>/` finds nothing (ASCII only, in code
      and in Serial output).
