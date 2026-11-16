# Lesson 22 - One Picture, Three Chips

Z1: cross-platform comparison and portability.

- The chips used in this course: ESP32-C3, RP2040, RP2350, STM32F401
- The same task through Arduino-ESP32, Arduino-Pico and STM32 HAL
- Layers: application, drivers, framework / HAL, registers
- WS2812B 8x8 panels: one wire, pulse-width bits, RMT vs PIO vs timer + DMA
- Board configuration in one place: `LED_DATA_PIN` in `platformio.ini`,
  the panel wiring in `xy()`
- Final project approval

## Contents

- `slides.pptx` - the deck
- `demo/` - the panel demo for `c3`, `pico` and `pico2`, see `demo/README.md`

## Homework

Start your final project in `students/<username>/final-project/`:

- a PlatformIO project that builds for your board
- `README.md` with what the device does and a pin table
  (on the ESP32-C3: GPIO 2, 8 and 9 are strapping pins)
- commit before session 23 (Fri 2026-11-20)
