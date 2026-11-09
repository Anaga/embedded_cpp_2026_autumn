# Homework 20 - MQTT thermostat

One ESP32-C3 at home, with the AHT10 and the RGB LED wired as in class.
Due before session 21.

## Start

Copy `../lab1-node/src/main.cpp` into your own folder (`<name>/src/`) and
put a `secrets.h` next to it (copy `../lab1-node/include/secrets.example.h`).
In `platformio.ini` uncomment only your own `src_dir` line and do not commit
`platformio.ini`. `secrets.h` is ignored by git - keep it that way.

## Broker at home

Pick one:

- **Your own Mosquitto** (recommended): macOS `brew install mosquitto`,
  Windows: the installer from mosquitto.org. Run it with the class config:
  `mosquitto -c ../broker/class.conf -v`. `MQTT_HOST` = your laptop IP.
  Allow port 1883 in the firewall when asked.
- **broker.hivemq.com**, port 1883: no install, but public - anyone can read
  and change your topics. Keep `MQTT_PREFIX = "gamma-ed/2026"`.

MQTT Explorer works with both.

## Task

The node becomes a room thermostat. The setpoint comes over MQTT, the
decision is made on the board.

Topics (`<p>/<id>` as in lab 1):

| Topic | Direction | Payload | Retained |
|---|---|---|---|
| `<p>/<id>/status` | node -> all | `online` / `offline` (last will) | yes |
| `<p>/<id>/aht10` | node -> all, every 5 s | `{"t":21.8,"rh":58.1,"sp":21.0,"mode":"OK","seq":12}` | no |
| `<p>/<id>/setpoint` | you -> node | `21.5` | yes - you publish it with retain |
| `<p>/<id>/mode` | node -> all | `HEAT`, `OK`, `COOL` or `SENSOR` | yes, only on change |
| `<p>/<id>/error` | node -> all | a short text, e.g. `setpoint "abc" rejected` | no |

Requirements:

1. **Setpoint.** Valid range 15.0 .. 30.0 C. Anything else (out of range,
   `abc`, empty, `21.5x`) is rejected: the old setpoint stays, one line on
   Serial, one message on `error`. Until the first valid setpoint arrives,
   use a default of 21.0 C. Range and default are named constants.
2. **Mode and RGB LED.** With a band of +-0.5 C (named constant):
   - `HEAT` - temperature below setpoint - band: red
   - `COOL` - temperature above setpoint + band: blue
   - `OK` - inside the band: green
   - `SENSOR` - AHT10 missing: LED off
3. **The network is optional, the thermostat is not.** Without WiFi or
   broker the LED keeps working with the last setpoint. (So the red channel
   no longer means "no broker" as in lab 1 - Serial tells you that.)
4. **Retained setpoint.** Reboot the board (RESET). Without you publishing
   anything, the setpoint must come back. Explain in `NOTES.md` why.
5. Keep the last will, the non-blocking reconnect and the callback rule
   (copy, flag, return - no publishing inside the callback).
6. Leave a comment in the code: what happens when the temperature sits right
   at the edge of the band? Hysteresis is optional.

## NOTES.md

In your folder, 10-20 lines, ASCII only:

- Why the setpoint survives a reboot of the node, and what happens to it
  when the broker restarts (Mosquitto with `persistence false`).
- One fault you caused yourself, in the lab 2 format: case, symptom,
  hypotheses, evidence, conclusion, fix. Ideas: stop the broker, switch off
  the WiFi, publish the setpoint without retain, publish `abc`.

## Hints

- Parse with `strtof()` and check the end pointer: `21.5x` must fail.
- `mqtt.subscribe()` after every connect, not once in `setup()`.
- Publish `mode` only when it changes, otherwise it is not a state but noise.
- To delete a retained setpoint: publish an empty retained message (or the
  trash icon in MQTT Explorer). What does your node do then?

## Submit

Commit only your own folder: `<name>/src/main.cpp` and `<name>/NOTES.md`.
Commit message in English, for example `hw20: mqtt thermostat`.
