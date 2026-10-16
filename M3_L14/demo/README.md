# Lesson 14 demo - the AHT10 on the Nokia 5110, through libraries

Reads temperature and humidity every 2 s with the Adafruit AHTX0 library and
shows them on the Nokia 5110 with Adafruit PCD8544 + Adafruit GFX. Serial
prints the same values and how long `display()` took. The on-board LED
blinks without `delay()` as a heartbeat.

This is the library version, for comparison with the driver you write in
class. Your own driver is the lab and is not in this demo.

## Wiring

| Nokia 5110 | Pico | Physical pin |
|---|---|---|
| RST | GP21 | 27 |
| CE (SCE, CS) | GP17 | 22 |
| DC | GP20 | 26 |
| DIN | GP19 | 25 |
| CLK | GP18 | 24 |
| VCC | 3V3 OUT - never VBUS | 36 |
| BL / LIGHT | see the note from class | - |
| GND | GND | any, e.g. 23 |

| AHT10 | Pico | Physical pin |
|---|---|---|
| VIN | 3V3 OUT - never VBUS | 36 |
| GND | GND | any |
| SDA | GP4 | 6 |
| SCL | GP5 | 7 |

Module labels and pin order differ between 5110 boards. Go by the label
printed on the board, not by the position.

## Run

Build, upload, open the serial monitor. PlatformIO fetches the libraries on
the first build (Adafruit BusIO comes in as a dependency).

At start-up the display shows the Adafruit splash image for a moment: it is
the initial content of the library's framebuffer.

If the screen is blank or all dark, change `LCD_CONTRAST` (try 0x28 .. 0x48).

## Things to try

- Note the `display() took` value on Serial. Work out how long 504 bytes take
  at 4 MHz. Does it match?
- Remove `lcd.clearDisplay()` and draw only the status line each time. What
  happens to the time, and what happens to the old text on screen?
- Find where the 504-byte buffer is declared in `Adafruit_PCD8544.cpp`. How
  much RAM does it cost, and when would that matter?
- Compare `begin()` in the library with the init sequence from the slides.
  Which command does the library leave out?
- Unplug the AHT10 while it runs. What happens? (Same question as lesson 13.)
