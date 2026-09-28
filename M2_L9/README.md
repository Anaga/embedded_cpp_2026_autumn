# Lesson 09 - References, Overloading and a Button Class

## What this lesson covers

- References: a second name for a variable that already exists
- Copy, pointer or reference: three ways to hand a value to a function
- `const &`: no copy and no changes, the everyday way to pass anything bigger
  than a number
- Overloading: one name, several versions, chosen by the arguments
- Why a function with `static` variables cannot serve two buttons
- A `Button` class: every object keeps its own memory

## Files

```
lesson-09/
  slides.pptx
  demo/               the program shown in class, see demo/README.md
  homework/           the home task, see homework/README.md
  README.md           this file
```

## Home task

A reaction game for two players: the LED turns green after a random pause,
and the faster button wins. The full specification is in `homework/README.md`.
