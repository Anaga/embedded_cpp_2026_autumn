# Lesson 11 demo - hardware timer

A hardware timer interrupt every 1 ms on the ESP32-C3. The interrupt measures
its own timing and sets a flag; `loop()` turns the flag into a heartbeat on the
green LED and prints the timing statistics once per second.

This is the code from the slides, put together in one working sketch.

## Wiring

Nothing new. The demo uses the green channel of the RGB LED you already have:

| Function  | Pin    | Note                                     |
|-----------|--------|------------------------------------------|
| Green LED | GPIO 6 | common anode: LOW = on, HIGH = off        |

## Build and run

1. Open the `demo` folder in VS Code (PlatformIO picks up `platformio.ini`).
2. Upload.
3. Open the serial monitor (115200 baud).

## What you should see

- The green LED blinks once per second.
- One line per second in the serial monitor:

```
n=<ticks> min=<us> us max=<us> us jitter=<us> us
```

- `n` is the number of interrupts in that second. The target is 1000.
- `min` and `max` are the shortest and longest time between two interrupts.
  The target is 1000 us.
- `jitter` is `max - min`.

The first line after a reset looks wrong. That is on purpose - it is the
question on the slide "Let the chip time itself".

## Where to find each idea in the code

| Slide                                     | In `src/main.cpp`                          |
|-------------------------------------------|--------------------------------------------|
| The arithmetic                            | `TIMER_DIV`, `ALARM_TICKS`                 |
| Four calls to a periodic interrupt        | end of `setup()`                           |
| Rules for an ISR                          | `onTick()`                                 |
| Flag pattern                              | `g_tickFlag`, `heartbeat()`                |
| Let the chip time itself                  | statistics in `onTick()`                   |
| Copy and reset inside a critical section  | `reportOncePerSecond()`                    |

## Try this

Predict first, then change the code and check.

1. Set `ALARM_TICKS` to `100U`. What does `n` become? How fast does the LED
   blink now, and why?
2. Set `TIMER_DIV` to `8U` and put `ALARM_TICKS` back to `1000U`. What is one
   tick now, and what is the interrupt rate?
3. Set `TIMER_NUM` to `2U`. What happens? Find the reason in the ESP32-C3
   Technical Reference Manual, chapter Timer Group (TIMG).

Put every value back before you start the lab.

## If you search the web for examples

Most ESP32 timer examples online are written for Arduino-ESP32 core 3.x:
`timerBegin(1000000)` with one argument, and `timerAlarm(...)`. PlatformIO
ships core 2.x, so those examples do not compile here. Translate them to the
four calls used in this demo instead of pasting them.
