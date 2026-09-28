# Lesson 09 - Demo

The program shown in class. Complete and working; read it, run it, change it.

## Files

```
demo/
  include/
    Colour.h        the struct, unchanged from lesson 08
    RgbLed.h        the LED class, now with const & and an overload
    Button.h        the new class: one object per button
  src/
    RgbLed.cpp
    Button.cpp
    main.cpp
```

## Wiring

| Part | Connects to |
|---|---|
| Button A, player 1 | GPIO 0 and GND |
| Button B, player 2 | GPIO 1 and GND |
| RGB LED, longest pin | 3V3 |
| RGB LED, red / green / blue | GPIO 5 / 6 / 7, each through 100 ohm |

## Build and upload

1. Open this `demo` folder in VS Code with PlatformIO.
2. Build, upload, open the serial monitor at 115200.

## What it shows

| Part | What to look for |
|---|---|
| 1. References | `alias = 7` changes `count`; the three `addOne` functions give 5, 6, 6 |
| 2. Overloading | two calls to `setColour`, one with a `Colour`, one with three numbers |
| 3. Two buttons | a line per counted press, and the LED turns red for A, blue for B |

## The button experiment

The constant `BUTTON_MODE` near the top of `src/main.cpp` picks how the
buttons are read. Run both, in this order.

**1. `MODE_SHARED_STATIC`** - one function with `static` variables, called for
both pins, exactly as you would write it after lesson 07.

- Press A alone: counted.
- Press B alone: counted.
- Now hold A down and press B while A is still held: **B is not counted.**

Single presses work, which is what makes this bug nasty: it only shows when
both buttons are in use at once. In a two-player game that is all the time.

**2. `MODE_OBJECTS`** - two `Button` objects.

- Hold A, press B: B is counted.

The logic inside `Button::wasPressed()` is the same as in the broken function.
The only difference is where the memory lives: in one set of statics for the
whole program, or in one set of fields per object.

## Things to try

- In `MODE_SHARED_STATIC`, press A and B quickly one after the other, within a
  few hundredths of a second. What happens to the second press?
- Add a third `Button` on another free pin. How many lines of code did it
  take?
- In `RgbLed`, remove `const` from `setColour(const Colour &c)` and build.
  Which call in `main.cpp` stops compiling, and why?
