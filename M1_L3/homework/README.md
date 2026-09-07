# Lesson 03 - Homework

Ohm's law, resistor selection and power.

No hardware required. Paper and a calculator are enough. Show your working -
the number alone is worth less than the reasoning that produced it.

Answer format: one number per problem, with units. Write your answers below
each problem in this file, commit and push.

Useful formulas:

```
V = I * R
P = V * I
```

Remember that current in these problems is in milliamps, and Ohm's law needs
amps. 10 mA = 0.010 A.

---

## Problem 1 - Resistor for a red LED

An ESP32-C3 GPIO pin outputs 3.3 V.

You want to drive a red LED from it. The LED has a forward voltage of 2.0 V,
and you want 10 mA flowing through it.

The LED and the resistor are in series, so the resistor gets whatever voltage
the LED does not use.

**What resistor value do you need?**

Answer:

---

## Problem 2 - Resistor for a blue LED

Same 3.3 V GPIO pin. This time a blue LED with a forward voltage of 3.0 V, and
you want 15 mA through it.

**What resistor value do you need?**

Answer:

Look at how much smaller this answer is than the one in problem 1, and think
about why. This is the practical consequence of blue LEDs having a high forward
voltage: on a 3.3 V supply there is almost nothing left over.

---

## Problem 3 - Power in the resistor

Take the resistor value you calculated in problem 1, carrying the same 10 mA.

Some of the energy passing through a resistor becomes heat. How much depends on
the current and the resistance:

```
P = I * I * R
```

**How much power is dissipated in that resistor?**

Answer:

A standard through-hole resistor is usually rated for 0.25 W. Compare your
answer to that rating and decide whether the resistor is safe to use.

---

## Problem 4 - From measurement back to theory

In class you measured current through a 1 kohm resistor. Suppose the
measurement was 3.2 mA at 3.3 V.

**What is the actual resistance, calculated from that measurement?**

Answer:

Compare it to the nominal 1000 ohm. Common resistors have a 5 percent
tolerance, which means anything from 950 to 1050 ohm is a resistor behaving
exactly as specified. Decide whether your calculated value falls inside that
range, and if it does not, think about where else the error could come from.

Then repeat the calculation with the numbers you actually measured in class.

---

## Problem 5 - Pull-up resistor sizing

Harder. Attempt it after the first four.

A pull-up resistor connects a GPIO input to 3.3 V. When a button is pressed and
pulls that pin down to GND, current flows through the pull-up resistor
continuously, for as long as the button is held. On a battery powered device
that wasted current matters.

You want that current to stay below 0.5 mA.

**What is the minimum pull-up resistor value that satisfies this?**

Answer:

There is a limit at the other end too. Making the pull-up very large reduces the
wasted current further, but the pin becomes slow to return to HIGH and more
easily disturbed by electrical noise. In the water analogy: a very narrow pipe
takes a long time to bring the pressure back up. Typical real pull-up values
sit between 4.7 kohm and 47 kohm for this reason.

---

## Submitting

Commit your answers to your own folder in the cohort repository:

```
students/<your-github-username>/lesson-03/homework.md
```

Commit message in English, as always.
