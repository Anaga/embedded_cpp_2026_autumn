# Final project: SMART MOTORBIKE

Author: Viktorija Trubaciute
Date: 2026-11-16
Status: draft -> approved (instructor fills this in on Monday)

Fill every section. One or two sentences is enough where nothing more is
needed; "TBD" is allowed only in section 9. Keep it ASCII, keep it in English.

---

## 1. One-sentence summary

A device mounted on the bike that regularly checks:
- Battery voltage
- Temperature and humidity
The device then sends this data to an indoor gateway.

The device can also be woken up by an accelerometer and in that case sends an alarm instantly.

The data is displayed on a web page with history.

## 2. Why this project

It solves the problem of storing the bike over the winter when there is a need to ensure its condition, and also to alert if the bike tips over or gets moved.

## 3. Platform

Pick one and say why in one line:

- ESP32-C3 Super Mini
  Mounted under the seat to read battery voltage and sensor data, sleeps most of the time
- ESP32-S3-DevKitC-1U (N16R8)
  Sits at home (or classroom) near the window and receives radio messages from ESP32-C3 Super Mini. Always on and powered, connected to home (or classroom) internet.
  Contains more logic, uploads data to a web page.

## 4. Hardware

List every part. Mark what you already have (course kit or your own) and
what still has to be bought or borrowed. Nothing on this list may arrive
after Friday 2026-11-20.

| Part                | Interface                   | Have it? | Source                   |
| ------------------- | --------------------------- | -------- | ------------------------ |
| AHT10               | I2C                         | yes      | course kit / own / order |
| MPU-6050            | I2C + GPIO interrupt        | yes      | course kit               |
| RA-02 LoRa          | SPI + GPIO + GPIO interrupt |          |                          |
| Mean Well regulator |                             | yes      | own                      |
| Capacitors          |                             | no       |                          |
| Cables              |                             | no       |                          |
| Boxes / enclosures  |                             | no       |                          |

## 5. Pin budget

Every pin you use, in one table. Check it against the pinout from the course
conventions: strapping pins (C3: 2, 8, 9), boot buttons, on-board LEDs, 5 V
tolerance. Two things on one pin = not approved.

### ESP32-C3 Super Mini

| GPIO          | Function                              | Notes                     |
| ------------- | ------------------------------------- | ------------------------- |
| 0             | test button                           | to wake up the super mini |
| 1             | MPU-6050                              | to wake up the super mini |
| 3             | battery voltage                       | ADC                       |
| 4 / 5 / 6 / 7 | RA-02 SCK / MISO / MOSI / chip select | SPI                       |
| 8 / 9         | SDA /SCL for AHT10 and MPU-6050       | I2C                       |
| 10            | RA-02 reset                           |                           |

### SP32-S3-DevKitC-1U

| GPIO              | Function                              | Notes                       |
| ----------------- | ------------------------------------- | --------------------------- |
| 9                 | RA-02 reset                           | GPIO output                 |
| 10 / 11 / 12 / 13 | RA-02 chip select / MOSI / SCK / MISO | SPI                         |
| 14                | RA-02 DIO0                            | GPIO interrupt              |
| 38                | On-board RGB LED                      | GPIO, one-wire LED protocol |
| 43 / 44           | Serial to the PC                      | UART                        |


## 6. Minimum and stretch

Minimum (must work on demo day):
- A device by itself and RD6006 to simulate the battery
- Live web page with temperature, humidity, and battery voltage, and some history
- Instant alert on web page on movement

Stretch (nice to have):
- The device is on the actual bike and connected to actual battery, standing outside the classroom (depends)
- Readings from the ECU (the other project - no details here for now)

## 9. Open questions and risks

What you do not know yet, what might not work, what you need from the
instructor (a part, a library, a decision). "TBD" lives here and nowhere
else.

- ...

## 10. Plan for the three sessions

| Session | Date           | Goal at the end of the evening                                           |
| ------- | -------------- | ------------------------------------------------------------------------ |
| 23      | Fri 2026-11-20 | Hardware wired, every peripheral talks (one test program per peripheral) |
| 24      | Mon 2026-11-23 | Minimum behaviour works end to end                                       |
| 25      | Fri 2026-11-27 | Demo: 20 minutes, live data, test of the movement alarm                  |
