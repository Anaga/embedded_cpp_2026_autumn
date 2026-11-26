# Winter Guardian

A battery and tamper monitor for a motorcycle parked over the winter. A sleeping node under the seat measures battery voltage, temperature, humidity and movement, and sends one LoRa packet every 20 s (demo interval; 10 min for real use). A gateway at home receives it, raises alarms, shows the most important one on its LED and reports to a Telegram bot.

## Hardware

| Part | Node (bike) | Gateway (home) |
|---|---|---|
| Board | ESP32-C3 Super Mini | ESP32-S3-DevKitC-1U |
| Radio | Ra-01 (SX1278, 433 MHz), spring antenna | Ra-01 |
| Sensors | AHT10 (I2C 0x38), MPU-6050 / GY-521 (I2C 0x68), 100k/10k battery divider | - |
| Power | USB (demo); later 12 V through a 3.3 V regulator | USB |

The node exists twice: the first on a breadboard, the second soldered onto a 4 x 6 cm protoboard (same pins, same firmware) to fit under the seat.

## Layout

```
common/    msg.h (packet struct), radio.h (pins + LoRa settings), used by both
node/      PlatformIO project, ESP32-C3
gateway/   PlatformIO project, ESP32-S3
```

Both projects add `-I ../common` in `platformio.ini`.

## Pins

Node (ESP32-C3):

| Function | GPIO |
|---|---|
| LoRa SCK / MISO / MOSI / NSS / RST | 4 / 5 / 6 / 7 / 10 (DIO0 not used) |
| I2C SDA / SCL (AHT10, MPU-6050) | 8 / 9 |
| MPU-6050 INT (deep sleep wake) | 1 |
| Battery ADC (divider midpoint, 100 nF to GND) | 3 |

Strapping pins: GPIO 8 and 9 are used for I2C. Both must be high at reset for a normal boot, and the I2C pull-ups keep them high, so this is safe. GPIO 2 is not used.

Gateway (ESP32-S3):

| Function | GPIO |
|---|---|
| LoRa SCK / MISO / MOSI / NSS / RST / DIO0 | 12 / 13 / 11 / 10 / 9 / 14 |
| On-board RGB LED (WS2812) | 38 |

All modules run on 3.3 V.

## Build and run

```
cd node && pio run -t upload
cd gateway && cp include/secrets.h.example include/secrets.h   # fill in bot token, chat id
pio run -t upload && pio device monitor
wifi add "<ssid>" <password>     # typed in the monitor, once per network, then: reboot
```

WiFi networks are saved in NVS (up to 5); the gateway joins the strongest one it sees. `wifi_store.*`, `console.*` and `loop_stats.h` come from lesson 24.

The node prints only on a cold start. After that, watch the gateway's serial output or the bot.

## Node

One cycle per wake, all in `setup()`:

1. Wake cause: timer, GPIO 1 (motion) or cold start.
2. Battery (`battery.h`): 64 ADC samples, trimmed mean, two-point calibration.
3. AHT10 read. MPU-6050 motion wake configured on cold start only; it keeps its settings through the C3's deep sleep.
4. One 23-byte `GuardianMsg` over LoRa (SF9, 10 dBm, about 205 ms air time).
5. `LoRa.sleep()`, NSS held high, deep sleep 20 s. Awake time is about 0.41 s.

The counter and the tamper cooldown live in `RTC_DATA_ATTR`. During the cooldown the motion wake is disabled, so a bike being pushed around doesn't wake the node continuously.

## Gateway

Built the lesson 24 way: one class per part, each with `begin()` and `update(now)`; `loop()` only calls them in turn, and no part waits. WiFi and Telegram must wait, so they run in their own task on core 0. The parts are listed at the top of `gateway/src/main.cpp`; every tunable number is in `config.h`.

| File | Part |
|---|---|
| `receiver.*` | reads a packet, checks size, magic and version, counts lost packets from gaps in `seq` |
| `alarms.*` | the alarm states, start/clear messages with advice |
| `history.*` | ring buffer of 2000 entries, its own mutex; `/history` averages it into about 12 rows |
| `status_led.*` | the most important problem as a colour (movement, battery, condensation, system fault, all OK) |
| `net.*` | the net task: WiFi, NTP, Telegram over HTTPS with certificate check; answers only `CHAT_ID` |
| `shared.*` | what `loop()` and the net task share: a mutex-protected copy of the latest reading, the alert queue |
| `format.*`, `emoji.h` | text helpers; emoji as UTF-8 escapes so the sources stay ASCII |

