# Lesson 07 - Home Task: The Multiplication Table

From this lesson on there is no starting code. You write the whole program.
This README is the specification.

## What the program does

1. At startup it prints a 16 x 16 multiplication table, in octal.
2. Each press of the button switches to the next mode and prints the table
   again in that mode:

   ```
   OCT -> DEC -> HEX -> OCT -> ...
   ```

3. Holding the button down does nothing extra. One press is one switch.

## What the table looks like

Rows and columns run from 1 to 16. The labels on the top row and on the left
are printed in the current mode too. This is the start and the end of the
table in hexadecimal:

```
Mode: HEX
   *   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F  10
   1   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F  10
   2   2   4   6   8   A   C   E  10  12  14  16  18  1A  1C  1E  20
   3   3   6   9   C   F  12  15  18  1B  1E  21  24  27  2A  2D  30
   ...
  10  10  20  30  40  50  60  70  80  90  A0  B0  C0  D0  E0  F0 100
```

Every cell is 4 characters wide. The largest value, 16 x 16, is `400` in
octal, `256` in decimal and `100` in hexadecimal: three digits in every mode,
so one width fits all three and the columns stay straight.

## Wiring

| Button leg | Connects to |
|---|---|
| One leg | GPIO 0 |
| The other leg | GND |

Use `INPUT_PULLUP`. Not pressed reads `HIGH`, pressed reads `LOW`. No resistor.

## Requirements

Your program must:

- use `isButtonPressed()` from the lesson, or your own version that does the
  same job: one press is one event, no matter how the contacts bounce or how
  long the button is held
- keep the three format strings in an array, and pick one by the current mode
  number, not with an `if` for every mode
- build each line of the table with a function of this shape, writing into a
  buffer that the caller owns:

  ```cpp
  void formatRow(char *buffer, size_t size, uint8_t row, uint8_t mode);
  ```

- build text with `snprintf` into fixed `char` buffers; no Arduino `String`
- use fixed-width types (`uint8_t`, `uint16_t`, ...) and named constants, as in
  every lesson so far
- print ASCII only, in the table and in every message

How you organise the rest is up to you.

## How to check yourself

| Check | Expected |
|---|---|
| Press once | the mode changes once, never skips one |
| Hold the button for two seconds | exactly one change |
| Bottom-right cell | `400` in OCT, `256` in DEC, `100` in HEX |
| Columns | straight in all three modes |

If a single press sometimes jumps two modes, the button is bouncing and the
debounce is not doing its job.

## Star task: show the mode on the RGB LED

Optional. Wire the RGB LED from lesson 06 back in and light it by mode:

| Mode | Colour |
|---|---|
| DEC | green |
| OCT | cyan |
| HEX | red |

Wiring as in lesson 06: common anode to 3V3, red to GPIO 5, green to GPIO 6,
blue to GPIO 7, each through 100 ohm. Remember that `LOW` switches a colour on.

If you keep the colours in an array next to the formats, the mode number picks
both at once.

## Where your code goes

Same as last time. Your file is `<your-name>/src/main.cpp` in this folder, and
in `platformio.ini` next to this README you uncomment the one `src_dir` line
with your name.
Change that line locally; do not commit `platformio.ini`.

## What to submit

```
git add .
git commit -m "Lesson 07: multiplication table"
git push
```

Deadline: before the next session.

## Common problems

**One press switches two or three modes.** The button bounces. Check that you
react to the change from `HIGH` to `LOW`, and that the pin has to be quiet for
a while before a new press counts.

**The table prints over and over while the button is held.** You are checking
whether the button is down, not whether it has just been pressed.

**The columns are ragged in one mode.** The width is missing from that format
string. All three need the same width.

**Garbage or a crash after the last line.** A buffer is too small for the
line. A full row is 17 cells of 4 characters, plus one byte for the zero at
the end.

**`sizeof(buffer)` inside `formatRow()` gives 4.** That is the pointer, not the
buffer. Use the `size` parameter; that is what it is for.
