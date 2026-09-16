# Lesson 05 - Variables, Types and Arithmetic

## What this lesson covers

- Memory, variables, and what a type actually decides
- Fixed-width types and the range each of them holds
- What happens when a value runs past the edge of its type
- Whole-number arithmetic: division that cuts, and why the order matters
- Reading a potentiometer and printing a temperature

## Files

```
lesson-05/
  slides.pptx         the deck used in class
  demo/               the program shown in class, complete and working
  homework/           your starting point, with TODOs
  README.md           this file
```

## Hardware

- ESP32-C3 Super Mini
- 10k linear potentiometer
- Breadboard and three jumper wires

### Wiring

| Potentiometer pin | Connects to |
|---|---|
| Middle pin (wiper) | GPIO 4 |
| One outer pin | 3V3, through a 3.3k resistor |
| Other outer pin | GND |

The three legs go straight into the breadboard. No soldering.

The resistor between 3V3 and the potentiometer is there for a reason. The
analog input stops responding above roughly 2.9 V, so with the potentiometer
wired directly to 3V3 the last part of the travel reads the same value and the
temperature stops changing. The resistor keeps the whole turn inside the range
the input can see. We come back to this properly in Session 11.

If the knob works backwards - clockwise makes it colder - swap the two outer
pins. Nothing is damaged, the direction simply reverses.

## Build and upload

1. Open the `homework` folder in VS Code with PlatformIO.
2. Build (the check mark in the status bar).
3. Upload (the arrow).
4. Open the serial monitor (the plug icon). Speed is 115200.

You should see lines like this when you turn the knob:

```
F =  +72   C =   +0
```

The Fahrenheit value already works. The Celsius value is your job.

## Your task

Open `src/main.cpp` and find `TODO 1`.

Write the body of `fahrenheitToCelsius()` so that it returns the temperature in
whole degrees Celsius. The formula is the one from the board. Turning it into
working C is the exercise.

Rules:

- whole numbers only, no `float` and no `double`
- fixed-width types: `int16_t`, `uint8_t` and friends, never a bare `int`
- no numbers buried inside the function body; give them names at the top of
  the file, the way the existing constants are written

## How to check yourself

Two positions of the knob tell you whether the formula is right:

| Turn the knob until | Fahrenheit | Celsius must read |
|---|---|---|
| The lowest value | -40 | -40 |
| A little above the lowest | 32 | 0 |

Minus forty is the one temperature where both scales agree. If your program
prints -40 and 0 at those two points, the formula is correct. If it prints 0
everywhere, or numbers that are close but not exact, look at the order of the
operations again.

## Bonus, if you have time

- `TODO 2`: the same temperature in tenths of a degree, so that 21.5 C prints
  as `21.5`. Still no float.
- `TODO 3`: change the Fahrenheit variable in `loop()` from `int16_t` to
  `int8_t`, turn the knob all the way up, and explain what happens. Write the
  explanation in your commit message.

## What to submit

Commit your work and push it to the course repository:

```
git add .
git commit -m "Lesson 05: Fahrenheit to Celsius"
git push
```

Your files go under `M2_L5\homework\src\<student_name>`.

Deadline: before the next session.

## Common problems

**The serial monitor stays empty.** Press the reset button on the board once
after opening the monitor. If it is still empty, check that the monitor is
open on the right COM port and that no other program is holding it.

**Upload fails with a port error.** Close the serial monitor and upload again.
Only one program can hold the port at a time.

**The temperature jumps around by a degree.** That is the analog input, not
your code. The program already averages several readings; the rest is normal
and we deal with it in Session 11.

**The value stops changing near one end of the travel.** Check that the
resistor to 3V3 is in place.

**Everything prints 0 for Celsius.** The function still returns its starting
value, or the division happened before the multiplication.
