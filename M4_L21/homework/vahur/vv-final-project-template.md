# Final project: PISI - Automated Microgreens System

Author: Vahur Varik
Date: 2026-11-16
Status: draft -> approved (instructor fills this in on Monday)

---

## 1. One-sentence summary

Auto-micrgreen grower is a small home system that waters microgreens from a tank and can control grow lights with an ESP32-C3.

## 2. Why this project

I want to grow plants at home with less daily work. I also want to learn how to connect a pump, sensors, lights and C++ code in one real project.

## 3. Platform

- [x] ESP32-C3 Super Mini
- [ ] Raspberry Pi Pico (RP2040)
- [ ] STM32F401 Black Pill
- [ ] Other

I already have this board and some working pump code with PWM (Pulse-Width Modulation).

## 4. Hardware

The parts marked "yes" are parts I have confirmed. Buy or check the other parts before Friday 2026-11-20. The store links below are product options.

| Part | Interface | Have it? | Source |
|------|-----------|----------|--------|
| ESP32-C3 Super Mini | GPIO / ADC / PWM / USB | yes | Course kit |
| 12 V water pump | PWM through MOSFET | yes | AliExpress |
| MOSFET module for pump | GPIO6 / PWM | yes | Course kit |
| 10 kohm potentiometer | ADC | yes | own |
| Water reservoir | Water system | yes | own |
| Water tubes | Water system | yes | own |
| Microgreens tray | Water system | yes | own |
| Resistors: 3.3 kohm and 100 ohm | ADC / GPIO | yes | own |
| Common-anode RGB LED | GPIO21 | yes | own |
| 12 V, 100 W supply: Mean Well LPV-100-12 | 12 V power | no | [Oomipood - LPV-100-12](https://www.oomipood.ee/product/lpv_100_12_toiteplokk_12vdc_8_5a_100w_ip67_mean_well), about EUR 32; check pump current first |
| 12 V, 4000 K LED strip: LEMLUX COB, 5 m | PWM through second MOSFET | no | [Oomipood - COB-11NW480-12](https://www.oomipood.ee/product/led_lint_cob_5m8mm_12v_11wm_1200lm_4000k_ip20_premium), about EUR 29; test plant light level |
| Second MOSFET: OKYSTAR OKY3496-8 | GPIO7 / PWM | no | [Oomipood - OKY3496-8](https://www.oomipood.ee/product/mos_reguleeritav_kontaktivaba_luliti_3320vdc_536vdc_15a), about EUR 2.50 |
| Light sensor: OKYSTAR OKY3105-1 | GPIO0 / ADC | no | [Oomipood - OKY3105-1](https://www.oomipood.ee/product/ldr_fototakisti_moodul_reguleeritav_335vdc_ttl), about EUR 3 |
| Aluminum profile for LED strip | Mechanical | no | [Oomipood - LED profile kit](https://www.oomipood.ee/product/led_profiili_komplekt_nurka_202m_anodeeritud_valge); check fit and quantity |
??? | 12 V to 5 V USB converter to run ESP32 from power supply: OKYSTAR OKY3501-1 | Power / USB | no | [Oomipood - OKY3501-1](https://www.oomipood.ee/product/oky3501_1_toitemoodul_dc_dc_step_down_6_24v_5v_3a_usb_pesa_15w), about EUR 6; optional |
??? | Pump flyback diode | Motor protection | no | [Oomipood - 1N5819](https://www.oomipood.ee/product/1n5819_1n5819_sb_d_40v_1a_25ap); check motor rating |
| Wires, connectors, fuse? and safe box | Power / GPIO | no | order or course kit; choose ratings after checking the pump |
| USB-C cable and USB charger | USB / power | yes | own |
??? | Aquarium filter | Water system | no | https://zoomaailm.ee/ee/akvaariumi-filter-hl-bt400-7w-380l-h-50-110l |

The 12 V LED strip is made for indoor lighting, not sold as a grow-light product. I need to test if it gives enough light for microgreens. The IP20 strip must stay in a dry area, away from splashing water.

## 5. Pin budget

| GPIO | Function | Notes |
|------|----------|-------|
| GPIO0 | Light sensor analog output | extra goal; power sensor at 3.3 V |
| GPIO3	| Red LED | Red flashes until the problem is fixed and the controller is reset |
| GPIO5	| Green LED | A short green heartbeat every 1000 ms |
| GPIO10| Blue LED | Blue stays on while the water pump is running |
| GPIO4 | Potentiometer input | 100 ohm resistors |
| GPIO6 | Pump control | PWM to MOSFET input |
| GPIO7 | Light control | PWM to 2nd MOSFET; extr goal |
??? | GPIO? | Aquarium pump control | 3rd MOSFET |

The board also uses 3V3, GND and USB-C. The pump and LED strip get 12 V from the power supply, not from a GPIO pin. All low-voltage non-isolated control parts need a common GND. The ESP32 GPIO pins must not receive 5 V or 12 V.

## 6. Behaviour

The water control has three states: IDLE, WATERING and FAULT.

1. On power-up: set the pump and light outputs off, start USB Serial and prepare PWM. Start the first watering after setup.
2. In IDLE: keep the pump off. Start watering every 60 minutes, counted from the last start.
3. In WATERING: increase pump power over 1 second. Stop after the chosen time. The current code uses 180 seconds; the minimum project will let the knob choose 60 to 360 seconds in 10-second steps.
4. In FAULT: if the pump has been on for 420 seconds, turn the output off and keep it off until reset.
5. Water goes from the tank through the pump to the tray, then drains back to the tank. Tank has aquarium filter.
6. Stretch: run lights for 14 hours, then turn them off for 10 hours. Use 15 minutes for sunrise and 15 minutes for sunset.
7. Stretch: read the light sensor once per second and show the value on USB Serial. If light is too low while the LEDs should be on, show a warning.
8. After a power cut: start the time cycle again from power-on. There is no clock or WiFi in the minimum project.

## 7. Interfaces and protocols

| Channel | Direction | Format |
|---------|-----------|--------|
| GPIO4 / ADC | in | Potentiometer voltage; map to 60 to 360 seconds |
| GPIO6 / PWM | out | Pump power, 1000 Hz, 12-bit |
| GPIO7 / PWM | out | Light power, 1000 Hz, 12-bit; extra |
| GPIO0 / ADC | in | Light sensor value; extra |
| GPIO21 / GPIO | out | Red LED, LOW is on |
| USB Serial | out | 115200 baud, ASCII lines |
| USB Serial | in | Stretch: single letters w, p, s, h |

The current code sends these messages:

```text
[12 s] pump on for 180 s
[312 s] pump off
FAULT: pump on too long, off until reset
```

## 8. Minimum and stretch

Minimum (must work on demo day):
- The ESP32-C3 controls the 12 V pump through the MOSFET.
- Water moves from the tank to the tray and drains back safely.
- The pump starts on a 60-minute cycle and stops after the chosen time.
- The potentiometer sets the pump time from 1 to 6 minutes.
- The pump has a 1-second soft start and a 7-minute software limit.
- The red RGB led shows status.
- USB Serial shows pump start, stop and fault messages.

Stretch (nice to have):
- The 12 V LED strips run 14 hours on and 10 hours off.
- The LED brightness changes slowly at sunrise and sunset.
- A light sensor on GPIO0 checks if the lights are working.
- USB Serial keys start or stop the pump and show status.
- A low-water sensor protects the pump in a later version.

## 9. Open questions and risks

- I must check the pump label for its power and start current before buying the 12 V supply.
- I must check the MOSFET module works with 3.3 V signals and the pump current.
- I need to find out if the pump can use PWM. If not, I will use simple ON/OFF control.
- I must check if the pump module already has a flyback diode. A 1N5819 may be too small for some pumps.
- The 12 V, 4000 K strip is not a tested grow lamp. I need to test it above the tray.
- The LED strip is IP20, so it must be mounted away from water. Use an aluminum profile to remove heat.
- The aquarium filter may slow down the water flow. It is not needed for the first demo.
- The tank must not overflow, and the pump must not run dry.
- The 420-second limit is in software. It does not protect the system if the controller stops working.
- I will keep 230 V wiring inside a certified power supply. I will not build an open 230 V switch near water.
- The light sensor gives a basic light value, not a true plant-light (PPFD) reading.
- Instructor: please check my pump driver, wiring and power supply choice.
- Exact pump current and final LED distance above the tray: TBD.

## 10. Plan for the three sessions

| Session | Date | Goal at the end of the evening |
|---------|------|--------------------------------|
| 23 | Fri 2026-11-20 | Buy or collect the missing parts. Wire the ESP32, MOSFET, pump and knob. Test each part. Check water flow and return. |
| 24 | Mon 2026-11-23 | Make the minimum project work: timed watering, knob, soft start, status LED and safety limit. Test with water. |
| 25 | Fri 2026-11-27 | Give a 5-minute demo. Show water flow, turn the knob and show the pump stop. If ready, also show the lights and light sensor. |
