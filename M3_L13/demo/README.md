# Lesson 13 demo - I2C scanner and the AHT10

Scans the I2C bus at start-up, then reads temperature and humidity every 2 s
through the Adafruit AHTX0 library and prints how long `getEvent()` blocked.
The on-board LED blinks without `delay()` as a heartbeat.

## Wiring

| AHT10 | Pico |
|---|---|
| VIN | 3V3 OUT (pin 36) - never VBUS |
| GND | GND |
| SDA | GP4 (pin 6) |
| SCL | GP5 (pin 7) |

## Run

Build, upload, open the serial monitor. Send `s` to scan again.

## Things to try

- Pull the SDA wire and send `s`. What does the scan say?
- Change `READ_PERIOD_MS` to 100 and watch the LED.
- Unplug the sensor while it runs. Why does everything stop?
  Look at `getStatus()` in the library source.
