# Lesson 10 - Demo

The program shown in class. Complete and working; read it, run it, change it.

## Files

```
demo/
  include/
    Colour.h        unchanged
    RgbLed.h        unchanged from lesson 09
    Button.h        unchanged from lesson 09
    RingBuffer.h    new: a class template, the whole of it in the header
  src/
    RgbLed.cpp
    Button.cpp
    main.cpp
```

`RingBuffer.h` has no matching `.cpp`. A template is a recipe, not finished
code: the compiler needs to see all of it wherever it is used, to write a
version for each type and size.

## Wiring

| Part | Connects to |
|---|---|
| Button A | GPIO 0 and GND |
| Button B | GPIO 1 and GND |
| Potentiometer, middle pin | GPIO 4 |
| Potentiometer, outer pins | 3V3 through 3.3k, and GND |
| RGB LED, longest pin | 3V3 |
| RGB LED, red / green / blue | GPIO 5 / 6 / 7, each through 100 ohm |

## What it shows

At startup, part 1 prints two templates at work: `largest()` with two
different types, and a small `RingBuffer` that keeps only the last four of
six values.

Then three things run at the same time in `loop()`. None of them uses
`delay()`, which is the only reason they can share one loop.

| Part | What happens | What to look for |
|---|---|---|
| 2. Traffic light | green, yellow, red, red + yellow | Press A while green: it waits for its minimum time, then changes. Press A during red: the request is remembered, and the next green ends after its minimum time |
| 3. Smoothing | a `[pot]` line once a second | Leave the knob alone: raw jumps, smooth barely moves, and the spread shows how noisy the last 16 readings were |
| 4. Interrupt | an `[irq]` line per press of B | 1 press, but often 2, 3 or more edges seen by the interrupt: the bounce, counted at full speed |

The traffic light is the state machine from the slides with different
states. Read `runTrafficLight()` and the four `handle...()` functions: each
state decides only when to leave, and `enterState()` is the one place that
changes the LED.

## Things to try

- Change `SMOOTH_SAMPLES` to 4, then to 64. What happens to the spread, and how
  fast does `smooth` follow the knob when you turn it?
- Press B very gently, so it only just clicks. Do the edge counts go up or
  down?
- Add a fifth state to the traffic light: blinking yellow at night, entered by
  holding A for three seconds. Which functions need to change?
