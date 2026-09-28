# Lesson 08 - Home Task: The Colour Wheel

There is no starting code. You write the whole program. This README is the
specification.

## What the program does

The potentiometer picks a colour, and the RGB LED shows it.

Turning the knob from one end to the other walks once around the colour wheel:

```
red -> yellow -> green -> cyan -> blue -> magenta -> red
```

The position on the wheel is called the hue and runs from 0 to 359. The
lowest position of the knob is hue 0, the highest is hue 359. Every colour is
at full strength: one channel is always at 255, one is always at 0, and the
third one moves.

## Wiring

Both parts come back from earlier lessons.

| Part | Pin | Connects to |
|---|---|---|
| Potentiometer, middle pin | GPIO 4 | |
| Potentiometer, outer pins | | 3V3 through 3.3k, and GND |
| LED, longest pin (common anode) | | 3V3 |
| LED, red | GPIO 5 | through 100 ohm |
| LED, green | GPIO 6 | through 100 ohm |
| LED, blue | GPIO 7 | through 100 ohm |

## From hue to colour

Split the wheel into six slices of 60 degrees. In each slice exactly one
channel changes, and the other two stay put:

| Hue | From | To | red | green | blue |
|---|---|---|---|---|---|
| 0 - 59 | red | yellow | 255 | rises 0 -> 255 | 0 |
| 60 - 119 | yellow | green | falls 255 -> 0 | 255 | 0 |
| 120 - 179 | green | cyan | 0 | 255 | rises 0 -> 255 |
| 180 - 239 | cyan | blue | 0 | falls 255 -> 0 | 255 |
| 240 - 299 | blue | magenta | rises 0 -> 255 | 0 | 255 |
| 300 - 359 | magenta | red | 255 | 0 | falls 255 -> 0 |

Two questions to answer before you write any code: which slice is a hue in,
and how far into its slice is it? Both are one integer operation each.

## Requirements

Your program must:

- carry colours in a `struct Colour` with `red`, `green` and `blue` fields
- drive the LED through a class of your own, with a constructor that takes the
  three pins, a `begin()` and a `setColour(Colour c)`; PWM and the inverted
  logic of the common anode live inside the class and nowhere else
- turn a hue into a colour with a function of this shape:

  ```cpp
  Colour hueToColour(uint16_t hue);
  ```

- print the hue and the three channel values to the serial monitor, only when
  the hue changes
- use whole numbers only, no `float`
- use fixed-width types, named constants and ASCII, as always

The `demo` of this lesson is a good starting point for the class. Read it,
then write your own.

## How to check yourself

| Knob position | Hue | LED | red, green, blue |
|---|---|---|---|
| lowest | 0 | red | 255, 0, 0 |
| about one sixth | 60 | yellow | 255, 255, 0 |
| about one third | 120 | green | 0, 255, 0 |
| half way | 180 | cyan | 0, 255, 255 |
| two thirds | 240 | blue | 0, 0, 255 |
| five sixths | 300 | magenta | 255, 0, 255 |
| highest | 359 | almost red again | 255, 0, a few |

The printed numbers are the real test. The LED itself is harder to judge:
green looks brighter than red at the same value, and that is not your bug.

## Star task: let it spin

Optional. Wire the button from lesson 07 back in (GPIO 0 and GND).

Each press switches between two modes:

- **knob**: the potentiometer picks the hue, as above
- **spin**: the hue moves around the wheel by itself, one full turn every few
  seconds, and the knob is ignored

Use your debounced button function from lesson 07.

## Where your code goes

Your files go in `<your-name>/src/` in this folder, and in `platformio.ini`
next to this README you uncomment the one `src_dir` line with your name.
Change that line locally; do not commit `platformio.ini`.

This time you will have more than one file. Keep all of them, the headers
included, in your own `src` folder. The shared `include` folder belongs to
everyone, and four `RgbLed.h` files in it would overwrite each other. From
`main.cpp` you then include your header with quotes:

```cpp
#include "RgbLed.h"
```

## What to submit

```
git add .
git commit -m "Lesson 08: colour wheel"
git push
```

Deadline: before the next session.

## Common problems

**The LED shows the opposite colour: ask for red, get cyan.** The common anode
inversion is missing. Duty 0 is full brightness on this LED.

**The LED flashes white when the board starts.** A freshly attached PWM
channel sits at duty 0, which here means full on. Switch the LED off at the
end of `begin()`.

**The colour jumps at 60, 120, 180 and so on.** The slice is right but the
position inside it is not starting from zero at each slice boundary.

**The top of the knob stays on one colour.** The potentiometer mapping stops
short. Check your ADC maximum against the value you measured in lesson 05.

**Everything is red or black, no in-between.** You are not using PWM; the pins
are still switched with `digitalWrite`.

**`RgbLed.h: No such file or directory`.** The header is not next to your
`main.cpp`, or it is included with `< >` instead of quotes.
