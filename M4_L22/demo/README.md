# Lesson 22 demo - one picture, three chips

A WS2812B 8x8 panel driven by the same `src/main.cpp` on three boards.
Adafruit NeoPixel uses RMT on the ESP32-C3 and PIO on the RP2040 / RP2350.

## Boards and pins

| env | Board | Data pin |
|---|---|---|
| `c3` | ESP32-C3 Super Mini | GPIO 10 |
| `pico` | Raspberry Pi Pico (RP2040) | GP22 |
| `pico2` | Raspberry Pi Pico 2 (RP2350) | GP0 |

The pin is set in `platformio.ini` (`-D LED_DATA_PIN=...`), not in the code.

## Wiring

Power off while you wire.

| Panel wire | Goes to |
|---|---|
| red / black pair (5V, GND) | 4.25 V supply + / - |
| WHITE (input connector) | GND: supply - AND board GND |
| green (input connector, DIN) | the data pin from the table above |
| DOUT group (other end) | leave free |

- **WHITE is ground on this panel, not data.**
- Supply and board share ground, or nothing works.
- Never power the panel from the board 5V pin.
- At 4.25 V the DIN threshold is 0.7 x 4.25 = about 3.0 V, so 3.3 V logic is
  enough. At 5 V it would be 3.5 V and 3.3 V is not.

Hold the panel with the connector on the right: pixel 0 is the top-right
corner, and the rows run as a serpentine.

## Run

```
pio run -e c3 -t upload
pio device monitor -e c3
```

First upload to a Pico or Pico 2: hold BOOTSEL while plugging in USB.

## What you see

| Phase | Expect |
|---|---|
| ORDER | pixel 0 red (top right), 1 green, 7 blue (top left), 8 white under it |
| CHANNELS | whole panel red, green, blue |
| WALK | a dot runs left -> right on every row |
| PICTURE | the picture in `kPicture` |
| RAINBOW | a diagonal rainbow |

Serial prints the time spent inside `strip.show()` once per second.
Theory: 64 x 24 bits x 1.25 us = 1920 us.

## Things to try

1. Change `kPicture` to your own picture. Same code on every board.
2. Move your code to another station: only `-e <env>` changes.
3. Set `kSerpentine = false` and watch WALK and RAINBOW break.
4. Compare `show()` on the three boards. Which one returns before the data
   is on the wire, and why?
