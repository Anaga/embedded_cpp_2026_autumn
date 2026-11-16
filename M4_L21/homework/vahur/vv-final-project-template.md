# Final project: <project name>

Author: <your name>
Date: <YYYY-MM-DD>
Status: draft -> approved (instructor fills this in on Monday)

Fill every section. One or two sentences is enough where nothing more is
needed; "TBD" is allowed only in section 9. Keep it ASCII, keep it in English.

---

## 1. One-sentence summary

What the device does, for whom, in one sentence.

> Example: A desk air monitor that shows temperature and humidity on the
> Nokia 5110 and publishes them over MQTT every minute.

## 2. Why this project

Two or three sentences: what problem it solves or what you want to learn
from it. Personal use is a perfectly good reason.

## 3. Platform

Pick one and say why in one line:

- [ ] ESP32-C3 Super Mini
- [ ] Raspberry Pi Pico (RP2040)
- [ ] STM32F401 Black Pill
- [ ] Other: <which, and do you already have it>

## 4. Hardware

List every part. Mark what you already have (course kit or your own) and
what still has to be bought or borrowed. Nothing on this list may arrive
after Friday 2026-11-20.

| Part | Interface | Have it? | Source |
|------|-----------|----------|--------|
| <sensor / display / actuator> | I2C / SPI / UART / GPIO / ADC / PWM / CAN / WiFi | yes / no | course kit / own / order |
| | | | |

## 5. Pin budget

Every pin you use, in one table. Check it against the pinout from the course
conventions: strapping pins (C3: 2, 8, 9), boot buttons, on-board LEDs, 5 V
tolerance. Two things on one pin = not approved.

| GPIO | Function | Notes |
|------|----------|-------|
| | | |

## 6. Behaviour

Describe what the device does as a short list of states or steps. If there
is a state machine, name the states. If there is a timing requirement
("every 2 s", "within 100 ms of the button"), write the number.

1. On power-up: ...
2. Normal operation: ...
3. On <event>: ...
4. On error (sensor missing, no WiFi, bus silent): ...

## 7. Interfaces and protocols

Which buses, which libraries, which messages. If data leaves the board (UART
line format, MQTT topic and payload, CAN ID and bytes, HTTP endpoint), write
the exact format here.

| Channel | Direction | Format |
|---------|-----------|--------|
| | in / out | |

## 8. Minimum and stretch

Two lists. "Minimum" is what you demo on 2026-11-27 even if everything goes
wrong; it must be reachable in two sessions of work. "Stretch" is what you
add if the minimum is done by Monday 2026-11-23.

Minimum (must work on demo day):
- ...
- ...

Stretch (nice to have):
- ...

## 9. Open questions and risks

What you do not know yet, what might not work, what you need from the
instructor (a part, a library, a decision). "TBD" lives here and nowhere
else.

- ...

## 10. Plan for the three sessions

| Session | Date | Goal at the end of the evening |
|---------|------|--------------------------------|
| 23 | Fri 2026-11-20 | Hardware wired, every peripheral talks (one test program per peripheral) |
| 24 | Mon 2026-11-23 | Minimum behaviour works end to end |
| 25 | Fri 2026-11-27 | Demo: 5 minutes, device running, two sentences on what was hard |

Adjust the goals to your project; keep the dates.
