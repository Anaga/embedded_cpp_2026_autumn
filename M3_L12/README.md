# Lesson 12 - UART on the Pico

Session 12, 2026-10-07. Board: Raspberry Pi Pico (RP2040), Arduino-Pico core.

First session on the Pico. Solder the headers, compare the three buses of
K4, then make two boards talk over UART - first as a chat, then as a small
protocol: a potentiometer on one board sets the blink rate on the other.

## In this folder

| Folder      | What is in it                                                    |
|-------------|------------------------------------------------------------------|
| `slides/`   | The deck (`lesson12-uart-on-the-pico.pptx`) and its generator    |
| `demo/`     | The USB <-> UART bridge from class - start with `demo/README.md` |
| `homework/` | The task in `homework/README.md`, plus `homework/platformio.ini`  |

## Topics

- Soldering the 2x20 headers with the breadboard as a jig; checking by eye
  and by continuity before power
- UART vs I2C vs SPI in one table, and the Pico pin map for the rest of K4
- UART: no clock wire, the 8N1 frame, 115200 baud in numbers
- `Serial` (USB) vs `Serial1` (UART0, GP0 / GP1) vs `Serial2` (UART1, GP8 / GP9)
- Wiring two boards: TX to RX crossed, shared GND, never 3V3 or VBUS
- Who talks first: stream, poll or command - the protocol decides the roles
- Master / slave polling with a timeout and a fail-safe LED
- Reading a line byte by byte into a `char` array

## Pins used tonight

| Function                 | GPIO       | Pin     |
|--------------------------|------------|---------|
| UART0 TX / RX (`Serial1`) | GP0 / GP1  | 1 / 2   |
| GND for the UART link    | -          | 3       |
| Potentiometer (ADC0)     | GP26       | 31      |
| AGND / 3V3 OUT for the pot | -        | 33 / 36 |
| On-board LED             | GP25       | -       |

## Next session

I2C with the OPT4001 light sensor on GP4 / GP5. Keep your Pico soldered and
bring it with the potentiometer still wired.
