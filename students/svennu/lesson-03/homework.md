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

Given:

- Supply voltage: 3.3 V
- LED forward voltage: 3.0 V
- Desired current: 15 mA

Working:

```text

```

Answer: <!-- value and unit -->

## Problem 3 - Power in the resistor

Use the resistor value from problem 1 with a current of 10 mA.

Working:

```text

```

Answer: <!-- value and unit -->

Is a standard 0.25 W resistor safe to use? <!-- yes/no and brief reason -->

## Problem 4 - From measurement back to theory

Given example measurement:

- Voltage: 3.3 V
- Current: 3.2 mA

Working:

```text

```

Answer: <!-- value and unit -->

Is the result within the 950-1050 ohm tolerance range? <!-- yes/no -->

### Calculation using my class measurement

Measured voltage: <!-- value and unit -->  
Measured current: <!-- value and unit -->

Working:

```text

```

Answer: <!-- value and unit -->

## Problem 5 - Pull-up resistor sizing

Given:

- Supply voltage: 3.3 V
- Maximum current: 0.5 mA

Working:

```text

```

Answer: <!-- minimum value and unit -->
