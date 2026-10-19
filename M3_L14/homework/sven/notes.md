# Lesson 14 notes

- Button: reused Button class on GP14/GND with INPUT_PULLUP. A falling edge
  counts after 30 ms without pin changes, using millis(). Holding does not repeat.
- Serial: missing float output fixed with dtostrf() and Serial.printf("%s", ...).
- Chart: rounding caused uneven bar widths; fixed to 2 pixels with 1-pixel gaps.
  The left scale replaces the README's top labels and 5-pixel-wide bars.
- Checks: Pico 2 build, ASCII and history logic checks passed. Final hardware
  checks remain pending.
- PulseView: postponed. Installation and capture unverified; no screenshot yet.
