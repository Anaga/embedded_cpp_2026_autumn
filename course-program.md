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

### Core - 60 hours, 15 sessions, order fixed

| Block | Content | Hours |
|---|---|---|
| K1 | Toolchain, git, GitHub, working process | 8 |
| K2 | Electronics, measurement, reading documentation | 8 |
| K3 | C and C++ on the reference platform (ESP32-C3) | 24 |
| K4 | Timers and interrupts (ESP32-C3), platform switch to Raspberry Pi Pico, I2C, SPI, UART, logic analyzer | 20 |

The core is non-negotiable. Everything downstream assumes it.

Changed 2026-10-05: the core was 56 hours / 14 sessions. From session 12 the
reference platform is the Raspberry Pi Pico (RP2040). The switch costs one
session, added to K4 and taken from the optional blocks.

### Optional blocks - 24 hours, 6 sessions

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

Note the arithmetic: the catalogue holds 44 hours of material for 24 hours of
schedule. That is the point. Writing more blocks than you teach is what buys
the ability to swap.

After the platform switch the 24 hours are two platform or protocol blocks
(16 hours, sessions 16-19) plus S5 (8 hours, sessions 20-21). S3 needs WiFi,
so it runs on the ESP32-C3: the original Pico has none.

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
| 11-14 | K4 Timers, UART, I2C, SPI | M4 | 16 |
| 15 | K4 Logic analyzer and bus diagnosis | M5 | 4 |
| 16-19 | Optional blocks (platform and protocol) | M4 | 16 |
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

### K4 - Peripherals and buses (sessions 11-15)

Re-planned 2026-10-05, at the start of K4. PWM, ADC basics, debounce and
first interrupts were already covered in K3 (see STATE AFTER K3 at the end), so
K4 spends its sessions on what is left: timers, then the three buses with
real devices.

Session 11 runs on the ESP32-C3. From session 12 the platform is the
Raspberry Pi Pico (RP2040, original board, no WiFi). Re-planned again
2026-10-07: the switch itself is kept small - toolchain at home (session 11
homework), soldering at the start of session 12 - so K4 stays about buses
with real devices. Porting the K3 classes (RgbLed, Button) and the session 11
timer code is deferred until a session needs the RGB LED, buttons or
potentiometer again.

**Session 11: Hardware timers and interrupt discipline** (2026-10-05)
- Why a hardware timer when millis() exists: polling vs a hardware event,
  jitter
- Timer hardware from the ESP32-C3 TRM, Timer Group chapter: APB 80 MHz ->
  16-bit prescaler -> 54-bit counter -> alarm -> interrupt. Two
  general-purpose timers (TIMG0, TIMG1). Compute divider and alarm value for a
  target rate. Ties back to session 4.
- Arduino-ESP32 core 2.x API: timerBegin(num, divider, countUp),
  timerAttachInterrupt, timerAlarmWrite, timerAlarmEnable. Examples found
  online are mostly core 3.x and use a different API (same trap as ledcAttach).
- ISR discipline: keep it short, IRAM_ATTR, volatile, no Serial, delay, heap
  or analogRead inside an ISR. The ISR records a value or sets a flag, loop()
  does the work.
- Timing measured in firmware, no oscilloscope: micros() timestamps in the
  ISR, min / max / jitter printed once per second
- Critical section introduced inside that measurement, not as a separate race
  demo: loop() snapshots and resets the ISR-updated statistics under
  portENTER_CRITICAL, with one slide on why
- Homework: get the Pico building and uploading from PlatformIO at home
  (blink GP25 with millis(), uptime line on Serial), plus pico-setup.md with
  what went wrong. Boards handed out without headers.

**Session 12: UART on the Pico** (2026-10-07)
- Homework check: Zadig for picotool on Windows, missing pinMode, "UF2
  Bootloader v2.0" is RP2040 revision B1, not a Pico 2
- Solder 2x20 headers with the breadboard as a jig; inspect and check
  continuity before USB (as in session 4)
- UART vs I2C vs SPI in one table: the map for sessions 12-14
- The Pico pin map for the rest of K4 (see Open decisions 5, settled)
- UART: no clock wire, 8N1 frame drawn bit by bit, 115200 baud in numbers,
  Serial (USB CDC) vs Serial1 (UART0, GP0 / GP1) vs Serial2
