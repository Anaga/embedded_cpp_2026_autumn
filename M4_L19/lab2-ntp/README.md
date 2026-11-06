# Lab 2 - NTP once, then sleep

On a cold start the board joins WiFi, gets the time from an NTP server,
turns WiFi off and goes to deep sleep. On every wake it prints the exact
local time from the RTC, with no WiFi at all.

## Before the first build

Copy `include/secrets.example.h` to `include/secrets.h` and fill in the
class WiFi (on the whiteboard). Never commit `secrets.h`.

Wiring: as in lab 1 (button A on GPIO 0, RGB LED on 5 / 6 / 7).

## Run

Expected output (your numbers differ):

```
Lesson 19 - Lab 2 - NTP once, then sleep
   WiFi 1840 ms, NTP 212 ms
   awake 10 s - upload now
2026-11-06 19:12:15  #1  cold    awake 2068 ms (+12000 waiting)
2026-11-06 19:12:37  #2  timer   awake 3 ms (+2000 waiting)
2026-11-06 19:12:59  #3  timer   awake 3 ms (+2000 waiting)
```

- **awake** is what the board really needs for its job.
- **waiting** is only for us: the pause for the USB monitor and the
  10 s upload window. A battery device would not spend it.
- `millis()` starts after the bootloader, so the real wake is a little
  longer than printed.

## Write down two numbers

1. awake on the cold start (WiFi + NTP)
2. awake on a timer wake

They go on the whiteboard. Part 3 of the lesson starts from them.

## Things to try

- Comment out `setenv("TZ", ...)` and `tzset()` in `setup()`. What time is
  printed after the first wake? Why only after a wake, not on the cold start?
- Turn the router off and press RESET. What happens on the cold start? Then
  press button A after the router is back.
- Change `SLEEP_S` to 60 and leave the board for 15 minutes. Compare the
  printed time with your phone.

## Troubleshooting

- **"no connection" on the cold start.** 2.4 GHz only. Check `secrets.h`.
  Some Super Mini boards need the reduced `TX_POWER`; it is already set.
- **"NTP ... no answer".** The network may block UDP port 123. Try the
  class router, or a phone hotspot.
- **The time is 2 or 3 hours off.** The TZ setting is missing on that wake.
