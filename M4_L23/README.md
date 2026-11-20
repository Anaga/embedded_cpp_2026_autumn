# Lesson 23 - Your Board in Your Pocket

Z2: Telegram bots on the ESP32-C3, then final project work.

- Three ways to reach a board: web server (18), MQTT (20), Telegram bot
- How a bot works: the board polls api.telegram.org over HTTPS, no open port
- @BotFather and the bot token - a password: `secrets.h`, `.gitignore`, `/revoke`
- The Bot API in two calls: `getUpdates` with `offset`, `sendMessage`
- UniversalTelegramBot on core 2.x: commands, reply keyboard, group chats
- Push alert once per crossing, with a 0.5 C band (the session 15 thermostat)
- What it costs: the poll blocks `loop()` for 0.6-2 s, sometimes more
- Final project work: questions, wiring, code

## Contents

- `slides/lesson23.pptx` - the deck
- `demo/` - the Telegram bot: `/status`, `/led on|off`, `/alert`, see
  `demo/README.md`

`/led` switches the green channel of the RGB LED (GPIO 6). The blue on-board
LED is on GPIO 8 = I2C SDA and only flickers with the AHT10 traffic.

## Homework

Your final project, in `students/<username>/final-project/`:

- session 24, Mon 2026-11-23: the minimum version works
- README with what the device does and a pin table
  (on the ESP32-C3: GPIO 2, 8 and 9 are strapping pins)
- commit and push before session 24
- a bot only if it is in your proposal, or your minimum version already works
- never commit a WiFi password or a bot token
