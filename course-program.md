# Course Program

**Embedded Software Development in C/C++**
100 classroom hours + 80 independent hours = 180 academic hours
25 sessions x 4 academic hours, 2-3 sessions per week, 9-12 weeks

---

## How this document works

The program has two views of the same course.

**The showcase** is what students, the certificate and the course description
see: six modules named by competence. It is stable. It does not change when a
group runs fast or slow.

**The delivery plan** is how the course is actually taught: a fixed core, a set
of interchangeable platform blocks, and a closing section. It absorbs schedule
reality.

They are linked by attribution: every session's 4 hours are booked against one
showcase module. Change which optional blocks you teach, keep the showcase
totals intact.

This exists because the previous stream reached session 17 of 25. A structure
with no slack does not survive contact with a real group. Here the slack is
designed in: drop an optional block, the core is untouched.

---

## View 1: The showcase

| # | Module | Hours | Sessions |
|---|---|---|---|
| M1 | Introduction to Microcontrollers and Electronics | 16 | 4 |
| M2 | Fundamentals of Programming in C | 12 | 3 |
| M3 | Fundamentals of C++ for Microcontrollers | 12 | 3 |
| M4 | Platforms, Peripherals and Protocols | 36 | 9 |
| M5 | Debugging and Testing Embedded Systems | 12 | 3 |
| M6 | Final Project | 12 | 3 |
| | **Total** | **100** | **25** |

Independent work: 80 hours, roughly 3 hours of homework per session.

### Learning outcomes

By the end of the course a student can:

- develop firmware for microcontrollers in C and C++
- work with at least two hardware platforms and move code between them
- program peripherals: GPIO, PWM, ADC, timers, interrupts, UART, I2C, SPI
- read a datasheet and a reference manual to answer a specific question
- diagnose a non-working board with a systematic method rather than by guessing
- use version control as part of normal development
- deliver a working device as an independent project

---

## View 2: The delivery plan

### Core - 56 hours, 14 sessions, order fixed

| Block | Content | Hours |
|---|---|---|
| K1 | Toolchain, git, GitHub, working process | 8 |
| K2 | Electronics, measurement, reading documentation | 8 |
| K3 | C and C++ on the reference platform (ESP32-C3) | 24 |
| K4 | Peripherals and buses: GPIO, PWM, ADC, timers, interrupts, UART, I2C, SPI | 16 |

The core is non-negotiable. Everything downstream assumes it.

### Optional blocks - 28 hours, 7 sessions, choose 4 of these

| Block | Content | Hours | Prerequisite |
|---|---|---|---|
| S1 | STM32: CubeMX and HAL | 8 | K4 |
| S2 | RP2350: PIO and the second core | 8 | K4 |
| S3 | Connectivity: WiFi, HTTP, JSON, MQTT | 8 | K3 |
| S4 | RTOS and multitasking | 8 | K4 |
| S5 | Testing and CI | 8 | K3 |
| S6 | Actuators: motors and drivers | 4 | K4 |

**Rule that makes this work:** every optional block may depend only on the
core, never on another optional block. The moment S2 assumes S1 was taught, the
modularity is decorative and you are back to a single fixed sequence.

Note the arithmetic: the catalogue holds 44 hours of material for 28 hours of
schedule. That is the point. Writing more blocks than you teach is what buys
the ability to swap.

### Closing - 16 hours, 4 sessions

| Block | Content | Hours |
|---|---|---|
| Z1 | Cross-platform comparison and portability | 4 |
| Z2 | Final project | 12 |

---

## Hour attribution

How delivery sessions book against showcase modules.

| Sessions | Delivery block | Showcase module | Hours |
|---|---|---|---|
| 1-2 | K1 Toolchain and workflow | M1 | 8 |
| 3-4 | K2 Electronics and documentation | M1 | 8 |
| 5-7 | K3 C fundamentals | M2 | 12 |
| 8-10 | K3 C++ fundamentals | M3 | 12 |
| 11-13 | K4 Peripherals and buses | M4 | 12 |
| 14 | K4 Logic analyzer and bus diagnosis | M5 | 4 |
| 15-19 | Optional blocks (platform and protocol) | M4 | 20 |
| 20-21 | Optional block (testing and CI) | M5 | 8 |
| 22 | Z1 Cross-platform comparison | M4 | 4 |
| 23-25 | Z2 Final project | M6 | 12 |

