# Lesson 09 - Home Task: The Reaction Game

There is no starting code. You write the whole program. This README is the
specification.

## The game

Two players, one button each, one LED between them.

1. The LED is off. Both players wait with a finger on their button.
2. After a random pause of 2 to 5 seconds, the LED turns green.
3. The first player to press after green wins the round. The LED shows the
   winner's colour: red for player 1, blue for player 2.
4. Pressing before green is a false start: that player loses the round, and
   the other one wins it.
5. If both presses are counted in the same pass of `loop()`, the round is a
   draw and the LED turns white.
6. After two seconds of showing the result, the LED goes off and the next
   round starts.

The serial monitor tells the story of every round:

```
Round 3: get ready...
GO!
Player 2 wins in 287 ms
Score: player 1 - 1, player 2 - 2

Round 4: get ready...
False start by player 1
Player 2 wins
Score: player 1 - 1, player 2 - 3
```

## Wiring

| Part | Connects to |
|---|---|
| Button A, player 1 | GPIO 0 and GND |
| Button B, player 2 | GPIO 1 and GND |
| RGB LED, longest pin | 3V3 |
| RGB LED, red / green / blue | GPIO 5 / 6 / 7, each through 100 ohm |

## Requirements

Your program must:

- read the buttons through a `Button` class, one object per button, each with
  its own debounce memory; no `static` variables inside the button logic
- drive the LED through your `RgbLed` class, which offers both
  `setColour(const Colour &c)` and `setColour(uint8_t red, uint8_t green, uint8_t blue)`
- have at least one function of your own that takes a parameter by reference
  and changes it, for example to add a point to a player's score
- **not use `delay()` while a round is running.** During the random pause the
  program must keep watching both buttons, or it cannot see a false start.
  Use `millis()` to know when the pause is over
- use fixed-width types, named constants and ASCII, as always

The random pause comes from `random()`:

```cpp
const uint32_t pause_ms = (uint32_t)random(2000, 5001);   // 2000 to 5000
```

On the ESP32 `random()` draws from a hardware generator, so no `randomSeed()`
is needed. The upper limit is not included, which is why it says 5001.

## How to think about it

The game is always in exactly one of a few situations: waiting for green,
green and waiting for a press, showing the result. Give each situation a name,
keep the current one in a variable, and let `loop()` ask "where are we, and
what can happen next?" on every pass.

Before writing code, draw the situations as boxes on paper, and the events
that move the game from one box to the next as arrows. If the drawing is
right, the code is mostly typing.

## How to check yourself

| Try this | Expected |
|---|---|
| Wait for green, press A | red, player 1 wins, a time in ms |
| Wait for green, press B | blue, player 2 wins |
| Press A while the LED is still off | false start by player 1, player 2 wins |
| Press both at once after green | usually one wins by a few ms; rarely a draw |
| Play five rounds | the pause is different every time |
| Hold A down from the start | one false start, not a new one every round |

The last line is the hard one. Think about what `wasPressed()` returns for a
button that was already down when the round started.

## Star task: first to five

Optional. The game becomes a match: the first player to win five rounds wins
the match. Celebrate with an animation in the winner's colour, for example a
slow fade up and down with PWM, and then start a new match.

The animation must not use `delay()` either, so that it can be interrupted.

## Where your code goes

Your files go in `<your-name>/src/` in this folder, headers included. In
`platformio.ini` next to this README, uncomment the one `src_dir` line with
your name. Change that line locally; do not commit `platformio.ini`.

## What to submit

```
git add .
git commit -m "Lesson 09: reaction game"
git push
```

Deadline: before the next session.

## Common problems

**A false start is never detected.** The random pause is a `delay()`. While
`delay()` runs, nothing else in `loop()` does, and the button is not looked at.

**Pressing B while A is held does nothing.** The button logic still has shared
`static` variables. See the demo of this lesson.

**One player always wins a draw.** Both buttons are read, and the first one
checked wins before the second is even looked at. Read both first, then
decide.

**The reaction time is huge, like 4000 ms.** The time is measured from the
start of the round, not from the moment the LED turned green.

**`random()` gives the same pause every round.** Check that you call it at
the start of each round, not once in `setup()`.
