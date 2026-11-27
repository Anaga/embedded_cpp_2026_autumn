# PISI - how the software works

The source files in this ZIP use the Arduino framework on the ESP32-C3. Each part is a small C++ class. The main `loop()` calls `update(now)` regularly so the pump, lights and RGB indicator can work at the same time.

## Classes

| File | Job |
|---|---|
| `src/main.cpp` | Starts components, connects their states, handles Serial `w`, `p`, `s`, `h` |
| `include/Config.h` | Pins, timing, PWM settings and temperature limits |
| `include/PumpController.h` | Pump timing, 1-second PWM start, manual start/stop, software fault cutoff |
| `include/LightController.h` | Light timing, 10-second fades, safety stop |
| `include/TemperatureMonitor.h` | Reads AHT10 and checks for overheat |
| `include/RgbLed.h` | Turns the three LED colors on/off |
| `include/StatusIndicator.h` | Chooses red, blue, green or the two-color pattern |
| `include/Potentiometer.h` | Reads the knob every 100 ms; **does not change** demo watering time |
| `include/LoopStats.h` | Prints loop count and the longest interval between calls every 5 s |

## Timing and ownership

| Part | When | Runs in |
|---|---|---|
| Pump controller and safety cutoff | Every loop pass | Main `loop()` |
| Grow-light schedule | Every loop pass | Main `loop()` |
| RGB status | Every loop pass | Main `loop()` |
| AHT10 temperature/humidity | Every 2 seconds, reconnect retry every 5 seconds | Main `loop()` |
| Potentiometer | Every 100 ms | Main `loop()` |
| Serial commands | Up to 12 input characters in each pass | Main `loop()` |
| Status message | Every 5 seconds | Main `loop()` |
| LoopStats | Each pass; report every 5 seconds | Main `loop()` |

There is no extra FreeRTOS task and no Wi-Fi in this demonstration. AHT10 library reads may still take some time. `LoopStats` measures the time between successive `update()` calls (including interrupts and scheduling), not a precise CPU-only measurement of the body of `loop()`.

## Pump schedule

- Immediately after setup: pump starts for 30 seconds. PWM rises from 65% to 100% during 1 second.
- Then pump stays off for 90 seconds.
- This is a fixed 120-second start-to-start cycle: 0-30 ON, 30-120 OFF, 120-150 ON, and so on.
- If a run lasts 40 seconds, the software stops it and sets a pump fault until reset.
- `w` manually starts the pump and restarts the 120-second cycle. `p` stops the current run but does not stop future automatic cycles.
- The potentiometer is read for information only.

## Light schedule and temperature protection

- Light schedule: 120 seconds on, then 60 seconds off; repeat (180-second cycle).
- Brightness increases during the first 10 seconds of the ON period and decreases during the last 10 seconds.
- If AHT10 is missing or not working, lights are kept OFF.
- If measured temperature is **28 C or above**, lights turn OFF (thermal pause).
- After at least **300 seconds** in thermal pause, with a valid temperature reading **strictly below 24 C**, lights may resume **only when the schedule is in its ON part**.
- A pump fault also switches the lights OFF. The pump schedule is independent from an AHT10 failure.

## LED priority

1. Pump fault, AHT10 failure or thermal pause -> flashing red.
2. Pump and grow lights both active -> alternating blue and green.
3. Pump only -> blue steady.
4. Grow lights only -> green steady.
5. Both off and no fault -> short green heartbeat.

## Serial Monitor

`h` = help; `s` = print status; `w` = water now; `p` = stop pump. Speed: **115200 baud**.

Source settings live in `include/Config.h`. Normal production timing and real calibrated plant-light measurement are **future work**.
