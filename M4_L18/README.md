# Lesson 18 - WiFi and a web server (ESP32-C3)

Back on the ESP32-C3. The board joins WiFi, reads the AHT10, serves a web
page and a small JSON API - and in lab 2 also talks MQTT.

| Folder | What |
|---|---|
| `slides/` | the deck (`lesson18.pptx`) and its generator |
| `lab1-web/` | AHT10 + WiFi + local web server: page, `/status`, `POST /led` |
| `lab2-mqtt/` | lab 1 + MQTT: publish the status, LED commands by topic |
| `homework/` | the task: RGB colour from the web (`homework/README.md`) |

## Before you start

- Wiring on the C3: AHT10 SDA -> GPIO 8, SCL -> GPIO 9, VIN -> 3V3. RGB LED
  stays on GPIO 5 / 6 / 7 as in K3.
- In each project copy `include/secrets.example.h` to `include/secrets.h`
  and fill in the class WiFi (on the whiteboard). Never commit `secrets.h`.
- macOS: allow the browser and the terminal under System Settings ->
  Privacy & Security -> Local Network, or they cannot reach the board.

Details, expected output and troubleshooting: the README in each lab folder.
