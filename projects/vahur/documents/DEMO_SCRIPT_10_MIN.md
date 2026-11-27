# PISI - 10-minute final demonstration script

**Language:** Simple English. Use your own words. After the talk there are 5 minutes for questions.

## Minute 0-1: Idea

"This is PISI, my automatic microgreens system. It can water plants and control LED lights. A small ESP32-C3 board controls the system. The goal is to make growing plants easier and to learn embedded C++."

## Minute 1-3: Hardware

Point at each part:

- "This is the ESP32-C3 controller. It uses USB for power and programming."
- "These two MOSFET modules switch the 12 V pump and the 12 V grow lights. The ESP32 sends only a 3.3 V control signal."
- "This is the AHT10 temperature and humidity sensor. If the temperature is too high, the lights stop."
- "This RGB LED shows the system status. Blue is the pump. Green is the grow lights. Red is a fault."
- "The knob is connected, but it does not change the fixed pump time in this demo."

Show the pin table in `docs/WIRING_AND_SAFETY.md`.

## Minute 3-7: Live demo

1. Start the system safely. Both schedules start after power-up.
2. Show that the pump runs for 30 seconds and stops for 90 seconds.
3. Show the grow lights: 120 seconds on, 60 seconds off; the first and last 10 seconds are brightness fades.
4. Show the RGB LED. Both working: blue ON 200 ms, OFF 200 ms, green ON 200 ms, OFF 200 ms. Light only: steady green. Pump only: steady blue. Both off: short green heartbeat.
5. Open Serial Monitor at 115200 baud and type `s`. Point to the pump, light, temperature and fault values.
6. Explain that **28 C** switches the grow lights off. They may restart after at least **5 minutes** only when the temperature is **below 24 C** and the light schedule says ON. Do not heat electrical parts just to demonstrate this.

*Tip:* Start the demo before the presentation. If you have little time, show a 30-second backup video of one complete cycle.

## Minute 7-9: Explain the program

Show `src/main.cpp` and one class, for example `PumpController.h`.

"I used a class for each main part. Each class has a `begin()` function for setup and an `update()` function for regular work. The `loop()` does not wait with long `delay()` calls. This allows different timers to work together."

Show `LoopStats` messages on Serial: `LOOP: passes=... in 5s, longest interval=... us`. Report **your measured value**, not an example number.

## Minute 9-10: Problems and next steps

"One positive power wire was connected to a negative terminal. I found the mistake when checking the wiring. I corrected the connection and the system worked. I also learned that I must check polarity before applying power."

"Next I would add a low-water sensor, better plant-light measurement and a safe closed cabinet."

## Prepare for likely questions

| Question | Short answer |
|---|---|
| Why a MOSFET? | The ESP32 cannot directly supply the current of the 12 V pump or lights. |
| Why `millis()`? | It lets the program check many things without waiting. |
| What happens if AHT10 is unplugged? | The grow lights stay OFF and the RGB status flashes red. |
| When do lights resume after overheating? | At least 5 minutes later, with AHT10 below 24 C, and only during the light ON period. |
| What was the hardest problem? | The wrong positive/negative wire. |
| What is the longest loop interval? | Give the real number from Serial Monitor. |

## Items on the table

ESP32 controller, 12 V power supply, pump in water, safe tray and drain, grow LEDs, USB cable, backup video, and the README open in VS Code. Keep wires dry and insulated.
