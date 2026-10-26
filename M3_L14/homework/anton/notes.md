# Lesson 14 homework notes - anton

## Button debouncing

Used the `Button` class from lesson 13 (K3), adapted for INPUT_PULLUP on GP14.
The class uses a time-based debounce: after any pin change, it waits 30 ms
of quiet time before accepting a new press. This ignores both the bouncing
of a press and the bouncing of a release.

Key points:
- `wasPressed()` returns true exactly once per press
- Holding the button down does not generate repeated presses
- Bounces after press and after release are both suppressed

## PulseView installation

Installed PulseView 0.4.2 on Windows 11:
1. Downloaded the stable release installer from sigrok.org
2. Ran the installer
3. Connected the logic analyzer (FX2-based)
4. Ran Zadig, selected the analyzer (appeared as "Unknown Device"),
   chose WinUSB, clicked "Replace Driver"
5. In PulseView: selected driver "fx2lafw (generic driver for FX2 based LAs)",
   interface USB, clicked Scan - the analyzer appeared in the list
6. Captured GP25 (on-board LED) at 1 MHz for a few seconds - capture successful
7. Saved screenshot as `pulseview.png`

No issues encountered. The analyzer LED blinked during capture as expected.

## What went wrong and how I fixed it

- Initially the history screen scale calculation divided by zero when all
  14 averages were identical. Fixed by checking `range > 0.0F` before
  computing the fraction, and giving the shortest bar a minimum height of 2 px.
- The `dtostrf()` function is not standard C++ but works in the Arduino
  environment; used it for serial output formatting.
- Software SPI with explicit pins (CLK, DIN, DC, CS, RST) works reliably
  at the default 4 MHz clock cap.

## Build verification

- Builds without warnings from own files
- Button pressed 10 times quickly: screen switches exactly 10 times
- Button held for 3 s: exactly one switch
- Left on screen 1 for a minute, then switched: history is there
- Screen 1: numbers change every second, labels do not flicker
- Breathing on sensor: screen 1 value jumps at once; screen 2 bars grow
  more gently and scale follows
- ASCII check: no non-ASCII characters in code or serial output