# Lesson 02 - First Firmware

PlatformIO, project anatomy, blink, serial output, reading a button,
and the git workflow used for every assignment in this course.

## Hardware

ESP32-C3 Super Mini, bare board. Nothing to wire - this lesson uses
only what is soldered on:

| Function | Pin | Notes |
|---|---|---|
| Blue status LED | GPIO 8 | active low: LOW = ON |
| BOOT button | GPIO 9 | active low; strapping pin, see below |
| RESET button | CHIP_PU | restarts the chip; not a GPIO |

GPIO 2, 8 and 9 are strapping pins - their state at reset selects the
boot mode. In this lesson that means one practical rule: do not hold
the BOOT button while the board resets, or the chip enters the serial
bootloader instead of running your firmware. (Held BOOT + tap RESET
is also the manual recovery combo if an upload ever refuses to
start.) Held at any other time, BOOT is just a button your code can
read.

## Building

1. Open this folder (`M1_L2`) in VS Code. PlatformIO picks up
   `platformio.ini` automatically.
2. Build: the check mark in the status bar.
3. Upload: the right arrow. The board must be connected over USB.
4. Monitor: the plug icon. Speed is 115200. Exit with Ctrl+C.

The first build downloads the ESP32 toolchain (about 1 GB). It is slow
once and fast afterwards. Do not cancel it.

### If upload fails or the port is missing

1. Close the serial monitor - it holds the port.
2. Try another USB cable. Charge-only cables have no data lines and
   are the single most common cause.
3. Try the other USB connector on the board.

## Demo (`src/main.cpp`)

The instructor demo from class. The blue LED blinks slowly; while
the BOOT button is held, it blinks fast. Every toggle is reported over
Serial. Complete and working - build it first to prove your setup.

## Homework (`homework/src/main.cpp`)

Make the LED blink the SOS pattern: three short pulses, three long
pulses, three short pulses, then a pause, repeating forever. While
the BOOT button is held down, the LED stays off and the pattern does
not advance.

The scaffold compiles as is. The timing constants and a `pulse()`
helper are provided; your work is the logic in `loop()`, marked with
`// TODO:` comments.

To build the homework instead of the demo, uncomment the `src_dir`
line at the top of `platformio.ini`:

```
[platformio]
src_dir = homework/src
```

(Comment it back to rebuild the demo.)

### Definition of done

- The rhythm is readable as SOS from across the room.
- Holding BOOT stops the LED; releasing it resumes the pattern.
- One line is printed over Serial per completed cycle.
- No magic numbers inside `loop()` - timing lives in the named
  constants at the top.

## Handing in

```
git pull
git add lesson-02/
git commit -m "lesson-02: SOS blinker homework"
git push
```

Push before the next session. Commit messages in English, ASCII only.
Never commit the `.pio/` directory - it is build output and the
repository's `.gitignore` already excludes it.
