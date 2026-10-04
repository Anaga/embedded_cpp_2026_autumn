# Lesson 10 homework - plan

Small steps. Test each one on the board before moving on.
Starting point: my lesson 09 game (copied here, works).

## Part A: setup

- [x] **0. Build the copy as it is**
  In `platformio.ini`, switch on `src_dir = viktorija/src` (local only, don't commit it).
  Change the header comment in `main.cpp` to lesson 10.
  *Test:* the game plays exactly like in lesson 09.

- [x] **1. enum class for the states**
  Replace the four `STATE_...` constants with `enum class State : uint8_t`.
  `current_state` becomes a `State`, the cases become `State::Wait` etc.
  *Test:* the game plays the same. Try `current_state = 1U;`: it should no longer compile.

## Part B: the history parts

- [x] **2. RingBuffer**
  Copy `RingBuffer.h` from `M2_L10/demo/include/` into `src/`. Include it with quotes.
  *Test:* in `setup()`, push 1..12 into a `RingBuffer<uint16_t, 10>`, print with `at()`.
  Expect `3 4 5 ... 12`. Remove the test afterwards.

- [x] **3. bestTime() and averageTime()**
  Exactly these signatures:
  `uint16_t bestTime(const RingBuffer<uint16_t, 10> &history);`
  `uint16_t averageTime(const RingBuffer<uint16_t, 10> &history);`
  Read values with `at()`, never the internal array.
  Sum in a `uint32_t`. Empty history: no division by zero.
  *Test:* in `setup()`, a known list (e.g. 300 200 250): best 200, avg 250.
  Then an empty buffer: no crash.

- [x] **4. Print one player's history**
  Something like `printHistory(const char *name, const RingBuffer<uint16_t, 10> &history)`.
  Format: `last N: ...   best X   avg Y`. Empty: `-` for best and avg.
  *Test:* call it at startup for both players: two lines with `-`.

## Part C: wire it into the game

- [x] **5. Two histories, push on a win**
  One `RingBuffer<uint16_t, 10>` per player.
  In the GO state, the winner's reaction time goes into their history.
  `reaction_time` is `uint32_t`: think about what happens when it doesn't fit into `uint16_t`.
  *Test:* win three rounds as player 1. Player 1 shows `last 3`, player 2 still `-`.

- [x] **6. Draw goes into both**
  *Test:* press both together after green. The same time appears in both lists.

- [x] **7. False start stores nothing**
  Should already be true. Check it, don't assume it.
  *Test:* false start, single and double. Neither list grows.

- [x] **8. Summary after every round**
  Winner + time, score, then the two history lines (see README example).
  Decide: print it when the round is decided, or when RESULT ends?
  *Test:* it shows after wins, draws and false starts.

- [x] **9. New match: what happens to the histories?**
  `celebrate()` resets scores. The README says nothing about histories.
  Decide: keep them or `clear()` them. Write the reason in a comment.

## Part D: check and submit

- [x] **10. README checklist**
  Run every row of the "How to check yourself" table.
  Twelve wins by one player: `last 10`, the two oldest gone, order still oldest to newest.
  Best is the smallest number actually shown.

- [x] **11. No heap**
  Search for `new`, `String`, `std::vector`, `malloc`. None allowed.
  Still no `delay()` while a round runs (the one in `setup()` is fine).

- [ ] **12. Commit**
  `git add viktorija/` only, not `platformio.ini`.
  Message: `Lesson 10: reaction game history`.

## Star task (optional): interrupt timestamps

- [ ] **13. One interrupt per button**
  `IRAM_ATTR` handler: if the flag isn't set yet, save `micros()` and set the flag.
  Only the first edge counts, so bounces are ignored.
  Shared variables are `volatile`. No `Serial`, no waiting.
  `attachInterrupt(digitalPinToInterrupt(pin), handler, FALLING)`.
  *Test:* print the timestamp from `loop()` on each press. One press, one value.

- [ ] **14. Measure with the timestamps**
  Save `micros()` when green turns on.
  Clear both flags when going to GO, so a false-start press doesn't leak in.
  The `Button` class still decides *whether* it was a press. The interrupt gives *when*.
  *Test:* print `231.4 ms`: tenths = us / 100, then `%u.%u` with `/ 10` and `% 10`.

- [ ] **15. History in tenths?**
  Decide: keep whole ms, or store tenths (up to 6553.5 ms fits in `uint16_t`).
