# Lesson 10 - States, Templates and Interrupts

## What this lesson covers

- State machines: one state at a time, events move the program on
- `enum class`: names for the states that cannot be mixed up with numbers
- What we leave out on a microcontroller, and why: the heap (`new`, `String`,
  `std::vector`), exceptions, RTTI
- Templates: a function or a class written once for any type
- `RingBuffer<T, N>`: a history of values with its size fixed at compile time
- A moving average that smooths the potentiometer
- Interrupts: `attachInterrupt`, `IRAM_ATTR`, `volatile`, and the rules for a
  handler

This lesson closes the C and C++ block. From the next lesson on, the course
turns to the chip's peripherals.

## Files

```
lesson-10/
  slides.pptx
  demo/               the program shown in class, see demo/README.md
  homework/           the home task, see homework/README.md
  README.md           this file
```

## Home task

The reaction game from lesson 09 gets a memory: the last 10 reaction times of
each player, with the best and the average. The full specification is in
`homework/README.md`.
