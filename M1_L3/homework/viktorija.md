# Lesson 03 - Homework (Viktorija)

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

Reasoning:

ESP 3.3V -> LED takes 2.0 V -> Resistor has 1.3V left -> before GND 0

Given 1.3V on the resistor, it needs X ohms to allow 10mA (0.01A)
x = V / I = 1.3 / 0.01 = 130 ohms

Answer: **130 ohms**

Explanation:

LED converts motivation of electrons into light. Resistor does not demotivate electrons, but creates an obstacle so that fewer of them can go through. Too many electrons would overstimulate the LED, because it can't handle too much motivation.

---

## Problem 2 - Resistor for a blue LED

Same 3.3 V GPIO pin. This time a blue LED with a forward voltage of 3.0 V, and
you want 15 mA through it.

**What resistor value do you need?**

Look at how much smaller this answer is than the one in problem 1, and think
about why. This is the practical consequence of blue LEDs having a high forward
voltage: on a 3.3 V supply there is almost nothing left over.

Reasoning:

Now we have only 0.3V on the resistor. Same formula, different values.

X = V / I = 0.3V / 0.015A = 20 ohms.

Answer: **20 ohms**

Thoughts: it seems we are giving more electrons for the blue light, because it needs more motivation to light up. Perhaps there is a point in that blue is associated with sadness ("I'm feeling blue").

---

## Problem 3 - Power in the resistor

Take the resistor value you calculated in problem 1, carrying the same 10 mA.

Some of the energy passing through a resistor becomes heat. How much depends on
the current and the resistance:

```
P = I * I * R
```

**How much power is dissipated in that resistor?**

A standard through-hole resistor is usually rated for 0.25 W. Compare your
answer to that rating and decide whether the resistor is safe to use.

Calculation:

Resistor - 130 ohms.
Current - 0.01 amps.

Power = 0.01 * 0.01 * 130 = 0.013 watts. Less than the rating, so it's safe. 

Answer: **0.013 watts and safe**

---

## Problem 4 - From measurement back to theory

In class you measured current through a 1 kohm resistor. Suppose the
measurement was 3.2 mA at 3.3 V.

**What is the actual resistance, calculated from that measurement?**

Compare it to the nominal 1000 ohm. Common resistors have a 5 percent
tolerance, which means anything from 950 to 1050 ohm is a resistor behaving
exactly as specified. Decide whether your calculated value falls inside that
range, and if it does not, think about where else the error could come from.

Then repeat the calculation with the numbers you actually measured in class.

Reasoning:

Resistor to match the measurements of 3.2mA & 3.3V would be 1031 ohms.
A little over 1 kohm is within the range.
I don't remember how much we actually measured in class.....

---

## Problem 5 - Pull-up resistor sizing

Harder. Attempt it after the first four.

A pull-up resistor connects a GPIO input to 3.3 V. When a button is pressed and
pulls that pin down to GND, current flows through the pull-up resistor
continuously, for as long as the button is held. On a battery powered device
that wasted current matters.

You want that current to stay below 0.5 mA.

**What is the minimum pull-up resistor value that satisfies this?**

There is a limit at the other end too. Making the pull-up very large reduces the
wasted current further, but the pin becomes slow to return to HIGH and more
easily disturbed by electrical noise. In the water analogy: a very narrow pipe
takes a long time to bring the pressure back up. Typical real pull-up values
sit between 4.7 kohm and 47 kohm for this reason.

Reasoning:

Resistance of the button has to be at least V/I = 3.3V / 0.0005A = 6600 ohms = 6.6 kohms

Answer: 6.6 kohms

---

## Submitting

Commit your answers to your own folder in the cohort repository:

```
students/<your-github-username>/lesson-03/homework.md
```

Commit message in English, as always.
