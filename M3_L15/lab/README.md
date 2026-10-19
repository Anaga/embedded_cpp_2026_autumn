# Lab: mystery firmware

Four firmware files, each with a bug report from a user. Your job is to find
out what is wrong - or to show that nothing is wrong on the bus - using the
logic analyzer.

Not every file is broken. The firmware will not tell you which.

## Setup

- Work in pairs at one of the two analyzer stations.
- Same wiring and the same analyzer harness as the demo (see
  `../demo/README.md`, "Logic analyzer harness").
- **Before the first file:** flash the demo on the station board and save a
  known-good capture of all three buses as a `.sr` file. You will compare
  against it.

## Flashing a mystery file

1. Unplug USB. Hold BOOTSEL, plug USB back in, release BOOTSEL.
2. Copy `mystery-X.uf2` onto the `RPI-RP2` drive. The board reboots and runs
   it.
3. When you are done with the lab: `pio run -t upload` in `../demo/` puts the
   demo back.

Order: one pair starts with A, the other with C, then continue in order
(A B C D or C D A B). About 8 minutes per file.

## Bug reports

**mystery-A**
> The display stays blank, or shows random dots. The USB monitor shows normal
> readings every second.

**mystery-B**
> The thermometer on the wall says 21.0 C, the device says 23.0 C. Something
> on the bus must be wrong.

**mystery-C**
> The display says SENSOR MISSING, but the sensor is plugged in and the wires
> are right. We already tried another sensor.

**mystery-D**
> Our data logger on UART0 (GP0, 115200 8N1) receives garbage. On the USB
> monitor everything looks fine.

## Report

One table per file in your notes. The evidence column is a screenshot of the
capture with the relevant samples visible - not a description.

| | |
|---|---|
| File | mystery-X |
| Symptom (what you saw yourself) | |
| Hypothesis (which bus, which part) | |
| Capture settings (channels, rate, trigger) | |
| Evidence (screenshot, what to look at) | |
| Conclusion | |

Not accepted as proof: a guess from the symptom, a hex editor, or "it works
now".
