# Homework 18 - RGB colour from the web

Lab 1 switches the green LED on and off. Now the page sets any colour.

## Tasks

1. **PWM colour**: Use an RgbLed class (or small implementation) with PWM on
   GPIO 5/6/7, common anode, 255 = full on means duty 0.
2. **POST /rgb**: Handle a POST with JSON {"r":255,"g":0,"b":128}. Validate all
   three fields are integers 0–255; return 400 with error text otherwise.
3. **Three sliders**: On the web page, three `<input type="range" min="0" max="255">`.
   Send POST /rgb on change (onchange, not oninput). A small square shows the
   current colour.

## Rules

- No `delay()` in `loop()`.
- Limits, PWM frequency, resolution are named constants.
- One function decides the colour (`rgbSet(const Colour&)`); the web handler,
  `/led` and MQTT only call it.

## Build

```bash
cd homework/anton M4_L18
pio run
pio run -t upload
```

## Hand-in

Commit `homework/anton/M4_L18` and push. Paste three bad curl outputs into a
comment at the top of `main.cpp`.