Totals: M1 16, M2 12, M3 12, M4 36, M5 12, M6 12. Sum 100.

If an optional block is swapped for one of a different length, re-check this
table. It is the only place the two views can drift apart.

---

## Core session outline

### K1 - Toolchain and working process (sessions 1-2)

**Session 1: Orientation**
- Course structure, expectations, how homework is submitted
- MCU architecture: what a microcontroller is and is not, memory, peripherals,
  the boot process
- Hardware on the table: ESP32-C3 (RISC-V) vs STM32 (Cortex-M) vs RP2350
  (carries both architectures on one die, boots into either). Use this to make
  the instruction-set discussion concrete rather than abstract.
- GitHub accounts, SSH keys, first clone
- Collect GitHub usernames before anyone leaves

**Session 2: First firmware**
- VS Code and PlatformIO installation and project anatomy
- `platformio.ini`, build, upload, serial monitor
- Blink, then blink with a button
- git workflow: add, commit, push, and what to do when it refuses

### K2 - Electronics and documentation (sessions 3-4)

**Session 3: Electrical fundamentals**
- Voltage, current, resistance, power, and why 3.3V vs 5V matters
- Passive components, pull-up and pull-down, current limiting
- GPIO electrical characteristics: what a pin can and cannot drive
- Multimeter: measuring instead of assuming

**Session 4: Reading documentation**
- Datasheet vs reference manual vs application note
- Finding one specific answer in 800 pages
- Component selection from specification
- AI tools as a documentation aid, and where they mislead

### K3 - C and C++ on ESP32-C3 (sessions 5-10)

**C fundamentals (5-7)**
- Types, fixed-width integers, operators, control flow
- Functions, scope, the stack
- Pointers and addresses
- Arrays, strings, structs
- Memory layout: what lives where and how much of it there is

**C++ for microcontrollers (8-10)**
- Classes as a way to wrap a peripheral
- Constructors, RAII, and what it costs on an MCU
- References, overloading, templates in moderation
- What to avoid in embedded C++ and why: exceptions, RTTI, dynamic allocation
- Driver design: a class that owns a pin

### K4 - Peripherals and buses (sessions 11-14)

- GPIO in depth, PWM, ADC and its noise floor
- Timers and interrupts, ISR discipline, what may not be called from an ISR
- UART, I2C, SPI: protocol, wiring, and one working sensor each
- Session 14: the logic analyzer. Diagnosing a bus that does not work.

---

## The debugging thread

Debugging is not a module at the end. It is a line that runs through the whole
course, with each tool introduced at the point where its absence has started to
hurt:

| Introduced | Tool | Triggered by |
|---|---|---|
| Session 2 | Serial output and useful logging | first program that misbehaves |
| Session 3 | Multimeter | first wiring that does not work |
| Session 11 | Oscilloscope view of PWM | first timing question |
| Session 14 | Logic analyzer, PulseView | first bus that stays silent |
| S1 block | ST-Link, breakpoints, single-stepping | first logic bug too subtle for prints |
| S5 block | Unit tests, CI | first regression |

A tool handed over before the student has felt the need for it is a tool they
will not use.

---

## Open decisions

1. **Which four optional blocks for this group.** S1 and S2 recreate the
   previous stream's strongest material. S3 connects to it well. S4 and S5 are
   new investment. S6 is short and can pair with another.
2. **Reference platform confirmation.** ESP32-C3 Super Mini is assumed
   throughout the core. Changing it means rewriting K3 and K4.
3. **Final project format.** Individual or paired, and whether the brief is
   fixed or student-chosen.
4. **Intro deck revision.** The existing deck promises hardware and tools that
   were not used: ESP32 Freenove, STM32 Discovery, Arduino IDE, RP2040 and
   Teensy. Slides 12 and 13 need to match the actual course before this group
   sees them.

---

## What changed from the 2026 stream

- Modular structure replaces a fixed six-module sequence, so schedule slippage
  costs an optional block rather than the end of the course.
- The 30-hour platforms module is broken into 8-hour blocks with checkpoints
  every two sessions instead of one checkpoint per 30 hours.
- Debugging is distributed across the course instead of parked at the end.
- Electronics and documentation get their own core block rather than being
  assumed as prior knowledge.
- GitHub Classroom is dropped in favour of a public repository with students as
  contributors - which is what actually happened last time.
