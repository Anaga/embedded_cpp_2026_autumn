# Lab 2 - AHT10 + WiFi + a local web server + MQTT

Lab 1 plus MQTT. The board also publishes its status to a broker every 5 s
and listens for LED commands. Everything from lab 1 (page, `/status`,
`POST /led`) still works.

Wiring and the first steps are the same as in `../lab1-web/README.md`.

## Run

1. Copy `include/secrets.example.h` to `include/secrets.h`: the class WiFi,
   and the broker (default `broker.hivemq.com:1883`, no login).
2. `pio run -e c3 -t upload` and `pio device monitor -e c3`.

Expected, in addition to lab 1:

```
MQTT: broker.hivemq.com:1883, publish gamma-ed/2026/esp-9494/status, subscribe gamma-ed/2026/esp-9494/led
MQTT: connected
esp-9494 | WiFi 192.168.3.43 -55 dBm | 23.5 C  53.6 % | LED off | MQTT ok
```

## Topics

| Topic | Direction | Payload |
|---|---|---|
| `gamma-ed/2026/<id>/status` | board -> broker, every 5 s | the same JSON as `/status` |
| `gamma-ed/2026/<id>/led` | broker -> board | `1` = on, `0` = off |

## Try it

```
# watch the whole class
mosquitto_sub -h broker.hivemq.com -t 'gamma-ed/2026/#' -v

# switch your LED
mosquitto_pub -h broker.hivemq.com -t 'gamma-ed/2026/esp-xxxx/led' -m 1
```

Install the tools: macOS `brew install mosquitto`; Windows: the installer
from mosquitto.org. Without them: MQTT Explorer (an app), or the HiveMQ
WebSocket client in the browser (`broker.hivemq.com`, port 8000).

Switch the LED from the page, then from MQTT: the page follows, because
both end in the same `ledSet()`.

## When MQTT does not connect

The monitor prints the PubSubClient state with a short explanation:

| State | Meaning |
|---|---|
| -2 | no TCP connection to the broker: wrong host or port, no internet, port blocked |
| -4 | the broker did not answer in time |
| 2 | client id rejected |
| 4 / 5 | wrong user name or password / not authorized |

## A public broker is public

Anyone can read these topics and publish to them - including your LED
topic. No passwords and no personal data in messages. The client id has to
be unique on the whole broker; the demo uses `gamma-ed-esp-xxxx`.

MQTT is off in access point mode (`USE_SOFTAP = true`): no internet.
