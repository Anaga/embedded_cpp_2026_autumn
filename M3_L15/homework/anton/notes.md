# Lesson 15 homework notes - anton

## RGB LED implementation

Used direct `digitalWrite()` calls for the three channels of the 500RGB4E
common anode LED (100 ohm resistors on each channel). Common anode means
a channel is ON when its pin is LOW.

Pins: GP10 (R), GP11 (G), GP12 (B). All initialized to HIGH (off) in setup().

## Thermostat logic

State transitions use a fixed band of 0.5 C (5 deci-degrees) around the
setpoint:
- T < SP - 0.5 C -> HEAT (red)
- SP - 0.5 C <= T <= SP + 0.5 C -> OK (green)
- T > SP + 0.5 C -> COOL (blue)
- Sensor missing -> LED off, state OFF

The band half-width is a named constant `BAND_HALF_WIDTH_DECI = 5`.

## Display updates

- Values (T, RH, SP) redrawn only when their text changes (same pattern as demo)
- State (HEAT/OK/COOL) shown on row 1, redrawn only when state changes
- `lcd.display()` called every tick; sends minimal data when nothing changed

## UART output format

`T=23.4 C RH=41.2 % SP=22.0 C COOL` (state appended at the end)
When sensor missing: `T=--.- C RH=--.- % SP=22.0 C` (no state)

Sent on both USB CDC (Serial) and UART0/GP0 (Serial1) for the logic analyzer.

## Think about answer (hysteresis)

When the temperature sits exactly on the band edge (e.g., SP = 22.0 C,
T ≈ 22.5 C), sensor noise and quantization cause the state to flip
between COOL and OK on consecutive readings. This "chattering" would
wear out a real heater/cooler relay.

A real thermostat uses **hysteresis**: different turn-on and turn-off
thresholds. For example, start cooling at SP + 0.5 C, stop cooling at
SP + 0.3 C. The 0.2 C dead band prevents rapid switching.

Optional software fix: track the previous state and require the
temperature to cross a wider threshold to change state. For instance,
if currently COOL, require T <= SP + 0.3 C to switch to OK; if currently
OK, require T >= SP + 0.5 C to switch to COOL. This implements the same
hysteresis logic in software.

## Build verification

- Builds without warnings from own files
- COOL: turn setpoint all the way down -> blue LED
- HEAT: turn setpoint all the way up -> red LED
- OK: set setpoint to current temperature -> green LED
- Moving sensor: breathe on it, watch state change without touching potentiometer
- Sensor missing: unplug AHT10 between readings -> LED off; plug back in -> recovers automatically
- ASCII check: no non-ASCII characters in code or serial output