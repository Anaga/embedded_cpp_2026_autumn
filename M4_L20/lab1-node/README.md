# Lab 1 - AHT10 node on the class MQTT broker

The node reads the AHT10 every 5 s and publishes the reading, its status and
its LED state to the class broker (Mosquitto on the instructor laptop). It
listens for LED commands. No web server this time - only MQTT.

## Wiring

Unchanged from session 18:

| Function | Pin | Notes |
|---|---|---|
| AHT10 SDA / SCL | GPIO 8 / GPIO 9 | power from 3V3, never 5V |
| RGB LED R / G / B | GPIO 5 / 6 / 7 | common anode to 3V3, LOW = on |

GPIO 8 and 9 are strapping pins. GPIO 9 is also the BOOT button: pressing it
during a reading spoils the reading. The blue on-board LED on GPIO 8
flickers with every I2C transfer - that is expected, not a bug.

RGB LED: **red** = no connection to the broker, **green** = the LED you
switch over MQTT.

## Run

1. Copy `include/secrets.example.h` to `include/secrets.h`. Fill in the class
   WiFi and `MQTT_HOST` = the instructor laptop IP (on the board).
2. `pio run -e c3 -t upload`, then `pio device monitor -e c3`.

Known-good output (your id and numbers differ):

```
Lab 1 - MQTT node esp-9494, client id gamma-ed-esp-9494
AHT10: found at 0x38
MQTT: broker 192.168.3.10:1883
  publish   gamma-ed/2026/esp-9494/status, gamma-ed/2026/esp-9494/aht10, gamma-ed/2026/esp-9494/led
  subscribe gamma-ed/2026/esp-9494/led/set
WiFi: connected, IP 192.168.3.43, -55 dBm
MQTT: connected
esp-9494 | WiFi -55 dBm | MQTT ok | 21.8 C  58.1 % | LED off | seq 1
```

**Save your own output and a screenshot of your node in MQTT Explorer.**
Lab 2 compares against it.

## Topics

`<p>` = `gamma-ed/2026`, `<id>` = `esp-xxxx` from the MAC address.

| Topic | Direction | Payload | Retained |
|---|---|---|---|
| `<p>/<id>/status` | node -> all | `online` / `offline` (last will) | yes |
| `<p>/<id>/aht10` | node -> all, every 5 s | `{"t":21.8,"rh":58.1,"seq":12}` | no |
| `<p>/<id>/led/set` | anyone -> node | `1` / `0` | never |
| `<p>/<id>/led` | node -> all | `1` / `0`, the real state | yes |

## Try it

- MQTT Explorer: host = laptop IP, port 1883, no user name. Find your node.
- Publish `1` to `<p>/<id>/led/set`: green on, `led` follows.
- Publish `maybe`: read the Serial line.
- Pull the USB cable and time it: when does `status` turn `offline`? Why?
- Subscribe to `gamma-ed/2026/+/aht10` to see the whole class.

## When it does not connect

The node prints the PubSubClient state on every failed attempt:

| state | Meaning | First suspect |
|---|---|---|
| -4 | timeout | weak WiFi, busy broker |
| -3 | connection lost | broker stopped, WiFi dropped |
| -2 | TCP connect failed | broker down, wrong IP or port, firewall |
| 0 | connected | not the connection - look at the topics |
| 2 | client id rejected | id empty or not allowed |
| 4 / 5 | bad credentials / not authorized | user name, password |

No WiFi at all next to the router: the sketch already sets
`WiFi.setTxPower(WIFI_POWER_8_5dBm)`; tell the instructor.

## Lab 2

The instructor flashes a mystery firmware on your board or changes
something in the class network, and gives you a bug report. Find the fault
with Serial, the broker log on the wall and MQTT Explorer. Write for each
case: symptom, hypotheses, evidence, conclusion, fix. Then upload lab1-node
again and check it against your known-good output.
