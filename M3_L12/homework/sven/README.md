# Two-board setup

The Pico reads the potentiometer; the ESP32-C3 Mini controls the LED.

Before building or uploading, enable the matching pair in the root
[platformio.ini](../platformio.ini) and comment out the other pair:

```ini
; Pico
extra_configs = sven/src_pico/platformio.ini
src_dir = sven/src_pico

; Mini
;extra_configs = sven/src_mini/platformio.ini
;src_dir = sven/src_mini
```

Both boards reuse the `pico` environment to minimize changes to the provided
configuration files. Normally, each board would have its own environment name.

For either board: `pio run -e pico` to build, or
`pio run -e pico -t upload` to build and upload to the selected board.
