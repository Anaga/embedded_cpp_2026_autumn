# Homework 15 - room thermostat

Turn the session 15 demo into a small room thermostat. The demo already
reads the sensor, the setpoint and drives the display. You add the decision
and an output: the RGB LED shows whether the room needs heating, is fine, or
needs cooling.

This closes K4: three buses on one board, plus the RGB LED back in use on the
Pico.

## Start

1. Uncomment your own `src_dir` line in `homework/platformio.ini` (do not
   commit that file).
2. Copy `../demo/src/main.cpp` into `homework/<name>/src/`.
3. Set `LCD_CONTRAST` to your module's value. Build and upload: you should
   see the demo screen before you change anything.

## Hardware

Add the RGB LED from K3 (500RGB4E, common anode):

| LED | Pico | Physical pin |
|---|---|---|
| Common anode | 3V3 OUT | 36 |
| R (100 ohm) | GP10 | 14 |
| G (100 ohm) | GP11 | 15 |
| B (100 ohm) | GP12 | 16 |

Common anode means a channel is **on when its pin is LOW**.

## Behaviour

| Condition | State | LED |
|---|---|---|
| T < SP - 0.5 C | HEAT | red |
| SP - 0.5 C <= T <= SP + 0.5 C | OK | green |
| T > SP + 0.5 C | COOL | blue |
| sensor missing | - | off |

- The setpoint comes from the potentiometer, 15.0 .. 30.0 C, as in the demo.
- The width of the band (0.5 C) is a named constant, not a number in the
  middle of an `if`.
- **Sensor missing -> LED off.** No reading means no decision, so the
  thermostat must not keep heating on stale data (same fail-safe idea as the
  "no reply" case in session 12).
- Everything is visible on the 5110: T, RH, SP as in the demo, plus the state
  (`HEAT`, `OK` or `COOL`) on the free row 1. Redraw the state only when it
  changes, like the demo does for the values.
- The UART line gets the state at the end:

```
T=23.4 C RH=41.2 % SP=22.0 C COOL
```

## Rules

- RGB LED code: your choice. Port your `RgbLed` class from K3, write a
  smaller one, or use plain functions. `analogWrite()` and `digitalWrite()`
  both work for three fixed colours.
- No `delay()` in `loop()`. The demo's once-per-second tick stays.
- Fixed-width types, named constants, ASCII only (code, comments and the
  UART line).
- Commit only your own folder `homework/<name>/`.

## How to test

- **COOL:** turn the setpoint all the way down.
- **HEAT:** turn it all the way up.
- **OK:** set it to the current temperature.
- **Moving the room:** hold the sensor between your fingers or breathe on it,
  and watch the state change without touching the potentiometer.
- **Sensor missing:** unplug the AHT10 between two readings. The LED goes
  off; plug it back in and the thermostat recovers by itself.

## Think about (answer in a comment at the top of your main.cpp)

Set the setpoint so that the temperature sits right on the edge of the band,
for example SP = 22.0 C and T around 22.5 C. Watch the LED for a minute.
What do you see, and why? What would a real thermostat do about it?

Optional: change your code so it does not do that, and explain the change.

## Deadline

Before session 16.
