# Lesson 03 - Homework Answers

Name: Sven Kirsimäe  
GitHub username: `svennu`

Show the calculation for each problem, then give one final answer with units.

## Problem 1 - Resistor for a red LED

An ESP32-C3 GPIO pin outputs 3.3 V.

You want to drive a red LED from it. The LED has a forward voltage of 2.0 V,
and you want 10 mA flowing through it.

The LED and the resistor are in series, so the resistor gets whatever voltage
the LED does not use.

**What resistor value do you need?**

Working:

```text
Given:

- Supply voltage: 3.3 V
- LED forward voltage: 2.0 V
- Desired current: 10 mA = 0.01 A

Thinking:

The LED drops 2.0 V, leaving 3.3 V - 2.0 V = 1.3 V across the series resistor.

To select the resistor, I will use Ohm's law: R = V / I.

R = 1.3 V / 0.01 A = 130 ohms

A nominal 130-ohm resistor with a 5% tolerance can have an actual resistance
between 123.5 and 136.5 ohms, so the actual current may differ slightly from
the desired 10 mA.
```

Answer: 130 ohms

## Problem 2 - Resistor for a blue LED

Same 3.3 V GPIO pin. This time a blue LED with a forward voltage of 3.0 V, and
you want 15 mA through it.

**What resistor value do you need?**

Answer:

Look at how much smaller this answer is than the one in problem 1, and think
about why. This is the practical consequence of blue LEDs having a high forward
voltage: on a 3.3 V supply there is almost nothing left over.

Working:

```text
Given:

- Supply voltage: 3.3 V
- LED forward voltage: 3.0 V
- Desired current: 15 mA = 0.015 A

Thinking:

The LED drops 3.0 V, leaving 3.3 V - 3.0 V = 0.3 V across the series resistor.

To select the resistor, I will use Ohm's law: R = V / I.

R = 0.3 V / 0.015 A = 20 ohms

A nominal 20-ohm resistor with a 5% tolerance can have an actual resistance
between 19 and 21 ohms, so the actual current may differ slightly from
the desired 15 mA.
```

Answer: 20 ohms

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

Working:

```text
From the problem 1:
- resistor: 130 ohms
- carrying: 10 mA = 0.01A

We need to find the power (watts, W) dissipated by the resistor using: P = I * I * R

P = 0.01 A * 0.01 A * 130 ohms = 0.013 W
```

Answer: 0.013 W

Is a standard 0.25 W resistor safe to use? Yes. The resistor is dissipating 0.013 W,
which is roughly 5% of its maximum 0.25 W power handling capacity.

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

Working:

```text
Given example measurement:

- Voltage: 3.3 V
- Current: 3.2 mA

By Ohm's law the calculated measurement of the resistance is:
 R = V / A = 3.3 V / ( 3.2 / 1000 ) A = 1031.25 ohms

Nominal resistance was 1000 ohm with 5% tolerance range:
 1000 ohm +/- 5% = 1000 ohm +/- 50 ohm = 950..1050 ohm.

The calculated value of 1031.25 ohms falls inside the specified tolerance range of
950..1050 ohm. The resistor is behaving exactly within its manufactured specification.
```

Answer: 1031.25 ohms

Is the result within the 950-1050 ohm tolerance range? yes

### Calculation using my class measurement

Measured voltage: 3.3V
Measured current: 3.2 mA

Working:

```text
I setup my own circuit w ESP32-c3 for power: 3.3V - resistor - LED - GND

- Voltage: 3.28 (reading from the board)
- Resistors:
-   1. expected 2200 ohm: measured as: 2.17 kohm = 2170 ohm
-   2. expected 560 ohm: measured as: 0.547 kohm = 547 ohm
- LED voltage drop: 1.8 V (reading between the anode and cathode)
- Board load: SOS code working on the board, blue led blinking

Two resistors were used:
1. 2170 ohm resistor: mA measurement: 0.6 mA
2.  547 ohm resistor: mA measurement: 1.8 mA

Let's see if all fits into Ohm's law:
1. R = V / A = (3.28 - 1.8) / (0.6 / 1000) = 2467 ohm
2467 ohm is off the 5% tolerance range of expected 2200 ohm. I expect I have a measurement error,
probably not knowing the proper LED voltage drop or simply a noisy setup.
2. R = V / A = (3.28 - 1.8) / (1.8 / 1000) = 822 ohm
822 ohm is off the 5% tolerance range of expected 560 ohm. I expect I have a measurement error,
probably not knowing the proper LED voltage drop or simply a noisy setup.
```

Answer: See last section in 'Working'.

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

Working:

```text
Given:

- Supply voltage: 3.3 V
- Maximum current: 0.5 mA (below!)

Need to find the smallest resistance that limits the button-pressed current to 0.5 mA.

R = V / I = 3.3 V / (0.5 / 1000) A = 6600 ohms = 6.6 kohm.

So 6.6 kohm is the boundary value. Because the wording says "below 0.5 mA",
 a practical standard choice would be the next value above it constraining in 5% tolerance.
I would use a common 8.2 kohm resistor landing safely below 0.5 mA to 0.40 mA with allowed tolerance:
I = V / R = 3.3V / (8.2 * 1000) = 0.00040 A = 0.40 mA
```

Answer: 8.2 kohm
