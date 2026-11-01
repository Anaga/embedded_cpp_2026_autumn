# Homework 18 - RGB colour from the web

Lab 1 switches the green LED on and off. Now the page sets any colour.

## Start

1. Create `homework/<name>/src/` (for example `homework/sven/src/`).
2. Copy into it from `M4_L18/lab1-web`: `src/main.cpp`,
   `include/board.h` and `include/secrets.example.h`.
3. In your folder, copy `secrets.example.h` to `secrets.h` and fill in
   your home WiFi. `secrets.h` is in `.gitignore` - never commit it.
4. In `homework/platformio.ini` uncomment your own `src_dir` line (do not
   commit that change).
5. Build and upload. You should see lab 1 working before you change
   anything.

## Task 1 - PWM colour

Use your `RgbLed` class from K3 (or write a small one): PWM on GPIO 5 / 6 / 7,
common anode, so 255 = full on means duty 0.

- Core 2.x PWM: `ledcSetup(channel, freq, bits)`, `ledcAttachPin(pin,
  channel)`, `ledcWrite(channel, duty)`. Not `ledcAttach` - that is core 3.x.
- The colour lives in one place, for example `struct Colour { uint8_t r, g, b; }`.
- `POST /led` keeps working: on = green `(0, 255, 0)`, off = `(0, 0, 0)`.

## Task 2 - POST /rgb

```
curl -X POST -H 'Content-Type: application/json' \
     -d '{"r":255,"g":0,"b":128}' http://esp-xxxx.local/rgb
```

- All three fields must be present and be integers from 0 to 255.
- Anything else -> `400` with a short text that says what is wrong
  (`"r missing"`, `"g out of range"`, ...). Test at least three bad inputs.
- On success the answer is the status JSON (as `/led` does now).
- `/status` gets the colour:

```
{"id":"esp-9494", ..., "rgb":{"r":255,"g":0,"b":128}}
```

Hint: `doc["r"].is<int>()` checks the type, then check the range yourself.

## Task 3 - three sliders on the page

- Three `<input type="range" min="0" max="255">`, one per channel.
- Send `POST /rgb` when a slider is released (`onchange`), not on every
  pixel of movement (`oninput` would flood the board with requests).
- A small square on the page shows the current colour
  (`style.background = 'rgb(r, g, b)'`).
- The page still polls `/status` every 2 s: if the colour is changed with
  curl, the sliders follow.

## Extra - MQTT

Take the MQTT part from `M4_L18/lab2-mqtt`. Subscribe to
`gamma-ed/2026/<id>/rgb`; the payload is the same JSON as for `POST /rgb`.
Bad payload -> one line on Serial, no change.

```
mosquitto_pub -h broker.hivemq.com -t 'gamma-ed/2026/esp-xxxx/rgb' -m '{"r":0,"g":0,"b":255}'
```

## Rules

- No `delay()` in `loop()`.
- Limits (0, 255) and the PWM frequency and resolution are named constants.
- One function decides the colour (`rgbSet(const Colour&)`); the web
  handler, `/led` and MQTT only call it.

## Hand-in

Commit `homework/<name>/src` (without `secrets.h`) and push. In the commit
message: what works and what does not. Paste the output of your three bad
`curl` calls into a comment at the top of `main.cpp`.
