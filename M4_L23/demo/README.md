# Lesson 23 demo - a Telegram bot on the ESP32-C3

Your board in your pocket: commands from the Telegram app on your phone,
and a message from the board when the room gets too warm. Works from any
network - the board only makes outgoing HTTPS connections, so no port
forwarding and no broker.

## What it does

| You send            | The board answers                                   |
|---------------------|-----------------------------------------------------|
| `/start` or `/help` | help text and a keyboard; this chat gets the alerts |
| `/status`           | temperature, humidity, LED, alert state, WiFi       |
| `/led on`           | green channel of the RGB LED (GPIO 6) on            |
| `/led off`          | ... off                                             |
| `/alert`            | the alert threshold                                 |
| `/alert 26.5`       | sets the threshold to 26.5 C                        |

Push alert: when the temperature reaches the threshold, the board writes
first - once. "Back to normal" comes when it drops 0.5 C below the
threshold (a band, as in the session 15 thermostat). Breathe on the AHT10
to test it: set `/alert` about 1 C above the current reading first.

## Wiring

Nothing new. Same as sessions 18-21:

| Part        | ESP32-C3 pin                                  |
|-------------|-----------------------------------------------|
| AHT10       | SDA GPIO 8, SCL GPIO 9, VIN 3V3, GND           |
| RGB LED     | R / G / B on GPIO 5 / 6 / 7, common anode to 3V3 |

GPIO 8 and GPIO 9 are strapping pins: do not press BOOT (GPIO 9) during a
reading, and nothing may hold SDA / SCL low at reset.

`/led` switches the green channel of the RGB LED, not the blue on-board LED.
The on-board LED sits on GPIO 8, which is I2C SDA: while the AHT10 is read it
only flickers with the I2C traffic and cannot be switched.

## 1. Create your bot (on the phone, 3 minutes)

1. In Telegram open **@BotFather** (blue check mark) and send `/newbot`.
2. Name: anything, for example `Anton C3`.
3. Username: must end in `bot` and be unique, for example `anton_c3_2026_bot`.
4. BotFather answers with a **token** like `123456789:AAH...`.
   That token IS your bot. Do not post it in a chat, do not commit it.

Quick check without the board, in any browser:
`https://api.telegram.org/bot<TOKEN>/getMe` -> `"ok":true` and your bot name.

## 2. Build and run

1. Copy `include/secrets.example.h` to `include/secrets.h`.
2. Fill in `WIFI_SSID`, `WIFI_PASS` and `BOT_TOKEN`.
3. Build, upload, open the monitor:

```
pio run -t upload
pio device monitor
```

4. Wait for `Bot: @your_bot is online`, open the bot in Telegram, press
   **Start**.

Serial shows every message with its chat id:

```
chat 123456789 <- /status
  -> esp-aab8 | 26.3 C, 35.1 % RH | Green LED off | ...
```

Copy that number into `ALERT_CHAT_ID` if you want alerts right after a
reboot without sending `/start` again.

## Things to notice

- **The poll blocks `loop()`.** The status line prints for example `poll
  590 ms (max 6399 ms)`: that is how long `getUpdates()` held the loop.
  Measured on a C3 at -70 dBm: about 0.6 s or about 2 s per poll (the slow
  ones open a new HTTPS connection), once 6.4 s. Fine for a
  sensor read every 2 s. Not fine for a button, a motor or LED animation -
  those need their own timing, or the bot goes on the other core / another
  task.
- **Groups work too.** Add the bot to a group: the chat id is negative and
  commands arrive as `/status@your_bot` - `splitCommand()` cuts the `@` part.
- **Anyone can find your bot** by its username and send it commands. For a
  real device check the sender:
  `if (m.chat_id != OWNER_CHAT_ID) return;`
- **`setInsecure()`**: the connection is encrypted, but the board does not
  check WHO it is talking to. On a hostile network someone could pretend to
  be api.telegram.org and read your token. A real product loads the root
  certificate with `tls.setCACert(...)` instead.
- **One alert per crossing.** Without the HOT / NORMAL states the board
  would send a message every 2 s - and Telegram starts refusing after about
  one message per second per chat.

## Things to try

- Add `/rgb 255 0 128` (the parser in `handleAlert()` shows how to read a number).
- Send `/status` every hour by itself, at a full hour (NTP from session 19).
- Turn the alert on and off from the keyboard: `/alert off`.

## Troubleshooting

| Serial says                         | Look at                                          |
|-------------------------------------|--------------------------------------------------|
| `include/secrets.h is missing`      | step 2.1                                         |
| `WiFi: connecting` forever          | 2.4 GHz only; `LIMIT_TX_POWER` in `board.h`      |
| `getMe failed`                      | token copied completely? The `getMe` URL above   |
| online, but no answer to messages   | another board runs with the same token - only one may poll |
| `send FAILED` now and then          | weak WiFi; the alert is retried with the next reading |

Examples found online are often for Arduino core 3.x or ArduinoJson 7.
This project is core 2.x with ArduinoJson 6 - translate, do not paste.
