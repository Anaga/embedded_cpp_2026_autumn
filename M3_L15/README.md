# Lesson 15 - Logic analyzer and bus diagnosis

The bus becomes visible. We clip a logic analyzer onto the Pico and read
UART, I2C and SPI from our own boards: first the frames we drew on slides in
sessions 12-14, then what the Adafruit libraries do on the wire that their
API hides. Then we diagnose broken firmware by evidence instead of guessing.

## What is where

| Folder | Contents |
|---|---|
| `slides.pptx` | the deck |
| `demo/` | all three buses and the potentiometer on one Pico - the capture target, and the homework starting point. Wiring and analyzer harness in `demo/README.md` |
| `lab/` | mystery firmware files and their bug reports for the diagnosis lab |
| `homework/` | room thermostat on top of the demo: task in `homework/README.md` |

## Before the session

- PulseView installed, analyzer detected (homework from session 14).
- Demo wired: AHT10, Nokia 5110 and potentiometer on the course pin map.

## Topics

- Why SPI0 sits on GP16-19, hardware CSn vs a GPIO as CS
- Logic analyzer basics: sample rate, duration, trigger, decoders
- UART 8N1 measured, baud rate read off the capture
- I2C transaction pieces, and the AHTX0 library polling the busy bit
- SPI mode 0, CS and D/C on the 5110, the PCD8544 library's dirty rectangle
- Diagnosis method: symptom, hypothesis, capture, compare, conclude
