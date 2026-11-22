# Lesson 24 demo - from parts to a device

One device, written twice:

| Environment | Folder        | What it shows                                             |
|-------------|---------------|-----------------------------------------------------------|
| `before`    | `src/before/` | every part glued into `loop()` as it was tested alone     |
| `after`     | `src/after/`  | one class per part, `loop()` is a list, WiFi + bot in a task |

Both do the same things:

- AHT10 reading every 2 s
- `after` also reads the chip's own temperature sensor every 2 s
  (`temperatureRead()`, no wiring - the die runs 10-20 C above the room)
- button A toggles the green LED
- blue LED = heartbeat: 4 blinks a second while `loop()` is alive
- Telegram: `/status`, `/led on`, `/led off` (`after` also has `/led toggle`)
- WiFi networks come from NVS - the strongest saved network wins
- every 5 s on Serial: `loop: ... passes/s, longest pass ... ms`

## Wiring

Nothing new. Same as sessions 18-23, plus button A from K3:

| Part      | ESP32-C3 pin                                     |
|-----------|--------------------------------------------------|
| AHT10     | SDA GPIO 8, SCL GPIO 9, VIN 3V3, GND             |
| RGB LED   | R / G / B on GPIO 5 / 6 / 7, common anode to 3V3 |
| Button A  | GPIO 0 to GND (INPUT_PULLUP)                     |

GPIO 8 and GPIO 9 are strapping pins: do not press BOOT (GPIO 9) during a
reading, and nothing may hold SDA / SCL low at reset.

## 1. Bot token

Copy `include/secrets.example.h` to `include/secrets.h` and put in your bot
token from lesson 23. WiFi passwords do NOT go there any more.

## 2. Flash `after` and save your networks

```
pio run -e after -t upload
pio device monitor
```

In the monitor (you see what you type, Enter sends the line):

```
wifi add "Home Net" mypassword123
wifi add "ClassRouter" classpassword
wifi list
reboot
```

| Command                        | What it does                                   |
|--------------------------------|------------------------------------------------|
| `help`                         | the command list                               |
| `status`                       | board id, sensors, LED, WiFi, longest loop pass |
| `wifi list`                    | saved networks, `<- connected` marks the active one |
| `wifi add "<ssid>" <password>` | save a network (quotes for names with spaces)  |
| `wifi add "<ssid>"`            | an open network, no password                   |
| `wifi del <n>`                 | delete network number n                        |
| `wifi clear`                   | delete all                                     |
| `reboot`                       | restart - new networks are used after this     |

Up to 5 networks. At boot the board scans and connects to the strongest one
it knows. If that network goes away, it scans again and takes the next one.

The networks are stored in NVS, a small key-value store in flash. It
survives power off and re-flashing - an upload does not erase it, only
"Erase Flash" does. It is not encrypted: whoever has the board can read the
passwords out. Passwords are never printed, `wifi list` shows only names.

## 3. Flash `before` and watch it struggle

```
pio run -e before -t upload
pio device monitor
```

It finds the networks you saved with `after` - same NVS.

- the blue heartbeat stutters
- press button A quickly a few times: some presses are lost
- `/status` answers late
- switch the router off: everything stops, the button too
- `loop: 0.4 passes/s, longest pass 2600 ms` (your numbers will differ)

## 4. Flash `after` again and compare

- the heartbeat is smooth, every press counts
- `/status` answers within about a second
- router off: heartbeat and button keep going, the net task searches
- `loop: about 80000 passes/s, longest pass 4-8 ms` (measured with WiFi and
  the bot running; one pass of a few hundred ms while WiFi connects is normal)
- AHT10 unplugged: `AHT10: missing`, plug it back - `AHT10: found`

## How `after` is built

| File               | Part                                   | Runs in  |
|--------------------|----------------------------------------|----------|
| `main.cpp`         | setup() and the list in loop(), LED glue | loop()   |
| `rgb_led.h`        | three channels, common anode           | loop()   |
| `heartbeat.h`      | blue blink every 125 ms                | loop()   |
| `button.h`         | 30 ms debounce, one event per press    | loop()   |
| `aht10_sensor.h`   | reading every 2 s, missing / found     | loop()   |
| `chip_temp.h`      | chip temperature every 2 s             | loop()   |
| `console.h / .cpp` | serial commands, one line at a time    | loop()   |
| `shared.h / .cpp`  | LED command queue, status snapshot     | both     |
| `net.h / .cpp`     | WiFiMulti + Telegram bot               | net task |
| `../common/loop_stats.h` | passes per second, longest pass  | loop()   |
| `../common/wifi_store.*` | networks in NVS (Preferences)    | both     |

Every part has `begin()` (once, from `setup()`) and `update(now)` (every
pass, returns at once). The net task owns WiFi and the bot; `loop()` owns
the pins. The bot never switches the LED itself - it puts a command into a
queue, and `loop()` takes it out.

## Use it in your project

- `wifi_store.h / .cpp` and `console.h / .cpp`: copy as they are, and you
  have several networks for demo day without a password in the code
- `loop_stats.h`: one object, `stats.update(millis())` as the last line of
  `loop()` - what is YOUR longest pass?
- `net.cpp`: only if your project has something that must wait for
  seconds (bot, HTTP, MQTT connect). Replace the bot part with yours

## Troubleshooting

| Serial says                                  | Look at                                        |
|----------------------------------------------|------------------------------------------------|
| `include/secrets.h is missing`               | step 1                                         |
| `net: 0 saved network(s)`                    | step 2: `wifi add`, then `reboot`              |
| `net: no saved network found` again and again | 2.4 GHz only; name typed exactly? `LIMIT_TX_POWER` in `board.h` |
| `wifi add ...: password must be 8 to 63`     | WPA2 rule; quotes if the password has spaces   |
| `bot: getMe failed`                          | token in `secrets.h`; is the network online?   |
| bot online, no answers                       | another board polls with the same token        |
| `before` prints `WiFi: connecting...` forever | no networks saved yet - flash `after` first   |

Examples found online are often for Arduino core 3.x or ArduinoJson 7.
This project is core 2.x with ArduinoJson 6 - translate, do not paste.