Alarms are ON/OFF and send one message when they start and one when they clear.

| Alarm | On | Off |
|---|---|---|
| Battery low / critical | < 12.2 V / < 11.8 V | +0.1 V hysteresis |
| Movement | TAMPER flag | 10 s without movement (demo; real: 2 min) |
| Node silent | 3 reports missed | next packet |
| Condensation risk | > 85 %RH | < 80 %RH |
| Temperature sensor | AHT10 missing, outside -40...+60 C, or a jump > 10 C | plausible reading |
| Motion sensor | MPU-6050 missing | answers again |
| Battery reading | below 3 V or above 16 V (divider not connected) | plausible reading |

The temperature checks exist because an AHT10 that loses VCC still answers through the I2C pull-ups, with readings around 50 C too high.

Alerts are queued while the gateway is offline and sent when it reconnects.

Bot: a status card with buttons (Status, History, Mute), and the commands `/status`, `/history [hours]`, `/mute`, `/unmute`. The history is in RAM, so a gateway restart clears it.

## Course lessons used

| Lesson | Where |
|---|---|
| 6 Functions, bits | `FLAG_*` bit masks in `msg.h`, flag tests everywhere |
| 7 Pointers, arrays, strings | `char name[9]` + `copyName()`, `hist[]` ring buffer, `LoRa.write((const uint8_t*)&msg, ...)` |
| 8 Structs, classes | `GuardianMsg`, `HistEntry`, `Latest`; classes `Aht10`, `Mpu6050`, `Battery`, `Alarms`, `History`, `StatusLed`, `Receiver` |
| 9 References | `read(float& t, float& h)`, `begin(TwoWire& wire = Wire)`, `const GuardianMsg&` parameters |
| 10 States, ring buffer, interrupts | alarm ON/OFF states (`enum class Alarm`), ring buffer history, `volatile` shared flags, MPU-6050 INT as a wake source |
| 13 I2C, AHT10 | `aht10.h` and `mpu6050.h` written without libraries (commands, registers, repeated start) |
| 14 SPI | Ra-01 over SPI; NSS held high through deep sleep |
| 18 WiFi | gateway WiFi STA, reconnect every 5 s, alerts queued while offline |
| 19 Sleep, NTP, node/gateway | deep sleep, timer and GPIO wake, `RTC_DATA_ATTR`, NTP with time zone; same node/gateway split as lab 3, with LoRa instead of ESP-NOW |
| 23 Telegram bot | status card, `/status`, `/history`, `/mute`, reply-keyboard buttons, alert once per change with advice, `secrets.h` in `.gitignore` |
| 24 Integration | one class per part with `begin()` / `update(now)`, `loop()` is a list of updates, no part waits; WiFi and Telegram in a FreeRTOS task on core 0; mutex copy for shared data, queue for alerts; `LoopStats`; WiFi list in NVS + WiFiMulti, serial console |

## Measured

- Range: node in a car next to the bike, gateway at a home window: 51/52 packets, RSSI -90 to -95 dBm, SNR +4 to +6 dB.
- Node awake time: 413 ms per wake (cold start: 2.6 s).
- Battery, breadboard node after calibration: within +-0.025 V of the reference, 10.97-14.78 V.
- Battery, soldered node, without calibration: 12.005 V at 12.00 V, 14.42 V at 14.40 V.
- History run of 1.7 h: 1 packet lost of 282.
- Mean Well N7803 no-load current: 5.07 mA.

## Known problems, next steps

- On the breadboard, powering the node from the Mean Well made the battery read about 2 V low: the node's current returned through jumper contacts and lifted the C3's ground during the ADC read. To be redone on the soldered board.
- The 5 mA idle current of the N7803 would drain the bike battery in about 2 months. Next step: a low-Iq regulator.
- One Ra-01 pad joint on the soldered node is intermittent (MISO); to be reflowed.
- Still to do: power budget per part, enclosure, running on the bike's 12 V.