- Wiring two boards: TX to RX crossed, shared GND, never 3V3 or VBUS
- Lab stage 1: chat between two boards in pairs (USB <-> UART bridge;
  monitor_echo and send_on_enter in platformio.ini)
- Who talks first: UART has no master, the protocol decides. Stream vs
  poll vs command, discussed before the answer is shown
- Lab stage 2 (pairs, then swap): potentiometer on one Pico, blink rate on
  the other. Poll: the LED board (master) sends POT? every 100 ms, the pot
  board (slave) answers POT <0..1023> and never talks first. Master maps to a
  50..500 ms half-period, non-blocking blink on GP25; no answer in 200 ms ->
  "no reply", LED off (fail safe). Master as an enum class state machine
  (IDLE / WAITING). Line assembly in a char array. Pot on GP26 / AGND / 3V3,
  no 3.3k resistor, analogRead 10 bits by default
- Homework: the same protocol on one Pico with both UARTs - pot side on
  UART0 (Serial1), LED side on UART1 (Serial2, GP8 / GP9), wires GP0 -> GP9
  and GP8 -> GP1. Hidden lesson: a blocking wait for the reply deadlocks,
  because the answering side runs in the same loop()
- Deferred, for later sessions: rewiring RGB LED, buttons and potentiometer;
  porting RgbLed (ledc -> analogWrite, analogRead 10 bits by default) and the
  timer code (64-bit 1 MHz timer, 4 alarms, pico-sdk repeating timers, no
  IRAM_ATTR, no portENTER_CRITICAL)

**Session 13: I2C and the OPT4001**
- GPIO in depth where it is needed: floating inputs, open-drain outputs, why
  I2C needs pull-ups
- I2C protocol: start, 7-bit address, R/W, ACK / NACK, register pointer,
  repeated start, stop
- I2C scanner as the first test of the wiring
- OPT4001 by hand with Wire: device ID, configuration, result registers ->
  exponent and mantissa -> lux

**Session 14: SPI and the Nokia 5110**
- SPI: SCK, MOSI, MISO, CS, clock modes. The 5110 is write-only; D/C selects
  command vs data.
- RAII: a chip-select guard class (deferred from K3 on purpose)
- Display driver: init sequence, clear, text from a supplied font table
- Integration: lux from session 13 on the display
- Homework includes installing PulseView (plus Zadig on Windows) and
  confirming the analyzer is detected, so session 15 does not start with
  driver problems

**Session 15: Logic analyzer and bus diagnosis**
- PulseView with fx2lafw
- First capture is the session 12 UART link: the 8N1 frame from the slide,
  now measured, baud rate read off the capture
- I2C and SPI decoders on the students' own working code from sessions 13-14
- Diagnosis: buses broken on purpose (missing pull-up, wrong address, swapped
  lines, wrong SPI mode, CS never asserted), found with the analyzer
- Stays booked to M5: the analyzer is the subject

---

## The debugging thread

Debugging is not a module at the end. It is a line that runs through the whole
course, with each tool introduced at the point where its absence has started to
hurt:

| Introduced | Tool | Triggered by |
|---|---|---|
| Session 2 | Serial output and useful logging | first program that misbehaves |
| Session 3 | Multimeter | first wiring that does not work |
| Session 11 | Timing measured in firmware: micros(), min / max / jitter | first timing question |
| Session 15 | Logic analyzer, PulseView | first bus that stays silent |
| S1 block | ST-Link, breakpoints, single-stepping | first logic bug too subtle for prints |
| S5 block | Unit tests, CI | first regression |

A tool handed over before the student has felt the need for it is a tool they
will not use.

---

## Open decisions

1. **Which optional blocks for this group.** After the platform switch: two
   blocks for sessions 16-19 plus S5 for sessions 20-21. S1 and S2 recreate
   the previous stream's strongest material; S2 now builds on a toolchain the
   group already knows. S3 needs the ESP32-C3 (no WiFi on the original Pico).
   S4 is new investment. S6 no longer fits the 16 hours on its own.
2. **Reference platform - settled 2026-10-05.** ESP32-C3 Super Mini for
   K1-K3 and session 11, Raspberry Pi Pico (RP2040) from session 12. K3
   material stays on the C3; K4 material from session 12 is written for the
   Pico.
3. **Final project format.** Individual or paired, and whether the brief is
   fixed or student-chosen.
4. **Intro deck revision.** The existing deck promises hardware and tools that
   were not used: ESP32 Freenove, STM32 Discovery, Arduino IDE, RP2040 and
   Teensy. Slides 12 and 13 need to match the actual course before this group
   sees them.
