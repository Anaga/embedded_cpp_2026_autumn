# Lesson 10 - Home Task: The Reaction Game Remembers

There is no starting code. You extend your own reaction game from lesson 09.
If it does not work yet, finish it first: this task builds on top of it.

## What changes

The game keeps a history of the last 10 winning reaction times for each
player, and after every round it shows the history, the best time and the
average.

```
Round 12: get ready...
GO!
Player 1 wins in 231 ms
Score: player 1 - 7, player 2 - 4

Player 1   last 10: 312 287 254 301 240 263 229 275 248 231   best 229   avg 264
Player 2   last 4:  298 276 341 259                           best 259   avg 293
```

Rules for the history:

- When a player wins a round, their reaction time goes into their history.
- A draw puts the time into both histories.
- A false start puts nothing anywhere: there is no reaction time to store.
- Once a history holds 10 times, each new time pushes out the oldest one.
- A player with no times yet shows `-` for best and average.

## Requirements

Your program must:

- keep each player's history in its own `RingBuffer<uint16_t, 10>`; you can
  copy `RingBuffer.h` from the demo of this lesson into your `src` folder, or
  write your own with the same idea
- compute the best and the average with two functions of exactly this shape,
  each taking the history by const reference:

  ```cpp
  uint16_t bestTime(const RingBuffer<uint16_t, 10> &history);
  uint16_t averageTime(const RingBuffer<uint16_t, 10> &history);
  ```

- name the states of the game with an `enum class` and run them from a
  `switch` in `loop()`; if your lesson 09 version used plain numbers or a pile
  of `bool` flags, rework it now
- use no heap at all: no `new`, no `String`, no `std::vector`
- still use your `Button` and `RgbLed` classes, and still no `delay()` while a
  round runs
- use fixed-width types, named constants and ASCII, as always

The average is a whole number: dividing the sum by the count and dropping the
fraction is fine.

## How to check yourself

| Try this | Expected |
|---|---|
| Fresh start, look at the first summary | `-` for both players, no crash, no division by zero |
| Win three rounds as player 1 | player 1 shows `last 3`, player 2 still `-` |
| Win twelve rounds as one player | `last 10`; the two oldest times are gone |
| Make a false start | no new time in either history |
| Compare best with the list | best is the smallest number actually shown |

## Star task: timestamps from an interrupt

Optional. Attach an interrupt to each button and record `micros()` in the
handler at the moment of the press. Measure the reaction time from those
timestamps instead of from the moment `loop()` noticed, and print it with one
digit after the point:

```
Player 1 wins in 231.4 ms
```

The rules from the slides apply: the handler only notes the time and sets a
flag, shared variables are `volatile`, the handler is `IRAM_ATTR`, and
nothing in it prints or waits.

Bounce does not go away. The handler fires on every bounce; make sure only
the first edge of a press counts. Your `Button` class still decides what is a
press.

The history can stay in whole milliseconds, or you can switch it to tenths:
`RingBuffer<uint16_t, 10>` holds up to 6553.5 ms in tenths, which is plenty.

## Where your code goes

Your files go in `<your-name>/src/` in this folder, headers included. In
`platformio.ini` next to this README, uncomment the one `src_dir` line with
your name. Change that line locally; do not commit `platformio.ini`.

## What to submit

```
git add .
git commit -m "Lesson 10: reaction game history"
git push
```

Deadline: before the next session.

## Common problems

**The first summary crashes the board or prints nonsense.** `averageTime()`
divides by `count()`, and `count()` is still 0. Check for an empty history
before dividing.

**Both histories show the same numbers.** There is only one `RingBuffer`, or
both players push into the same one.

**The average is wrong once there are 10 times.** The sum is kept in a
`uint16_t`. Ten times of a few hundred fit, but ten slow ones do not: add them
up in a `uint32_t`.

**After 10 rounds the list is in the wrong order.** The loop reads the
internal array from index 0 instead of using `at()`. `at(0)` is the oldest
value, wherever it sits in the array.

**`RingBuffer.h: No such file or directory`.** The header is not in your own
`src` folder next to `main.cpp`, or it is included with `< >` instead of
quotes.
