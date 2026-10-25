# Debounce and implementation notes for Lesson 14 Homework

## Button Debouncing
The button is debounced using the custom `Button` class ported from K3, which utilizes a non-blocking `millis()` based approach. A state change is registered only after a continuous raw signal state of at least 30 ms (`DEBOUNCE_DELAY_MS = 30U`), effectively filtering out contact bounce. Holding the button doesn't trigger multiple switches because we only react on the edge event captured by `wasPressed()`.

## What went wrong and how I fixed it
1. **Division by Zero:** When auto-scaling the history bar chart, if all 14 averages in the RingBuffer were identical, a naive implementation would result in division by zero (`range = maxVal - minVal = 0`). I fixed this by checking if the range is near zero (`range > 0.001F`) and drawing static half-height bars if it is.
2. **First-render Labels:** Screen 1 shouldn't flicker, so labels are drawn once when the screen becomes active. I implemented static rendering for labels upon screen switch and resetting cached shown strings (`lastShownT[0] = '\0'`) to force immediate clean printing of the first values.

## PulseView Installation
- Downloaded and installed PulseView 0.4.2 on Windows.
- Connected the logic analyzer and ran Zadig to replace the driver with WinUSB on the appropriate interface.
- Confirmed PulseView scans and identifies the generic FX2 device under `fx2lafw`.
- Took a capture of the onboard LED GP25 blinking at 1 MHz, which works beautifully.
- Saved the screenshot as `pulseview.png`.
