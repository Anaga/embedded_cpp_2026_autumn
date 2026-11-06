# Lab 1 - Deep sleep, timer wake and button wake

The board sleeps 20 s, wakes up by itself, prints one line and sleeps
again. Button A wakes it at any time. A counter in RTC memory shows what
survives deep sleep and what does not.

## Wiring

Same as session 18. Only these are used here:

| Part | Pin | Notes |
|---|---|---|
| Button A | GPIO 0 -> button -> GND | `INPUT_PULLUP`, active low. Only GPIO 0..5 can wake the C3 |
| RGB LED | R 5, G 6, B 7 | common anode to 3V3, LOW = on. Green flashes on every wake |

## Run

1. Open `M4_L19/lab1-sleep` in VS Code, build, upload, open the monitor.
2. Wait. The board prints, stays awake 10 s, then sleeps 20 s.

Expected output (your numbers differ):

```
Lesson 19 - Lab 1 - deep sleep
#1 wake: cold start (power-on or reset)
   awake 10 s - upload now
   sleeping 20 s
#2 wake: timer
   prev awake 12051 ms
   sleeping 20 s
#3 wake: button
   awake 10 s - upload now
   prev awake 2006 ms
   sleeping 20 s
```

The monitor shows the port going away and coming back after each wake.
That is normal: the C3's USB is inside the chip and powers off in deep
sleep. The PlatformIO monitor reconnects by itself.

## Uploading to a sleeping board

There is no USB port while the board sleeps, so an upload fails with
"port not found" or similar.

1. Build first.
2. Press RESET or button A. The board stays awake 10 s.
3. Click Upload inside those 10 s.

Last resort: hold BOOT, press and release RESET, release BOOT. The board
now waits in download mode until an upload arrives.

## Things to try

- Press RESET while the board sleeps. Does `bootCount` go back to 1?
- Unplug USB and plug it back in. And now?
- Set `SERIAL_WAIT_MS` to 0. Which lines disappear from the monitor, and why?
- Hold button A down for 3 s while the board goes to sleep. What does
  `waitForRelease()` prevent?
- Where do the 2000 ms of "prev awake" come from on a timer wake?

## Troubleshooting

- **The board wakes again and again without a press.** The button input
  floats during sleep. Add an external 10k pull-up from GPIO 0 to 3V3.
- **The RGB LED glows faintly while the board sleeps.** The LED pins are
  held HIGH with `gpio_hold_en()`; check that nothing else drives them.
- **Nothing in the monitor after a wake.** Wait for the port to come back,
  or press RESET to start over with a cold start.
