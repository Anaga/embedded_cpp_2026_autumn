# Lesson 07 - Demo

The program shown in class. It is complete and works as it is; read it, run
it, change it.

## Wiring

| Button leg | Connects to |
|---|---|
| One leg | GPIO 0 |
| The other leg | GND |

No resistor: `setup()` switches on the internal pull-up.

## Build and upload

1. Open this `demo` folder in VS Code with PlatformIO.
2. Build, upload, open the serial monitor at 115200.
3. Press the reset button on the board if the monitor is still empty.

## What it prints

At startup the program runs five experiments, one per part of the lesson,
and then waits for the button.

| Part | What to look for |
|---|---|
| 1. Arrays | `readings[0]` is the first box, `readings[4]` the last; five `uint8_t` take 5 bytes |
| 2. Where things live | the global and the static local sit close together; the two locals are somewhere else, the deeper one lower; the text and the code are in a completely different range |
| 3. Pointers | `addOne()` changes a variable it does not own; `minMax()` returns two answers; `sizeof` gives two different sizes for the same array |
| 4. Strings | `"Hi"` takes 3 bytes, not 2; the same 255 in three formats |
| 5. The button | a line each time the press counter changes |

Your addresses in part 2 will not match anyone else's. Compare which ones are
near each other, not the numbers themselves.

## The button: three modes

The constant `BUTTON_MODE` near the top of `src/main.cpp` picks how the button
is read. Try them in this order. Change the constant, upload, press the button
a few times, watch the `(+n)` at the end of each line.

| `BUTTON_MODE` | How it reads the button | What one press adds |
|---|---|---|
| `MODE_LEVEL` | counts while the button is down | thousands |
| `MODE_EDGE` | counts the change from released to pressed | usually 1, sometimes 2 to 5 |
| `MODE_DEBOUNCED` | uses `isButtonPressed()` | exactly 1, every time |

The first mode shows that holding is not pressing. The second fixes that, and
what is left over is bounce: the metal contacts clatter for a few
milliseconds, and the chip sees every clatter. The third ignores any change
that comes too soon after the previous one.

If `MODE_EDGE` gives a clean 1 every time, your button happens to bounce
little. Press it quickly and slightly off-centre and it will start.

## Things to try

- Set `DEBOUNCE_MS` to 1 and use `MODE_DEBOUNCED`. Do the extra counts come
  back? Then try 200 and press as fast as you can. Which presses are lost now?
- Hold the button for two seconds in each mode, then let go. Which mode counts
  the release?
- In part 2, add a second global and a second static local. Where do they land?
