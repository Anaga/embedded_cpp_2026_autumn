# Lesson 08 - Structs, Classes and PWM

## What this lesson covers

- Structs: several values that belong together, under one name
- Structs are values: assigning one makes a copy
- The first class: data plus the things you can do with it
- `public` and `private`, the constructor, and why hardware waits for `begin()`
- PWM: how a pin that can only be on or off produces any brightness
- PWM on the ESP32-C3 with `ledcAttach()` and `ledcWrite()`
- Why a class pays off: the inside changes, the calls do not

## Files

```
lesson-08/
  slides.pptx
  demo/               the program shown in class, see demo/README.md
  homework/           the home task, see homework/README.md
  README.md           this file
```

## Home task

A colour wheel on the RGB LED: the potentiometer picks the hue, the LED shows
it. The full specification is in `homework/README.md`.
