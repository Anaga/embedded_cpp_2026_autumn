# Lesson 03 - Electrical Fundamentals

Block K2, session 3 of 25. Showcase module M1.

This session has no firmware. Everything is measurement, components and hand
tools. Bring the board you already flashed in lesson 02 - we measure the blink
program you already have.

---

## Goals

By the end of this session you can:

- explain voltage, current and resistance well enough to predict what a circuit
  will do before you build it
- apply Ohm's law to pick a resistor rather than guessing
- name the common passive components and say what each one is for
- explain why a pull-up resistor makes a button read LOW when pressed
- state what a GPIO pin can and cannot drive
- use a multimeter to measure voltage, current and resistance
- describe what a soldering station is and how a joint is made

---

## What you need

- ESP32-C3 Super Mini with the lesson 02 blink firmware still on it
- USB-C cable
- Breadboard and jumper wires
- Multimeter
- Assorted resistors (1 kohm for the practical part)
- One LED

The bench power supply and the soldering stations are shared. You do not need
your own.

---

## Session plan

| Time | Block |
|---|---|
| 30 min | Voltage, current, resistance, power |
| 60 min | Passive components |
| 15 min | GPIO electrical characteristics |
| 45 min | Ohm's law in practice - measurement |
| 30 min | Soldering - demonstration |

---

## 1. Voltage, current, resistance, power

The water analogy, used throughout this session:

| Electrical | Water |
|---|---|
| Voltage (V) | pressure in the pipe |
| Current (I) | flow rate through the pipe |
| Resistance (R) | how narrow the pipe is |

Ohm's law: `V = I * R`

Rearranged, which is how you will actually use it:

```
I = V / R      how much current will flow
R = V / I      what resistor do I need
V = I * R      how much voltage is dropped
```

Power: `P = V * I`

Power matters for LEDs and resistors, where energy turns into heat and light.
It does not matter much for GPIO logic signals, where currents are tiny.

### Why 3.3V and not 5V

The ESP32-C3 runs its logic at 3.3V. A GPIO pin expects at most 3.3V on an
input. Applying 5V to a GPIO input can damage the pin permanently.

The board does have a 5V pin, but that is the USB supply passing through, not a
logic level. Never connect the 5V pin to a GPIO pin.

---

## 2. Passive components

A tour, not a deep dive. The point is that you know these exist and what each
one is for, so that a schematic stops looking like noise.

### Resistors

A resistor limits current. Its value is in ohms.

Resistors have a colour code printed as bands. It exists, and you can learn it,
but in practice it is faster and more reliable to measure the resistor with a
multimeter - especially since surface-mount resistors use a completely
different marking system anyway.

### LEDs

An LED is not a resistor. It drops a roughly fixed voltage, called the forward
voltage, and the current through it must be limited by something else -
normally a series resistor.

Forward voltage depends on colour:

| Colour | Typical forward voltage |
|---|---|
| Red | 1.8 - 2.0 V |
| Yellow | 2.0 - 2.2 V |
| Green | 2.0 - 3.0 V |
| Blue | 2.8 - 3.2 V |
| White | 2.8 - 3.4 V |

This is why the same resistor gives you a different current with a different
colour of LED. It is also why a blue LED on a 3.3V supply has almost no voltage
headroom left for the resistor.

The blue status LED on GPIO 8 of the Super Mini already has its current
limiting resistor on the board. That calculation was done for you. When you
wire your own LED to a pin, it is your job.

### Pull-up and pull-down resistors

An input pin that is connected to nothing does not read as 0. It floats, and
picks up whatever electrical noise is around it. A resistor is what gives a
disconnected input a defined value.

- **Pull-up**: resistor from the pin to 3.3V. Pin reads HIGH when nothing else
  drives it. A button then connects the pin to GND, so pressing reads LOW.
  This is active-low logic.
- **Pull-down**: resistor from the pin to GND. Pin reads LOW at rest, and a
  button connects it to 3.3V, so pressing reads HIGH. This is active-high.

The BOOT button on GPIO 9 uses a pull-up. That is why pressing it reads LOW.

In class we also build the opposite case on the breadboard - a button with an
external pull-down - so you can see both conventions side by side.

### Capacitors

A capacitor stores charge. You will see them everywhere in power supply
circuits, smoothing out a voltage that would otherwise dip every time the chip
draws a burst of current. Those small ones scattered next to every chip on a
board are decoupling capacitors doing exactly that job.

You do not need to calculate anything with them yet. Know that they exist and
that they are about storing and steadying.

### Inductors

An inductor resists changes in current. They appear in switching power
supplies and in filters. Same as capacitors for now: know they exist, know they
are about current and time.

---

## 3. GPIO electrical characteristics

A GPIO pin is a small switch, not a power supply.

- It can source or sink only a limited current - tens of milliamps at most, and
  much less if you want it to stay reliable.
- Driving a motor, a relay or a long LED strip directly from a pin will not
  work. Those need a driver or a transistor between the pin and the load.
- Total current across all pins is also limited, not just per pin.

### Strapping pins

GPIO 8 and GPIO 9 are not ordinary pins. The chip reads their voltage level at
the moment it powers up to decide which mode to boot into. Something connected
to them can therefore stop the board from booting normally, even when the code
is perfectly correct.

This is why the wiring on those two pins was left alone in lesson 02.

We come back to this properly in block K4, where GPIO gets a full session.

---

## 4. Ohm's law in practice

### Part A - bench power supply, demonstration

One resistor, one bench supply. The voltage is swept slowly while the
multimeter shows the current. You see a continuous relationship rather than two
isolated data points: double the voltage, double the current.

### Part B - your own board, hands on

Your board is your voltage source. It gives you two fixed voltages: the 3.3V
pin and the 5V pin.

1. Put a 1 kohm resistor on the breadboard.
2. Connect it between the 3.3V pin and GND, with the multimeter in series so it
   measures the current through the resistor.
3. Record the current.
4. Repeat from the 5V pin.
5. For each measurement, calculate R = V / I and compare with 1 kohm.

The currents are small - a few milliamps - but they are easily readable on any
multimeter and they are safe for the board.

Note what you find: the calculated value will not be exactly 1000 ohm. Real
resistors have a tolerance, commonly 5 percent, and your multimeter has its own
error. Measuring beats assuming, and this is the first place where you see it.

### Multimeter reminders

- **Voltage** is measured in parallel, across the thing you care about.
- **Current** is measured in series - the circuit must be broken and the meter
  put in the gap.
- **Resistance** is measured with the power off and the component disconnected
  from the rest of the circuit.

Putting the meter in current mode across a voltage source is the classic way to
blow the fuse inside the meter. Check the mode and the socket before you
connect.

---

## 5. Soldering - demonstration

Watch only this session. You solder on Friday.

Three soldering stations are available, plus the instructor's own irons.

What is covered:

- what a soldering station is and how temperature control matters
- what flux does and why a joint without it looks and behaves badly
- the shape of a good joint versus a cold joint or a bridge
- the most common real task: soldering header pins onto a board such as the
  ESP32-C3 Super Mini

Safety, briefly:

- the tip is hot enough to burn instantly and stays hot long after switch off
- the iron lives in its holder, never on the bench
- solder fumes are flux smoke - work in ventilated space, do not lean over it
- wash hands afterwards

---

## Homework

See `homework/README.md`. Five calculation problems on Ohm's law, resistor
selection and power. Paper, calculator, no hardware needed.
