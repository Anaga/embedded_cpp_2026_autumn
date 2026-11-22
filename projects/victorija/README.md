# Winter Guardian

A battery and tamper monitor for a motorcycle parked over the winter. A sleeping node under the seat measures battery voltage, temperature, humidity and movement, and sends one LoRa packet every 20 s (demo interval). A gateway at home receives it, raises alarms and reports to a Telegram bot.

## Hardware

| Part | Node (bike) | Gateway (home) |
|---|---|---|
| Board | ESP32-C3 Super Mini | ESP32-S3-DevKitC-1U |
| Radio | Ra-01 (SX1278, 433 MHz) | Ra-01 |
| Sensors | AHT10 (I2C 0x38), MPU-6050 / GY-521 (I2C 0x68), 100k/10k battery divider | - |
| Power | 12 V -> Mean Well N7803-1PV -> 3V3 (bench: USB) | USB |

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
| On-board RGB LED | 38 |

All modules run on 3.3 V.

## Build and run

```
cd node && pio run -t upload
cd gateway && cp include/secrets.h.example include/secrets.h   # fill in WiFi, bot token, chat id
pio run -t upload && pio device monitor
```

The node prints only on a cold start. After that, watch the gateway's serial output or the bot.

## How it works

Node, one cycle per wake, all in `setup()`:

1. Wake cause: timer, GPIO 1 (motion) or cold start.
2. Battery: 64 ADC samples, trimmed mean, two-point calibration.
3. AHT10 read. MPU-6050 motion wake configured on cold start only; it keeps its settings through the C3's deep sleep.
4. One 23-byte `GuardianMsg` over LoRa (SF9, 10 dBm, ~205 ms air time).
5. `LoRa.sleep()`, NSS held high, deep sleep 20 s. Awake time is about 0.41 s.

The counter and the tamper cooldown live in `RTC_DATA_ATTR`. At most one TAMPER is sent per 2 min; during the cooldown the motion wake is disabled.

Gateway:

- `loop()`, core 1: radio only. It validates size, magic and version, counts lost packets from gaps in `seq`, checks alarms and stores history.
- `botTask`, core 0: WiFi, NTP, Telegram over HTTPS with certificate check. It answers only `CHAT_ID`.
- Shared state: a copy under a mutex (`latestLock`). Alerts go through a FreeRTOS queue.

Alarms are ON/OFF. `setAlarm()` calls `notify()` only on a change, so there is one message when an alarm starts and one when it clears.

| Alarm | On | Off |
|---|---|---|
| Battery low / critical | < 12.2 V / < 11.8 V | +0.1 V hysteresis |
| Movement | TAMPER flag | 2 min without movement |
| Node silent | 60 s without a packet | next packet |
| Temperature sensor | AHT10 missing, or a jump > 10 C between packets | plausible reading |
| Motion sensor | MPU-6050 missing | answers again |

The temperature jump check exists because an AHT10 that loses VCC still answers through the I2C pull-ups, with readings around 50 C too high.

Bot commands:
- `/status`: the latest reading and the active alarms.
- `/history [hours]`: from a ring buffer of 2000 entries, averaged into about 12 rows.

The history is in RAM, so a gateway restart clears it.

## Course lessons used

| Lesson | Where |
|---|---|
| 6 Functions, bits | `FLAG_*` bit masks in `msg.h`, flag tests everywhere |
| 7 Pointers, arrays, strings | `char name[9]` + `copyName()`, `hist[]` ring buffer, `LoRa.write((const uint8_t*)&msg, ...)` |
| 8 Structs, classes | `GuardianMsg`, `HistEntry`, `Latest`; `Aht10` and `Mpu6050` classes |
| 9 References | `read(float& t, float& h)`, `begin(TwoWire& wire = Wire)`, `const GuardianMsg&` parameters |
| 10 States, ring buffer, interrupts | alarm ON/OFF states, ring buffer history, `volatile` shared flags, MPU-6050 INT as a wake source |
| 13 I2C, AHT10 | `aht10.h` and `mpu6050.h` written without libraries (commands, registers, repeated start) |
| 14 SPI | Ra-01 over SPI; NSS held high through deep sleep |
| 18 WiFi | gateway WiFi STA, reconnect loop |
| 19 Sleep, NTP, node/gateway | deep sleep, timer and GPIO wake, `RTC_DATA_ATTR`, NTP with time zone; same node/gateway split as lab 3, with LoRa instead of ESP-NOW |
| 23 Telegram bot | `/status`, `/history`, alert once per change, `secrets.h` in `.gitignore` |
| 24 Integration | radio in `loop()`, bot in its own FreeRTOS task, mutex copy for data, queue for alerts |

## Measured

- Range: node in a car next to the bike, gateway at a home window: 51/52 packets, RSSI -90 to -95 dBm, SNR +4 to +6 dB.
- Node awake time: 413 ms per wake (cold start: 2.6 s).
- Battery after calibration: within +-0.025 V of the reference, 10.97-14.78 V.
- History run of 1.7 h: 1 packet lost of 282.
- Mean Well N7803 no-load current: 5.07 mA.

## Known problems, next steps

- On the breadboard, powering the node from the Mean Well makes the battery read about 2 V low. The node's current returns through jumper contacts and lifts the C3's ground during the ADC read. On USB the reading is correct. To be redone on the soldered board.
- The 5 mA idle current of the N7803 would drain the bike battery in about 2 months. Next step: a low-Iq regulator (Pololu D36V6F3).
- Still to do: power budget per part, AHT10 on a 1 m cable, enclosure, a soldered node.