5. **Pico pin map - settled 2026-10-07.** Buses on the Arduino-Pico default
   pins, so Serial1, Wire and SPI need no pin setup. UART0 GP0 / GP1 (pins
   1 / 2, GND pin 3); I2C0 GP4 SDA / GP5 SCL (6 / 7); SPI0 GP16 MISO / GP17 CS
   / GP18 SCK / GP19 MOSI (21 / 22 / 24 / 25); Nokia D/C GP20, RST GP21
   (26 / 27); RGB GP10 / 11 / 12 (14 / 15 / 16); buttons GP14 / GP15
   (19 / 20); potentiometer GP26 = ADC0 (31); on-board LED GP25. Still to be
   copied into course-conventions.md.
6. **Pico hardware before session 12.** Confirm the four boards, 8 x 20-pin
   male headers, and buy 1-2 spare boards: one for the instructor demo, one
   to replace a board that dies in class. course-conventions.md needs the
   Pico added to the inventory and toolchain sections.

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

STATE AFTER K3 (sessions 5-10 delivered, K3 closed 2026-10-02 on schedule)

Package format (since lesson 07, at the students' request):
- slides/ (generator .js + .pptx), demo/ (+ demo/README.md),
  homework/README.md as the full task spec, homework/platformio.ini,
  short lesson README.md pointing to both. No scaffold, no solution.
- homework/platformio.ini: [platformio] block with one src_dir line per
  student (sven, viktorija, anton, vahur), placed BEFORE [env:...] so the
  build_flags stay in the env section. Students keep headers in their own
  <name>/src folder, not in the shared include/.

Toolchain fact: PlatformIO platform = espressif32 ships Arduino-ESP32
core 2.x, not 3.x. PWM uses channels: ledcSetup / ledcAttachPin /
ledcWrite(channel, duty). ledcAttach does not exist here.

Hardware as actually wired:
- Button A GPIO 0, Button B GPIO 1, both INPUT_PULLUP, to GND
- RGB LED 500RGB4E, common anode to 3V3, R/G/B on GPIO 5/6/7,
  100 ohm on every channel (measured: all three look right)
- Potentiometer 10k on GPIO 4 (ADC1), 3.3k in series to 3V3;
  without it the ADC clips at about 2.9 V (measured max ~3850 counts)

What the students already have and can use:
- struct Colour, class RgbLed (PWM and common-anode inversion inside,
  begin/off, setColour(const Colour&) and setColour(r,g,b))
- class Button (debounced: a press counts only after 30 ms of quiet)
- template RingBuffer<T, N> (no heap), moving average
- state machines with enum class + switch, no delay() in loops
- interrupts touched once: attachInterrupt, IRAM_ATTR, volatile

Consequence for K4: PWM, ADC basics, button debounce and first
interrupts are already done. K4 re-planned 2026-10-05 around what is
left: 11 timers and ISR discipline (ESP32-C3), 12 soldering + three
buses + UART on the Pico, 13 I2C (OPT4001), 14 SPI (Nokia 5110), 15
logic analyzer. Details in the K4 section of the core session outline.

Deferred into K4 on purpose:
- RAII -> session 14, as a chip-select guard class
- critical sections -> session 11, snapshot of ISR-updated timing
  statistics

Pico toolchain facts (found 2026-10-06):
- platformio.ini: platform = https://github.com/maxgerhardt/platform-raspberrypi.git,
  board = pico, framework = arduino, board_build.core = earlephilhower.
  Arduino-Pico already builds as gnu++17.
- Windows: long paths must be enabled before the first build. Upload uses
  picotool, which on RP2040 needs Zadig: RP2 Boot (Interface 1) -> WinUSB
  (never Interface 0). Fallback: copy .pio/build/pico/firmware.uf2 onto the
  RPI-RP2 drive.
- The instructor's boards report "UF2 Bootloader v2.0": RP2040 revision B1.
  The bootloader is in ROM and cannot be updated.
- On-board LED is GP25 (LED_BUILTIN); without pinMode() it stays dark.
- Clock: F_CPU (build-time), rp2040.f_cpu() (run time),
  frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) (measured).

Cohort: 4 students, 1 with prior programming (Java). Pace accordingly.
Slide environment resets between days: copy slides-theme.js and
npm install pptxgenjs react react-dom react-icons sharp before building.