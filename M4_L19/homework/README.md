# Homework 19 - A clock that sleeps

One board, at home, no gateway. The board spends almost all of its time in
deep sleep and still knows the exact time.

## Start

1. Create `homework/<name>/src/` (for example `homework/sven/src/`).
2. Copy into it from `M4_L19/lab2-ntp`: `src/main.cpp` and
   `include/secrets.example.h`.
3. In your folder, copy `secrets.example.h` to `secrets.h` and fill in
   your home WiFi. `secrets.h` is in `.gitignore` - never commit it.
4. In `homework/platformio.ini` uncomment your own `src_dir` line (do not
   commit that change).
5. Build and upload. You should see lab 2 working before you change
   anything.

## What the board does

1. **Cold start** (power-on or RESET): WiFi on -> NTP -> WiFi off -> deep
   sleep. Stay awake 10 s after the cold start, so an upload can start
   (as in the labs).
2. **Timer wake every 10 minutes**: blink the green LED (GPIO 6), print
   the exact local time, sleep again. No WiFi.
3. **Button A** (GPIO 0) wakes the board at once: blink, print the time,
   stay awake 10 s. The next timer wake comes 10 minutes **after the
   press**, not at the old time.
4. **Before every sleep** print when the next wake will be.
5. **Once an hour**: on the first wake after the hour is over, turn WiFi
   on, sync with NTP again and print how far the RTC clock had drifted
   since the last sync, in milliseconds. Then WiFi off again.

Example (your format may differ, the content may not):

```
2026-11-08 21:40:00  #37  timer   next wake 21:50:00
2026-11-08 21:43:12  #38  button  next wake 21:53:12
2026-11-08 21:53:12  #39  timer   resync: drift -412 ms in 63 min
                                  next wake 22:03:12
```

## Rules

- The wake interval (10 min) and the resync period (60 min) are named
  constants at the top of the file. While you develop, set them to 1 min
  and 5 min - nobody waits an hour to test a branch.
- Everything that must survive deep sleep is in `RTC_DATA_ATTR` variables.
  Nothing else survives.
- The TZ setting is set again on every wake (lab 2 shows why).
- The LED pins stay held HIGH during sleep (lab 1 shows how).
- A failed resync (no WiFi, no NTP answer) is printed once and does not
  stop the clock. Try again on the next wake after another full period,
  not on every wake.
- `uint32_t` / `int32_t` and friends, `%lu` with `static_cast<unsigned long>`.

## Hints

- The drift is "what the RTC said" minus "what NTP says", at the same
  moment. Read the RTC time with `gettimeofday()` just before the sync.
  The sync itself takes time - account for it with `millis()`.
- Trap: `getLocalTime()` only checks that the year looks plausible. After
  the first sync the RTC year is always plausible, so it returns at once,
  **before** the new answer has arrived. Wait for
  `sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED` instead
  (`#include <esp_sntp.h>`), with a timeout.
- `esp_sleep_enable_timer_wakeup()` takes microseconds as `uint64_t`.
  10 minutes = 600000000 us still fits in 32 bits, 2 hours does not.
  Write the constants with `ULL` so a longer interval never overflows.

## Hand-in

1. Let the board run overnight with the real constants (10 min / 60 min).
2. Commit `homework/<name>/src` (without `secrets.h`) and push.
3. In the commit message: what works and what does not, and the resync
   lines from the night (at least five). Is the drift always the same
   sign? About how many seconds per day is that?
