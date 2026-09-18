# Lesson 06 - Functions, Scope and Bits

## What this lesson covers

- Writing your own functions: parameters, return values, prototypes
- Where a variable is visible and how long it lives
- The stack: what a call and a return actually do
- `static`: the local variable that survives the return
- Bits, masks and shifts
- An RGB LED driven by three bits

## Files

```
lesson-06/
  lesson06.pptx
  L6_demo/            the program shown in class
  homework/           your starting point, with TODOs
  README.md           this file
```

## Hardware

- ESP32-C3 Super Mini
- 10k linear potentiometer, as in the last lesson
- RGB LED, 5 mm, common anode (500RGB4E)
- Three 100 ohm resistors
- Breadboard and jumper wires

### Wiring

| LED pin | Connects to | Through |
|---|---|---|
| Longest pin (common anode) | 3V3 | direct |
| Red | GPIO 5 | 100 ohm |
| Green | GPIO 6 | 100 ohm |
| Blue | GPIO 7 | 100 ohm |

The potentiometer stays where it was: middle pin to GPIO 4, outer pins to 3V3
through the 3.3k resistor and to GND.

**Common anode means the logic is inverted.** `LOW` switches a colour on,
`HIGH` switches it off. That is the same backwards logic as the LED on the
board itself. `setColour()` already handles it for you.

Check the pin order with the diode test on your multimeter before trusting the
drawing. The longest pin is the common one.

## The colour code

A colour is three bits:

| Bit | Channel | Value |
|---|---|---|
| 0 | Red | 1 |
| 1 | Green | 2 |
| 2 | Blue | 4 |

Five of the eight combinations are used:

| Temperature, Celsius | Band | Colour | Bits |
|---|---|---|---|
| below 0 | cold | blue | 100 |
| 0 to 14 | cool | cyan | 110 |
| 15 to 24 | normal | green | 010 |
| 25 to 34 | warm | yellow | 011 |
| 35 and above | hot | red | 001 |

## Where your code goes

Everyone works in the same project, so each of you has your own source folder:
`sven/src`, `viktorija/src`, `anton/src` and `vahur/src`. Copy `src/main.cpp`
into yours and work there.

Then open `platformio.ini` and uncomment the one line that points at your
folder, leaving the others commented:

```ini
[platformio]
;src_dir = src
src_dir = sven/src
```

Exactly one line must be active. If two are uncommented, the last one wins and
you will spend ten minutes wondering why your changes do nothing.

Change that line on your own machine, but do not commit `platformio.ini`. If
all four of you push a different version of it, every pull turns into a merge
conflict.

## Build and upload

1. Open the `homework` folder in VS Code with PlatformIO.
2. Build, upload, open the serial monitor at 115200.
3. Turn the knob. The temperature prints, but the LED stays dark until you
   write the missing function.

## Your task

`TODO 1` in `src/main.cpp`: write `colourForTemperature()` so that it returns
the right colour for a temperature, following the table above.

Rules:

- give every boundary a name at the top of the file, next to the other
  constants; no bare numbers inside the function
- return the `COLOUR_` constants that are already defined, not raw numbers
- whole numbers only

## Star task

Turn the knob until the temperature sits exactly on a boundary, for example on
24 or 25 degrees, and leave it there. The colour will jump back and forth,
because the reading moves by a degree on its own.

`TODO 2`: make it stop. The colour should keep its band until the temperature
has moved far enough to really mean it.

You need a value that is still there on the next call. There is exactly one
thing in this lesson that does that.

## How to check yourself

| Turn the knob to | The LED should be |
|---|---|
| the lowest position | blue |
| about 20 C | green |
| the highest position | red |

The serial line prints the colour in bits, so you can see what your function
returned even when the LED looks wrong. If the bits are right and the colour
is not, the problem is in the wiring, not in the code.

## What to submit

```
git add .
git commit -m "Lesson 06: colour thermometer"
git push
```

Your files go under `students/<your-github-username>/lesson-06/`.

Deadline: before the next session.

## Common problems

**The LED stays dark.** Check that the long pin goes to 3V3. If it is wired to
GND instead, nothing will light no matter what the code does.

**All three colours are on at once, and it looks white.** The pins are set to
`LOW` somewhere they should be `HIGH`. Remember the inverted logic.

**One colour never lights.** Swap that resistor for one you know works, then
check the pin. Red is on GPIO 5, green on 6, blue on 7.

**Red looks weaker than the others.** That is normal for this LED at these
resistor values. The datasheet shows why: red gives the least light per
milliamp of the three.

**The colour flickers at a boundary.** That is the star task, not a fault.
