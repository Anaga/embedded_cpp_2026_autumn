# Lesson 19 - Sleep, time and ESP-NOW (ESP32-C3)

A board on a battery has to sleep. Sleep costs us two things we took for
granted: the clock and the network connection. NTP gets the clock back,
ESP-NOW makes the network cheap again.

| Folder | What |
|---|---|
| `slides/` | the deck (`lesson19.pptx`) and its generator |
| `lab1-sleep/` | deep sleep, timer wake, button wake, RTC memory |
| `lab2-ntp/` | NTP once on a cold start, exact time on every wake |
| `lab3-node/` | a sleeping AHT10 node that reports over ESP-NOW |
| `lab3-gateway/` | the receiving side (runs on the instructor's board): ESP-NOW + WiFi + web page |
| `homework/` | the task: a clock that sleeps (`homework/README.md`) |

## Before you start

- Wiring from session 18 stays: AHT10 SDA -> GPIO 8, SCL -> GPIO 9,
  VIN -> 3V3. RGB LED on GPIO 5 / 6 / 7. Button A on GPIO 0 (K3 wiring).
- `lab2-ntp`: copy `include/secrets.example.h` to `include/secrets.h` and
  fill in the class WiFi (on the whiteboard). Never commit `secrets.h`.
- `lab3-node`: fill in `include/class_net.h` from the whiteboard.

## Uploading to a sleeping board

The C3's USB is inside the chip and powers off in deep sleep: no port, no
upload. Press RESET or button A - the board stays awake 10 s - and click
Upload in that window. Last resort: hold BOOT, press RESET, release BOOT.

Details, expected output and troubleshooting: the README in each lab folder.
