# Taimer

Timed watering and grow light on an ESP32-C3 Super Mini.

## What it does

- **Pump (PWM A).** Waters every 60 minutes, counted start to start. The knob
  sets how long each watering lasts: 1 to 6 minutes in 10 s steps. The first
  watering starts right after power-on. The pump ramps up over 1 s to soften
  the inrush current on the 12 V supply.
- **Light (PWM B).** 14 h on, 10 h off, counted from power-on, with a 15 min
  sunrise and sunset. Switch the device on in the morning and the cycle
  follows the real day.
- **Red LED.** Solid while the pump runs. Fast blink means a fault. A short
  flash every 5 s means the firmware is alive.
- **Safety cutoff.** If the pump output stays on longer than 7 minutes for
  any reason, it is switched off and stays off until the board is reset.

There is no clock and no network. After a power cut the day starts again
at the moment power returns, with a sunrise and a watering.

## Hardware

- ESP32-C3 Super Mini
- 2 x PWM MOSFET module, 12 V (one for the pump, one for the light)
- 12 V pump, 12 V LED light, 12 V supply
- Potentiometer 10 kohm, 3.3 kohm and 100 ohm resistors
- RGB LED, common anode (only red is used), 100 ohm resistor
- Flyback diode for the pump if the module has none (SS34 or 1N5819)

## Wiring

| Signal | ESP32-C3 pin | Notes |
|---|---|---|
| PWM A, pump module input | GPIO 6 | |
| PWM B, light module input | GPIO 7 | |
| Potentiometer wiper | GPIO 4 | ADC1 channel 4 |
| Red LED cathode, through 100 ohm | GPIO 21 | Active LOW. UART0 TX, see note |
| Module signal GND, 12 V supply GND | GND | One common ground |

GPIO 6 and 7 are plain GPIOs. GPIO 8 and 9 are avoided on purpose: they are
strapping pins (GPIO 9 is also the BOOT button), and a load module that
pulls GPIO 9 down at reset keeps the chip in download mode.

Potentiometer divider:

```
3V3 ---[3.3k]---+
                |
              [pot] <-- wiper --> GPIO 4
                |
GND ---[100R]---+
```

Red LED:

```
3V3 --- common anode    red cathode ---[100R]--- GPIO 21
```

GPIO 21 LOW turns the LED on. Current is about (3.3 - 1.9) / 100 = 14 mA,
within what a GPIO can sink. The green and blue cathodes stay unconnected.

GPIO 21 is also UART0 TX. The boot ROM prints its log there before the
firmware starts, so the LED flickers briefly after every reset. That is
expected. Serial output itself goes over USB, not UART0.

## Hardware checks before first use

### 1. Flyback diode on the pump

A pump motor is an inductive load. If the module has no diode across its
output, add one across the pump: cathode to +12 V, anode to the module's
switched output.

### 2. The module must switch at 3.3 V

Modules with a logic-level MOSFET (D4184 / AOD4184) work from 3.3 V.
Modules with an IRF520 barely turn on at 3.3 V. Opto-isolated modules are
slow: keep `PUMP_PWM_HZ` and `LIGHT_PWM_HZ` at 1 kHz or below.

Between reset and `setup()` the PWM pins are not driven, and only the
module's own pull-down keeps the load off. `setup()` drives both pins LOW
before anything else.

### 3. Potentiometer range

The ESP32-C3 ADC reads linearly only up to about 2.5 V at 12 dB
attenuation. The divider is sized for that: with 3.3 kohm on top, a 10 kohm
pot and 100 ohm at the bottom, GPIO 4 measures 0.028 V to 2.50 V, so the
whole knob travel is usable.

The 100 ohm keeps the wiper off exactly 0 V. The 3.3 kohm sets the top: an
earlier build with 1 kohm reached 3.0 V, and the last 15-20 percent of the
travel read as maximum.

Measured through `analogReadMilliVolts` with a 16-sample average:

| Knob | Reading | At rest |
|---|---|---|
| Fully down | 0 mV (28 mV is below the ADC floor) | 0-2 mV |
| Fully up | 2490-2510 mV | average within about +-30 mV |

The defaults `POT_MV_LOW = 100` and `POT_MV_HIGH = 2400` sit inside both
ends with margin, so 1 min and 6 min are reached reliably and the noise at
the top end is clamped away.

Calibration:

1. Open the serial monitor.
2. Turn the knob fully down, press `s`, note the mV value.
3. Turn it fully up, press `s`, note the mV value.
4. Set `POT_MV_LOW` and `POT_MV_HIGH` slightly inside those two values.
5. If turning clockwise shortens the watering, set `POT_REVERSED = true`.

## Settings

All settings are in the CONFIGURATION section at the top of `src/main.cpp`.
Invalid combinations stop the build with a `static_assert` message.

| Setting | Default | Meaning |
|---|---|---|
| `WATER_INTERVAL_MIN` | 60 | Minutes between watering starts |
| `WATER_ON_BOOT` | true | Water right after power-on |
| `WATER_MIN_SEC` / `WATER_MAX_SEC` | 60 / 360 | Watering length at the knob ends |
| `WATER_STEP_SEC` | 10 | Knob resolution |
| `PUMP_DUTY_PERCENT` | 100 | Pump power after the soft start |
| `PUMP_RAMP_MS` | 1000 | Soft start duration |
| `PUMP_HARD_LIMIT_SEC` | 420 | Safety cutoff, latches a fault |
| `LIGHT_ON_HOURS` / `LIGHT_OFF_HOURS` | 14 / 10 | Day and night length |
| `LIGHT_FADE_MIN` | 15 | Sunrise and sunset length, each |
| `LIGHT_MAX_PERCENT` | 100 | Full-day brightness |
| `LIGHT_PWM_HZ` / `PUMP_PWM_HZ` | 1000 | PWM frequencies |
| `POT_MV_LOW` / `POT_MV_HIGH` | 100 / 2400 | Knob calibration, see above |
| `POT_REVERSED` | false | Flip the knob direction |
| `SMOOTH_SAMPLES` | 16 | Knob moving average, one sample per tick |
| `POT_PRINT_PERIOD_MS` | 1000 | Knob changes are printed at most this often |
| `TICK_MS` | 20 | Hardware timer period |

## Serial commands

Monitor at 115200 over USB. Single characters, Enter is optional.

| Key | Action |
|---|---|
| `w` | Water now. The next scheduled watering moves to 60 min from now. |
| `p` | Stop the pump. The schedule is unchanged. |
| `s` | Status: uptime, light phase, pump, knob in mV and seconds, next watering |
| `h` | Help |

## Build and upload

VS Code with PlatformIO:

```
pio run -t upload
pio device monitor
```

The code uses the Arduino core 2.x APIs (`ledcSetup`, `timerBegin` with a
divider). PlatformIO `espressif32` 6.x and 7.x both ship core 2.0.17. If a
future platform switches to core 3.x, the build stops with a clear message.

## How it works

- A hardware timer interrupt fires every 20 ms and only increments a
  counter. `loop()` processes the ticks one by one, so a slow Serial write
  delays a step but never skips one.
- The watering schedule, the pump soft start, the light fades and the LED
  patterns all count the same ticks.
- The light brightness is computed from the position in the 24 h cycle, so
  it cannot drift out of step with the schedule. Brightness is squared
  before it goes to the PWM, so the fade looks even to the eye.
- The safety cutoff watches the pump output itself, not the schedule logic.
- The cycle is only as accurate as the board's crystal, which is good to
  a few seconds per day.
