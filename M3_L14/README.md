# Lesson 14 - SPI and the Nokia 5110

The third bus after UART (lesson 12) and I2C (lesson 13). You drive the
Nokia 5110 display (PCD8544 controller) byte by byte over SPI, wrap chip
select in a RAII class, and show the AHT10 reading on your own display.

## What this lesson covers

- SPI next to I2C: four wires, push-pull, one CS per device, full duplex,
  no ACK - and so no error when something is wrong
- Clock modes (CPOL / CPHA); the PCD8544 is mode 0, MSB first, up to 4 MHz
- The PCD8544: 84 x 48 pixels in 6 banks of 84 bytes, commands vs data
  (D/C pin), reset, init sequence, contrast (Vop)
- RAII: a `ChipSelect` guard that releases CS on every exit
- Lab: your own driver - checkerboard, text from a font table, the AHT10 on
  screen
- The same screen through the Adafruit libraries, and what they hide

## In this folder

| Path | What it is |
|---|---|
| `slides/lesson14.pptx` | the slides |
| `lab/font5x7.h` | the 5x7 font table for lab 2 and the homework - copy it into your own folder |
| `demo/` | the library version: AHT10 + Nokia 5110 with Adafruit libraries. See `demo/README.md` |
| `homework/README.md` | the homework: a climate dashboard on your own driver, plus PulseView |
| `homework/platformio.ini` | uncomment your own `src_dir` line, do not commit it |

## Wiring

The course pin map from lesson 12. The AHT10 stays on GP4 / GP5; both
modules can be wired at the same time.

| Nokia 5110 | Pico |
|---|---|
| RST / CE / DC | GP21 / GP17 / GP20 |
| DIN / CLK | GP19 / GP18 |
| VCC / GND | 3V3 OUT (pin 36) / GND - never VBUS |

## Before lesson 15

Install PulseView and check that it sees your logic analyzer (part 2 of the
homework). Keep your UART, I2C and SPI code from lessons 12-14 working: it is
what we capture next time.
