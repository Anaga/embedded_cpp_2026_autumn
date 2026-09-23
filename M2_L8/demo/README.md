# Lesson 08 - Demo

The program shown in class. Complete and working; read it, run it, change it.

## Files

This is the first demo split over several files, the way a real project is:

```
demo/
  include/
    Colour.h        the struct
    RgbLed.h        the class: what it offers
  src/
    RgbLed.cpp      the class: how it works inside
    main.cpp        the program that uses it
```

`main.cpp` includes the two headers and never looks inside `RgbLed.cpp`.

## Wiring

| LED pin | Connects to | Through |
|---|---|---|
| Longest pin (common anode) | 3V3 | direct |
| Red | GPIO 5 | 100 ohm |
| Green | GPIO 6 | 100 ohm |
| Blue | GPIO 7 | 100 ohm |

## Build and upload

1. Open this `demo` folder in VS Code with PlatformIO.
2. Build, upload, open the serial monitor at 115200.

## What it shows

| Part | What to look for |
|---|---|
| 1. A struct is a value | after `b = a` and `b.red = 99`, `a.red` is still 10 |
| 2. One object, many colours | nine named colours, each one call to `led.setColour()` |
| 3. PWM | red fades smoothly from dark to full and back |
| Then | the palette repeats forever |

Green and white look brighter than red at the same number. That is the LED
and your eye, not the code.

## The experiment that matters

Open `src/RgbLed.cpp` and change one line:

```cpp
static const bool USE_PWM = false;
```

Upload. The same class now drives the LED with plain `digitalWrite`:

- orange turns into plain red, and dim white goes dark
- the fade in part 3 becomes a single hard switch halfway up

Now look at `main.cpp`. Not one character of it changed. How the LED is driven
is the class's own business, and that is exactly what a class is for.

Set it back to `true` when you are done.

## Things to try

- Add a colour of your own to `PALETTE`. Nothing else needs to change.
- Change `PWM_FREQ_HZ` to 50 and look at the LED from the corner of your eye.
  What do you see?
- Remove the `off()` call at the end of `begin()` and press the reset button.
  What happens in the first moment?
