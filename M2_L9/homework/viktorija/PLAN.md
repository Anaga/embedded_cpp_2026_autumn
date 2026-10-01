# Lesson 09 homework - plan

Small steps. Test each one on the board before moving on.

## Part A: build the parts

- [x] **1. Upgrade the LED class**
  Copy `Colour.h`, `RgbLed.h`, `RgbLed.cpp` from lesson 08.
  Change `setColour(Colour c)` to `setColour(const Colour &c)`.
  Add the overload `setColour(uint8_t red, uint8_t green, uint8_t blue)`.
  *Test:* in `setup()`, show red, green, blue and white, using both versions.

- [x] **2. Button class**
  Turn the lesson 07 `isButtonPressed()` into `Button.h` / `Button.cpp`.
  The two `static` variables become fields.
  *Test:* print "A" / "B" on each press. Hold A, press B: B is still counted.

- [x] **3. Reference function**
  Something like `addPoint(uint8_t &score)`.
  *Test:* A adds a point to player 1, B to player 2. Print the score.

## Part B: the game

- [x] **4. Paper first**
  Draw the states as boxes and the events as arrows. Check it before coding.

- [x] **5. State machine, no buttons yet**
  Off for a random pause, then green, then the result, then the next round.
  `millis()` only, no `delay()`. Print "Round N: get ready..." and "GO!".

- [x] **6. Pressing after green**
  Winner, reaction time (measured from green), draw, score.

- [x] **7. False start**
  A press while the LED is still off.

- [x] **8. The hard test**
  Hold A down from the very start. Expect one false start, not one every round.

## Star task (optional)

- [x] **9. First to five**
  When a score reaches 5: print who won the match, reset, start a new match.
- [x] **10. Celebration**
  New state: fade up and down in the winner's colour, `millis()` only.
  A button press interrupts it.
