# Lesson 24 - From Parts to a Device

Z2: integration and getting ready for demo day, then final project work.

- Every part worked alone, together they wait: the glued `loop()`
- Measure first: passes per second and the longest `loop()` pass
- The rule: a part never waits - check the clock, do a little, return
- One part = one class with `begin()` and `update(now)`; `loop()` is a list
- What must wait (WiFi, bot) gets its own FreeRTOS task
- Two tasks, one board: a queue for commands, a copy under a lock for data
- Integrate one part at a time, commit after each
- One board, many networks: Preferences (NVS) + WiFiMulti
- Demo day: format, checklist, README

## Contents

- `slides/lesson24.pptx` - the deck
- `demo/` - the same device twice: env `before` (glued) and env `after`
  (one class per part, WiFi + bot in a task), see `demo/README.md`

## Demo day - Fri 2026-11-27

Each of you: 10 minutes demo + 5 minutes questions.

| Minute  | What                                  |
|---------|---------------------------------------|
| 0 - 1   | the idea: what problem, for whom      |
| 1 - 3   | hardware: board, parts, wiring        |
| 3 - 7   | live demo, main use case first        |
| 7 - 9   | code: one part you are proud of       |
| 9 - 10  | what went wrong, what is next         |
| 10 - 15 | questions                             |

## Homework - before Friday

In your project folder, `projects/<name>/`:

- the minimum version works from a cold start: unplug, power bank, no laptop
- the class network is saved on the board (`wifi add`) and tested in class
- `README.md` in English: what it does, pin table, how to build and run,
  how it works (parts, periods, who runs what), known problems and next steps
- on the ESP32-C3: say which strapping pins (GPIO 2, 8, 9) you use and why
  it is safe
- no WiFi password or bot token in git - `secrets.h` stays in `.gitignore`
- code frozen and pushed by Thursday evening; record a 30 s video of it
  working as plan B
- rehearse the 10 minutes once with a timer
