# Lab 1 - Deep sleep, timer wake and button wake

The board is awake for 30 s, sleeps for 60 s, wakes up by itself and
starts again. Button A wakes it at any time. A counter in RTC memory shows
what survives deep sleep and what does not.

## The LED tells you what the chip is doing

| LED | Meaning |
|---|---|
| red blink | just woke up: `setup()` starts again from the top |
| blue | awake: the USB COM port works, an upload can start |
| green | asleep: the pin is held LOW through deep sleep |

## Wiring

Same as session 18. Only these are used here:

| Part | Pin | Notes |
|---|---|---|
| Button A | GPIO 0 -> button -> GND | `INPUT_PULLUP`, active low. Only GPIO 0..5 can wake the C3 |
| RGB LED | R 5, G 6, B 7 | common anode to 3V3, LOW = on |

## Run

1. Open `M4_L19/lab1-sleep` in VS Code, build, upload, open the monitor.
2. Watch the LED and the monitor together.

Expected output (your numbers differ):

```
Lesson 19 - Lab 1 - deep sleep
#1 wake: cold start (power-on or reset)
   awake 30 s - LED blue, upload now
   sleeping 60 s - LED green
#2 wake: timer
   prev awake 30302 ms
   awake 30 s - LED blue, upload now
   sleeping 60 s - LED green
#3 wake: button
   prev awake 30298 ms
   awake 30 s - LED blue, upload now
   sleeping 60 s - LED green
```

When the LED turns green, the COM port goes away; it comes back after the
red blink. That is normal: the C3's USB is inside the chip and powers off
in deep sleep. The PlatformIO monitor reconnects by itself.

The board sleeps on a USB cable too. The chip powers down; the power LED
and the regulator on the board stay on, because USB still supplies them.

## Uploading to a sleeping board

There is no USB port while the LED is green, so an upload fails with
"port not found" or similar.

1. Build first.
2. Press RESET or button A. The LED goes red, then blue for 30 s.
3. Click Upload while it is blue.

Last resort: hold BOOT, press and release RESET, release BOOT. The board
now waits in download mode until an upload arrives.

## Things to try

- Press RESET while the board sleeps. Does `bootCount` go back to 1?
- Unplug USB and plug it back in. And now?
- Set `SERIAL_WAIT_MS` to 0. Which lines disappear from the monitor, and why?
- Hold button A down while the LED turns green. What does
  `waitForRelease()` prevent?
- The green LED during sleep is for us. What would a battery device do with
  the LED pins before sleeping?

## Troubleshooting

- **The board wakes again and again without a press.** The button input
  floats during sleep. Add an external 10k pull-up from GPIO 0 to 3V3.
- **The LED goes dark instead of green during sleep.** The pin hold did not
  work on this board; tell the instructor.
- **Nothing in the monitor after a wake.** Wait for the port to come back,
  or press RESET to start over with a cold start.
