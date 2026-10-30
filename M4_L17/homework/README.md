# Homework 17 - night light and orientation

Extend the lesson 17 project. The demo already reads both sensors every
500 ms. You add two decisions based on those readings.

## Start

1. Copy the whole folder `M4_L17/lesson16` to `M4_L17/homework/<name>/lesson16`
   (for example `homework/sven/lesson16`).
2. Open **that copy** in VS Code (File -> Open Folder), Set Up STM32Cube
   project, build and upload. You should see the demo output before you
   change anything.
3. Change only `Core/Src/app.c`. Do not edit `main.c` and do not press
   Generate Code.

## Task 1 - night light

Today the LED just toggles with every sample. Make it depend on the light:

| Light | LED |
|---|---|
| below 10 lx | on (steady) |
| 10 .. 300 lx | toggles every sample, as now |
| above 300 lx | off |
| BH1750 missing | toggles every sample, as now |

- The two limits are named constants, not numbers in the middle of an `if`.
- `bh_read_lux10()` gives tenths of a lux: 10 lx is `100`.
- The LED is **active low**: `GPIO_PIN_RESET` switches it on. A small
  `led_set(bool on)` function keeps this detail in one place.
- Test with your hand over the sensor (dark) and the phone torch (bright).

## Task 2 - orientation

Use the accelerometer to tell how the board lies. Lying flat, the Z axis
carries about +1000 mg.

| Z acceleration | Orientation |
|---|---|
| az >= 700 mg | `FLAT` |
| az <= -700 mg | `UPSIDE DOWN` |
| otherwise | `ON EDGE` |

Add the orientation at the end of each line:

```
   12500 ms  lux 213.3  acc 12 -40 1003 mg  temp 26.4 C  FLAT
```

- The limit (700 mg) is a named constant.
- Write the decision as its own function that takes the `MpuSample` and
  returns an `enum`. A second small function turns the `enum` into text.
- MPU-6050 missing -> no orientation, the line stays as in the demo.

## Extra

When the orientation changes, print one extra line, only once per change:

```
-> now UPSIDE DOWN
```

Hint: remember the previous orientation in a `static` variable. The very
first sample is not a change.

## Hand-in

Commit `homework/<name>/lesson16` (the `build/` folder is ignored by git) and
push. In the commit message: what works and what does not.
