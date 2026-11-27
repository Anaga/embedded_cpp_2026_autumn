# PISI - wiring and safety

This document is based on `include/Config.h` in this ZIP and the photos of the controller board. It lists **GPIO numbers**, not the physical positions of the pins. Before powering the board, check the printed labels and measure continuity on the real board.

## 1. ESP32-C3 pin list

| Part | ESP32-C3 connection | Other connection |
|---|---|---|
| Pump MOSFET control | GPIO3, PWM output | Pump module `TRIG/PWM` (if confirmed active-HIGH, 3.3 V compatible) |
| Light MOSFET control | GPIO10, PWM output | Light module `TRIG/PWM` (same checks) |
| Pump and light control ground | ESP32 GND | Each module's control GND / common 12 V negative as required by its design |
| AHT10 data | GPIO0 | AHT10 SDA |
| AHT10 clock | GPIO1 | AHT10 SCL |
| AHT10 power | 3V3 and GND | AHT10 VCC and GND (only if module rated for 3.3 V) |
| Potentiometer | GPIO4 | Center/wiper, with the original 3.3 kOhm top and 100 Ohm bottom resistors |
| RGB LED red | GPIO5 | Red cathode through its own resistor |
| RGB LED green | GPIO6 | Green cathode through its own resistor |
| RGB LED blue | GPIO7 | Blue cathode through its own resistor |
| RGB LED common anode | 3V3 | The actual common-anode leg of the LED; verify LED pin order |
| Light detector (optional, **not installed in code**) | GPIO20 | Digital DO only; `LIGHT_SENSOR_INSTALLED=false` |
| Controller power / programming | USB-C | Regulated 5 V USB supply |

GPIO2, GPIO8 and GPIO9 are boot-strapping pins and are not used in this version. GPIO21 is not assigned. Never connect 12 V to an ESP32 GPIO.

## 2. Two 12 V MOSFET modules

Read the exact labels on **your** modules. A board may say `VIN+`, `VIN-`, `OUT+`, `OUT-`, `TRIG/PWM`, `GND`. Only use this table when those names match your hardware and its datasheet:

| Connection | Pump module | Grow-light module |
|---|---|---|
| 12 V supply positive | `VIN+` | `VIN+` |
| 12 V supply negative | `VIN-` | `VIN-` |
| Load positive | `OUT+` to pump + | `OUT+` to LED strip + |
| Load negative | `OUT-` to pump - | `OUT-` to LED strip - |
| Control signal | `TRIG/PWM` from GPIO3 | `TRIG/PWM` from GPIO10 |
| Control ground | `GND` to ESP32 GND | `GND` to ESP32 GND |

**Do not choose positive and negative by cable color alone.** Confirm the markings on the pump, LED strip and modules. A reversed connection previously caused dangerous heating in this project. Check continuity with **all power removed** before starting again. Replace any wire or pump damaged by heat.

The 12 V positive can be present at the module output even if the negative-side MOSFET is OFF. Measure the voltage **across the connected load**, not from just one output terminal to supply ground. A module's red indicator usually shows a control condition, not an alarm.

The original pump uses a 1-second PWM ramp from 65% to 100%, confirmed as working by the project owner. Stop if the brushless pump stalls, clicks, restarts or heats up. Confirm modules do not turn themselves on when the ESP32 is unpowered; check whether they require a control pull-down resistor.

## 3. RGB status LED

The RGB LED is **common-anode**. Connect the common leg to ESP32 3V3. Connect each color through a separate current-limiting resistor to GPIO5 (red), GPIO6 (green) and GPIO7 (blue). `LOW` turns a color ON.

- **Fault:** red 200 ms ON / 200 ms OFF
- **Only pump:** steady blue
- **Only grow lights:** steady green
- **Both:** blue 200 ms ON, 200 ms OFF, green 200 ms ON, 200 ms OFF
- **Neither:** green heartbeat 100 ms ON every 1000 ms

## 4. Power and water safety

- Unplug both the 12 V supply and USB before moving any wire.
- Use insulated connectors, strain relief and proper fuses rated for the actual wires and loads. Do not send pump/LED current through a small breadboard.
- Keep the 12 V power supply, MOSFET modules, exposed joints and controller in a dry enclosure, above possible water spills. Keep mains connections inside approved closed equipment.
- Do not use aluminum foil as a work surface: it is electrically conductive.
- Make sure the pump has enough water and the tray has a working return path.
- The 40-second pump cutoff runs in software and does **not** prevent damage from shorts, a dry pump, a frozen controller or a stuck MOSFET.
- If any wire, pump or module becomes hot, smells unusual, or changes color, disconnect both power sources and stop testing.

## 5. Diagram

Open [`WIRING_DIAGRAM.svg`](WIRING_DIAGRAM.svg) in a browser for a visual **functional** connection diagram. It is not a photo, physical pin map or certified electrical schematic.
