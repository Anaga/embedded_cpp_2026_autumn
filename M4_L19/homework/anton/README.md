# Homework 19 - A clock that sleeps

One board, at home, no gateway. The board spends almost all of its time in
deep sleep and still knows the exact time.

## What it does

- **Cold start**: WiFi on -> NTP -> WiFi off -> deep sleep. Stay awake 30 s.
- **Timer wake every 10 min**: blink green LED (GPIO 6), print exact local time,
  sleep again.
- **Button A (GPIO 0)**: wakes immediately; next timer wake comes 10 min after
  press, not at old time.
- **Before every sleep**: print when the next wake will be.
- **Once an hour**: on the first wake after the hour, turn WiFi on, sync with
  NTP again, print drift in ms since last sync. WiFi off again.

## Rules

- The 10 min interval and 60 min resync are named constants (lower them when
  developing).
- Everything that must survive deep sleep is `RTC_DATA_ATTR`. Nothing else.
- The TZ setting is set again on every wake.
- The LED pins stay held HIGH during sleep.
- A failed resync is printed once and does not stop the clock.

## Build

```bash
cd homework/anton M4_L19
pio run
pio run -t upload
```

## Hand-in

1. Run overnight with real constants.
2. Commit `homework/anton/M4_L19` and push.
3. In the commit message: what works and what does not, and the resync lines
   from the night.