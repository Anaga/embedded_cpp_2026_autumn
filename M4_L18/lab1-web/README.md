# Lab 1 - AHT10 + WiFi + a local web server

The ESP32-C3 joins the class WiFi, reads the AHT10 every 2 s and serves a
page with the temperature, the humidity and two LED buttons. `/status`
returns the same data as JSON; `POST /led` switches the LED.

Builds for the ESP32-C3 Super Mini (`c3`) and the Lolin S2 Mini (`s2`).

## Wiring (ESP32-C3)

| Signal | Pin | Notes |
|---|---|---|
| AHT10 VIN | 3V3 | never 5V |
| AHT10 GND | GND | |
| AHT10 SDA | GPIO 8 | strapping pin, on-board blue LED |
| AHT10 SCL | GPIO 9 | strapping pin, BOOT button |
| RGB LED R / G / B | GPIO 5 / 6 / 7 | common anode to 3V3, as in K3 |

GPIO 8 and 9 are strapping pins. The AHT10 pull-ups keep them HIGH at
reset, which is the normal boot. The blue on-board LED sits on GPIO 8 (SDA)
and flickers with every I2C transfer - an I2C activity light. GPIO 9 is the
BOOT button: pressing it while the board runs spoils one reading.

The "LED" in this lab is the green channel of the RGB LED.

## Run

1. Copy `include/secrets.example.h` to `include/secrets.h` and fill in the
   class WiFi. `secrets.h` is in `.gitignore` - never commit it.
2. Build, upload, watch:

```
pio run -e c3 -t upload
pio device monitor -e c3
```

Expected output:

```
Lesson 18 - Lab 1 - Arduino core 2.0.17, IDF v4.4.7-dirty
Board ESP32-C3 Super Mini  MAC 28:37:2F:75:94:94  id esp-9494
Scanning...
 0  class-router                      -55 dBm  ch  6
WiFi: connecting to "class-router"
AHT10: found
WiFi: associated, waiting for an IP address
WiFi: got IP 192.168.3.43 -> http://192.168.3.43/
mDNS: http://esp-9494.local/
esp-9494 | WiFi 192.168.3.43 -55 dBm | 23.5 C  53.6 % | LED off
```

## Try it

| What | How |
|---|---|
| The page | `http://esp-xxxx.local/` or `http://<ip>/` |
| JSON with headers | `curl -i http://esp-xxxx.local/status` |
| LED from a form field | `curl -X POST -d 'on=1' http://esp-xxxx.local/led` |
| LED from JSON | `curl -X POST -H 'Content-Type: application/json' -d '{"on":false}' http://esp-xxxx.local/led` |
| Broken JSON | `curl -i -X POST -H 'Content-Type: application/json' -d '{"on":"yes"}' http://esp-xxxx.local/led` |

Windows PowerShell: use `curl.exe` and double quotes, with the inner quotes
of the JSON escaped: `-d "{\"on\":false}"`.

Things to watch:

- Breathe on the AHT10 and watch the page update without a reload.
- Press BOOT while the board runs: "AHT10: missing", then "found" again.
- Put a wrong password into `secrets.h`: which disconnect reason appears?

## The page does not open

- Laptop and board on the same WiFi?
- macOS: System Settings -> Privacy & Security -> Local Network: allow the
  browser and the terminal (VS Code too, if you use its terminal).
- `.local` does not resolve? Use the IP from the monitor.

## No router, or the board will not connect

Set `USE_SOFTAP = true` in `secrets.h`. The board becomes an access point:
join the WiFi network `esp-xxxx` with `AP_PASS` and open
`http://192.168.4.1/`.

Some Super Mini boards connect only with reduced TX power. `board.h` sets
`WIFI_POWER_8_5dBm` for the C3; if a board still fails next to the router,
try `LIMIT_TX_POWER = false`.
