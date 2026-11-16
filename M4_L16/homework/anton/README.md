# Homework 16 - UART commands, non-blocking I/O

This homework adds a UART (USB CDC) command interface with full non-blocking I/O.
No `delay()` anywhere — all timing uses `millis()`.

## Features

- **Line reader**: bytes are collected until `\n`, then parsed
- **Command dispatch**: `help`, `id`, `led on|off|status`, `sensor`, `time`
- **Heartbeat**: LED blinks every second (non-blocking)
- **All timing via `millis()`**: no blocking delays

## Build

```bash
cd homework/anton M4_L16
pio run
pio run -t upload
```

## Hand-in

Commit `homework/anton/M4_L16` and push.