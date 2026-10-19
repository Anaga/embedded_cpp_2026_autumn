# Lesson 15 demo - three buses on one Pico

One board, all three buses at once, plus the potentiometer:

| Bus | What | Library |
|---|---|---|
| I2C (GP4 / GP5) | AHT10 temperature and humidity | Adafruit AHTX0 |
| SPI (GP16 .. GP21) | Nokia 5110 display | Adafruit PCD8544 + GFX |
| UART0 (GP0, `Serial1`) | one status line per second | - |
| ADC0 (GP26) | potentiometer -> setpoint 15.0 .. 30.0 C | - |

Once per second the demo reads the setpoint and the sensor, redraws only the
values that changed, and prints the same line on USB and on UART0:

```
T=23.4 C RH=41.2 % SP=22.0 C
T=--.- C RH=--.- % SP=22.0 C sensor missing
```

In class this is what you clip the logic analyzer onto. At home it is the
starting point for the room thermostat (see `../homework/README.md`).

## Wiring

Everything below is the course pin map from session 12. Nothing is new
except that it is all connected at the same time.

| Part | Pin | Pico GPIO | Physical pin |
|---|---|---|---|
| AHT10 | SDA | GP4 | 6 |
| AHT10 | SCL | GP5 | 7 |
| AHT10 | VIN / GND | 3V3 OUT / GND | 36 / any GND |
| Nokia 5110 | CLK (SCK) | GP18 | 24 |
| Nokia 5110 | DIN (MOSI) | GP19 | 25 |
| Nokia 5110 | CE (CS) | GP17 | 22 |
| Nokia 5110 | DC | GP20 | 26 |
| Nokia 5110 | RST | GP21 | 27 |
| Nokia 5110 | VCC / GND | 3V3 OUT / GND | 36 / any GND |
| Potentiometer | wiper | GP26 (ADC0) | 31 |
| Potentiometer | ends | AGND / 3V3 OUT | 33 / 36 |
| UART0 TX | (analyzer only) | GP0 | 1 |

Power the AHT10 and the 5110 from 3V3 OUT, never from VBUS.

## Logic analyzer harness

| Channel | Signal | GPIO | Pin |
|---|---|---|---|
| D0 | UART0 TX | GP0 | 1 |
| D1 | SDA | GP4 | 6 |
| D2 | SCL | GP5 | 7 |
| D3 | SCK | GP18 | 24 |
| D4 | MOSI | GP19 | 25 |
| D5 | CS | GP17 | 22 |
| D6 | D/C | GP20 | 26 |
| D7 | RST | GP21 | 27 |
| GND | GND | - | 3, 23 or 38 |

Clip on with USB unplugged. Ground first.

## How to run

1. Set `LCD_CONTRAST` in `src/main.cpp` to the Vop value you found for your
   module in session 14.
2. `pio run -t upload`, then `pio device monitor`.
3. Turn the potentiometer: SP moves in 0.5 C steps.
4. Breathe on the sensor: T and RH rise within a few seconds.

## Things to try

- **UART on the analyzer.** D0 only, 2 MHz, trigger on the falling edge.
  Measure the narrowest pulse and compute the baud rate. The first byte of
  every line is `T` (0x54).
- **The library's polling.** D1 / D2, 2 MHz, about 200 ms. Find the trigger
  write `AC 33 00`, the status reads every 10 ms while the busy bit is set,
  and the final 6-byte read. How long did the conversion take on your
  sensor?
- **The dirty rectangle.** D3 .. D6, 24 MHz, trigger on the falling edge of
  CS. Capture a second where nothing changes, then one where only SP changes.
  Count the banks, the data bytes and the commands. Which command is always
  sent, even when nothing changed?
- **Unplug the sensor** between two readings: the screen shows
  `SENSOR MISSING` and the demo retries every second. On the analyzer the
  retry is an address byte with a NACK. Plug it back in and the readings
  return.
- **Unplug the sensor during a reading** (hard to time, but it happens): the
  demo freezes. `getEvent()` loops while the busy bit is set, and a failed
  read returns 0xFF - busy set forever. Your own `Aht10` class from session
  13 does not have this problem.
- **Analyzer cannot keep up at 24 MHz?** Set `SPI_CLOCK_CAP_HZ` to
  `1000000UL` and sample at 8 MHz. The display does not care.

## Notes

- `ClockCappedSpi` is a small subclass of the Arduino-Pico SPI class. The
  library calls `beginTransaction()` for every transfer, so overriding that
  one function limits the clock without changing the library.
- `lcd.display()` runs every second on purpose. When nothing changed it sends
  a single command byte - look for it.
- The library's `begin()` pushes its splash image once before the screen is
  cleared. Reset the board with the capture armed to see the whole init.
