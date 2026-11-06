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

Selected 2026-10-31 for this group (see "Plan for sessions 16-25" below): S1
STM32 (sessions 16-17), then connectivity on the ESP32-C3 for sessions 18-21.
S3 is extended to four sessions: WiFi and a web server (18), power modes, NTP
and ESP-NOW (19), MQTT (20), CAN through TWAI (21). S5 Testing and CI is not
taught. Sessions 20 and 21 carry a diagnosis lab each and are booked to M5, so
the showcase totals stay intact. S2, S4 and S6 are not taught.

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
| 16-17 | S1 STM32: CubeMX and HAL | M4 | 8 |
| 18-19 | S3 WiFi, web server; power modes, NTP, ESP-NOW (ESP32-C3) | M4 | 8 |
| 20 | S3 MQTT and network diagnosis | M5 | 4 |
| 21 | CAN through TWAI and bus diagnosis | M5 | 4 |
| 22 | Z1 Cross-platform comparison | M4 | 4 |
| 23-25 | Z2 Final project | M6 | 12 |

Totals: M1 16, M2 12, M3 12, M4 36, M5 12, M6 12. Sum 100.

Re-booked 2026-10-31: sessions 20-21 were S5 Testing and CI. They are now
connectivity sessions, and stay on M5 only because each carries a real
diagnosis lab (fault injection, analyzer or protocol-inspector capture,
comparison with a known-good trace). If that lab is cut, this table no longer
holds.

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

Changed 2026-10-12: the OPT4001 is no longer available. Session 13 uses the
AHT10 temperature and humidity sensor instead. The AHT10 is command-based,
not register-based, so the register pointer and repeated start stay on the
slides as theory. Session 14 integration and the session 15 captures follow
the sensor change.

Changed 2026-10-16: spare AHT10 modules turned up in session 13. Every
student and the instructor now has an AHT10, and every student has a Nokia
5110, so labs from session 14 on are individual rather than in pairs.

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

**Session 13: I2C and the AHT10** (plan settled 2026-10-12, delivered as
planned)
- Homework check: two UARTs on one Pico, and why a blocking wait for the
  reply deadlocks
- GPIO in depth where it is needed: floating inputs, open-drain outputs, why
  I2C needs pull-ups. Students measure the pull-ups on their own module with
  the multimeter (SDA -> VIN, SCL -> VIN, unpowered), as in session 3
- I2C protocol: start, 7-bit address, R/W, ACK / NACK, stop. Register pointer
  and repeated start shown as "how most sensors work"; then register-based vs
  command-based devices, and why the AHT10 is the second kind
- Wiring: 3V3 OUT, GND, SDA GP4, SCL GP5. Never VBUS: if the module pull-ups
  go to VIN, 5 V lands on pins that are not 5 V tolerant
- I2C scanner as the first test of the wiring: expect 0x38
- AHT10 by hand with Wire, in pairs: init (0xE1 0x08 0x00) and calibration bit,
  trigger (0xAC 0x33 0x00), wait about 75 ms in an enum class state machine
  (no delay() - follows IDLE / WAITING from session 12), read 6 bytes, busy
  bit, unpack the two 20-bit fields with shifts and masks, convert:
  RH = raw / 2^20 * 100, T = raw / 2^20 * 200 - 50. Check: breathe on it
- Trap, same family as core 3.x: most examples online are for the AHT20 /
  AHT2x (init 0xBE, 7th CRC byte). Translate, do not paste
- Then the same reading through a library (Adafruit AHTX0): compare the
  numbers and see what the library hides
- Demo covers the library version and what the slides show; the by-hand
  driver is the lab and stays out of the demo
- Homework: a non-blocking Aht10 class, a reading every 2 s, "sensor missing"
  once and automatic recovery. Modules go home with the students

**Session 14: SPI and the Nokia 5110** (plan settled 2026-10-16)
- Homework check: the non-blocking Aht10 class from session 13, "sensor
  missing" and recovery. The class goes onto the display in lab 3
- SPI on paper: SCK, MOSI, MISO, CS; push-pull (contrast with open-drain in
  session 13), one CS per device instead of an address, full duplex, no ACK,
  so no scanner and no error when a wire is wrong (bridge to session 15).
  CPOL / CPHA, the PCD8544 is mode 0, MSB first, up to 4 MHz; SPISettings,
  beginTransaction / endTransaction
- PCD8544 from the datasheet: 84 x 48, 6 banks x 84 bytes = 504 bytes, one
  byte = a vertical column of 8 pixels, bit 0 on top. Write-only (GP16 MISO
  stays free); D/C selects command vs data; RST pulse after power-up. Basic
  and extended instruction sets (0x20 / 0x21), Vop, temperature coefficient,
  bias, display control, set X / Y
- RAII: a ChipSelect guard class (deferred from K3 on purpose) - constructor
  opens the transaction and pulls CS low, destructor releases both
- Every student has their own Pico, AHT10 and 5110, so all lab stages are
  individual. Lab 1: wiring, reset, init, checkerboard, contrast (Vop differs
  per module). Lab 2: text from a supplied 5x7 font table (lab/font5x7.h,
  14 x 6 characters). Lab 3: AHT10 reading on the display every 2 s, no
  delay()
- Library comparison, as in session 13: the demo shows the same screen
  through Adafruit PCD8544 + GFX. What it hides: the 504-byte framebuffer,
  display() sending the changed rectangle (all 504 bytes after
  clearDisplay()), CS and D/C handling, the init sequence. The
  by-hand driver is the lab and stays out of the demo
- Homework (changed after the session, 2026-10-16: the group is better
  served by learning to use libraries than by writing drivers): start from
  the demo, add a button on GP14 (INPUT_PULLUP, debounce method free).
  A reading every second. Screen 1: temperature and humidity now, only the
  two values redrawn each second. Screen 2: bar chart with fillRect(), each
  bar the average of 5 readings (one bar every 5 s, calmer than raw values),
  last 14 bars, newest on the right, automatic scale with min / max on a
  text line. The history keeps filling while screen 1 is shown. Plus installing PulseView (Zadig on
  Windows, Rosetta 2 and "Open Anyway" on Apple Silicon Macs) and confirming
  the analyzer is detected, so session 15 does not start with driver
  problems

**Session 15: Logic analyzer and bus diagnosis** (plan settled 2026-10-19)
- Two analyzers (instructor's and one student's): two stations, every
  capture lab in pairs
- Homework check: session 14 button and two screens, PulseView status
- Opening slide (asked for after session 14, a student question): why SPI0
  sits on GP16-19. SPI0 signal-to-GPIO table (RX 0/4/16/20, CSn 1/5/17/21,
  SCK 2/6/18/22, TX 3/7/19/23 - four positions per signal, mixable, the
  pinout only looks like five groups because GND pins split GP0-7). Why
  16-19: Arduino-Pico defaults; GP0-7 collide with UART0 and I2C0; GP23 is
  not on the Pico header; one edge of the board with D/C and RST. Hardware
  CSn vs software CS: one CSn per block, PL022 pulses it between bytes in
  mode 0, so drivers use any GPIO as CS, one per device. Students clip the
  analyzer onto exactly these pins
- PulseView with fx2lafw. The FX2 clone shows up as "Saleae Logic" (it reuses
  Saleae's USB ID). Sample at 4-10x the bit rate; samples = rate x time; SPI
  at 4 MHz needs 24 MHz sampling, which is the limit of the USB stream. The
  FX2 sends one byte per sample for D0-D7, so disabling channels does not
  help; the demo can cap the SPI clock instead (SPI_CLOCK_CAP_HZ)
- One harness for all three buses: D0 TX GP0, D1 SDA, D2 SCL, D3 SCK, D4
  MOSI, D5 CS, D6 D/C, D7 RST
- First capture is UART: the 8N1 frame of 'T' (first byte of every demo
  line), baud rate read off the narrowest pulse
- I2C on the AHT10 through Adafruit AHTX0: the library writes AC 33 00, then
  reads one status byte every 10 ms while busy (about 50 ms blocked in
  getEvent()), then reads 6 bytes. Real capture from the instructor bench on
  the slide. getStatus() returns 0xFF on error, so an unplugged sensor during
  a reading hangs the loop
- SPI on the 5110: mode 0, CS per command byte and per data block, D/C read
  off the trace. begin() order on the wire: bias, Vop, function set, display
  control, splash. Dirty rectangle: clearDisplay() + display() = 6 banks /
  504 bytes; one value changed = one bank, changed columns; nothing changed =
  only the final Set Y (the library itself does not know why it is there)
- Diagnosis: symptom -> hypothesis -> capture -> compare with a known-good
  capture -> conclusion. Lab with four prebuilt mystery firmware files and a
  bug report each: wrong I2C address (0x39), SPI mode 1, UART0 at 9600, and
  one without a bus fault. Answer key and generator in solution/ only. Dropped
  after the plan discussion: swapped SDA / SCL (GP4 / GP5 cannot swap on I2C0,
  it would need bit-banging) and the missing pull-up (Arduino-Pico Wire
  enables the RP2040 internal pull-ups anyway)
- Demo: the three buses and the potentiometer on one Pico (setpoint 15.0 ..
  30.0 C, values redrawn only on change, the same line on USB and Serial1
  every second) - capture target in class, homework base at home
- Homework (light, closes K4): room thermostat on the demo. RGB LED back on
  the Pico (GP10 / 11 / 12): HEAT red / OK green / COOL blue with a +-0.5 C
  band as a named constant, LED off when the sensor is missing, state on the
  5110 and at the end of the UART line. RGB implementation free. Question
  in the code: what happens at the edge of the band (hysteresis optional)
- Stays booked to M5: the analyzer is the subject

---

## Plan for sessions 16-25

Settled 2026-10-31, after session 17. Eight sessions remain after 17: weeks
45-48, two per week, fixed Monday and Friday (Wednesdays belong to another
group). Session 25 is demo day, Fri 2026-11-27.

### S1 STM32 - delivered (sessions 16-17)

**Session 16** (2026-10-26): STM32F411 Black Pill, CubeMX and HAL, USB CDC,
I2C with the BH1750 and the GY-521 (MPU-6050). Upload over USB DFU only, no
debugger. Went poorly: CubeMX configuration took most of the session and no
working demo was reached.

**Session 17** (2026-10-30): the same project pre-generated and verified on the
instructor's machine, students got it ready-made: build, DFU upload, sensors.
Homework: night light (BH1750 thresholds) and orientation (MPU-6050 Z axis,
enum + to-text function). The students' three Black Pills turned out to be
STM32F401CCU6; only the instructor's board is an F411. No F401 variant is
made: the STM32 block ends here.

Not taught from the STM32 wish list: CAN on STM32 (moved to the ESP32-C3,
session 21), K-Line, DMA, high-speed ADC / DAC. The ST-Link step of the
debugging thread did not happen (no debugger on the Black Pill).

### Connectivity on the ESP32-C3 (sessions 18-21)

Back on the ESP32-C3 Super Mini, PlatformIO platform espressif32, Arduino
core 2.x (examples online are mostly 3.x, same trap as ledcAttach). Pin budget
on the Super Mini is tight: 13 usable GPIO, and 2, 8, 9 are strapping pins.

**Session 18: WiFi and a web server** (Mon 2026-11-02). Package ready
2026-11-01 and tested on the instructor C3: slides (26), lab1-web (AHT10 +
WiFi + web server), lab2-mqtt (lab 1 + MQTT on broker.hivemq.com), homework
(RGB colour from the web: POST /rgb with validation, three sliders, MQTT
extra). Class network: the instructor's own router on the table. Delivered:
all boards on WiFi, page and /status worked for everyone.
- WiFi station: connect, status, non-blocking reconnect
- Web server on the board: a page with buttons that switch the LED, and
  /status returning JSON with the AHT10 reading. HTTP request and response,
  GET vs POST, JSON
- Practical: 2.4 GHz only, no WPA2-Enterprise - the class runs on its own
  router or hotspot. Many Super Mini boards connect only after
  WiFi.setTxPower(WIFI_POWER_8_5dBm); check every student board on the bench
  first
- Collect one line per student: final project idea and the hardware it needs

**Session 19: Sleep, time and ESP-NOW** (Fri 2026-11-06, plan settled the
same day). Widened from ESP-NOW alone at the instructor's request: power
saving and NTP come first, so ESP-NOW answers a question the group has
already felt - "why not just WiFi, we are always online and on USB?". Package
M4_L19: slides (26), lab1-sleep, lab2-ntp, lab3-node (C3), lab3-gateway (the
instructor's Lolin S3 / S3 Mini, envs s3mini / s3 / c3), homework. Folders named lab1..lab3 as in L18, no
single demo/.
- Order: homework 18 check (15 min) -> power modes (45, lab 1) -> exact time
  (30, lab 2) -> ESP-NOW (40, demo) -> lab 3 (40) -> summary (10)
- Power: ESP32-C3 datasheet v1.7 numbers (TX up to 335 mA peak, RX 84 mA,
  modem sleep 13-28 mA, light sleep 130 uA, deep sleep 5 uA, power off 1 uA);
  battery arithmetic for a 1000 mAh cell; chip numbers vs board numbers
  (power LED, regulator, AHT10 on the rail). Deep sleep is a reboot:
  RTC_DATA_ATTR (8 KB RTC memory) and the RTC timer survive. Wake sources:
  timer, GPIO 0-5 only (button A on GPIO 0), RESET. Wake cause switch.
  USB Serial/JTAG disappears in deep sleep; the PlatformIO monitor
  reconnects by itself; upload by pressing RESET or button A for a 30 s
  awake window, BOOT + RESET as last resort. The board sleeps on the USB
  cable too. RGB LED as a state display in every lab (asked for on
  2026-11-06): red blink = woke up, blue = awake and COM port up, green =
  asleep (held LOW with gpio_hold_en + gpio_deep_sleep_hold_en). Lab 1:
  30 s awake, 60 s asleep on every wake; lab 2: 60 s sleep, 30 s window on
  cold start and button; lab 3 node: 20 s sleep, three red blinks on FAIL
- Time: configTzTime with the Tallinn POSIX TZ string
  (EET-2EEST,M3.5.0/3,M10.5.0/4), NTP once on a cold start, WiFi off, the
  RTC keeps time across deep sleep; TZ must be set again on every wake;
  the internal 136 kHz RC drifts with temperature
- ESP-NOW: motivated by the lab 2 awake times (seconds with WiFi vs
  milliseconds without); MAC addressing, MAC-level ACK on unicast, no ACK on
  broadcast, 250 bytes, 20 peers; core 2.x callback signatures (core 3.x
  examples do not compile); one packed NodeMsg struct with a version byte
  and static_assert in a shared msg.h; the channel rule - the gateway joined
  to the router sets the channel, the node scans for the SSID once and keeps
  the channel in RTC memory, rescans after 3 failed sends; gateway needs
  WiFi.setSleep(false)
- Lab 3: sleeping AHT10 node -> ESP-NOW -> instructor gateway (WiFi + NTP +
  web page with every node, last packet time, awake_ms, missed packets).
  AHT10 code as in L18 lab1-web (Adafruit AHTX0 with an address check first)
- Homework: a clock that sleeps, one board at home. Cold start WiFi -> NTP
  -> sleep; timer wake every 10 min with a green blink and the exact time;
  button A wakes at once and restarts the 10 min; next wake time printed
  before every sleep; hourly NTP resync with the measured RTC drift in ms.
  Overnight run, drift lines in the commit message
- Code not compiled before the session (no PlatformIO registry access from
  the build environment); built and checked on the instructor's machine

**Session 20: MQTT and network diagnosis** - booked to M5
- Broker (on the instructor laptop or a Pi), PubSubClient, MQTT Explorer
- Publish the AHT10 reading, subscribe to an LED command; topic per student
- Diagnosis lab (required for the M5 booking): MQTT Explorer used as a
  protocol inspector; injected faults - broker down, WiFi lost, wrong topic,
  a retained message nobody expected. Symptom -> hypothesis -> evidence ->
  conclusion, as in session 15

**Session 21: CAN through TWAI and bus diagnosis** - booked to M5
- ESP32-C3 TWAI controller (ESP-IDF driver/twai.h from Arduino), TJA1050
  transceivers, three nodes on one bus
- Hardware: only 3 TJA1050 modules, so three stations. The TJA1050 runs from
  5 V and drives RXD to 5 V; the C3 is not 5 V tolerant, so RXD goes through a
  divider (for example 10k / 20k). TXD from 3.3 V is fine. Check the 120 ohm
  terminations on the modules
- Diagnosis lab (required for the M5 booking): PulseView CAN decoder on the
  RX line; faults - missing termination, wrong bit rate, a single node with no
  one to ACK -> error frames. Known-good capture to compare against

### Closing (sessions 22-25)

**Session 22: Z1 cross-platform comparison**
- The group has worked on the ESP32-C3, the Pico and the Black Pill: the same
  logic on three platforms, what carries over unchanged and what is HAL
- Final project ideas approved (students have been thinking about them for a
  while; ideas collected in session 18, checked against the C3 pin budget and
  available hardware)

**Sessions 23-25: Z2 final project**, demo day in session 25 (Fri 2026-11-27).

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
| Session 20 | MQTT Explorer as a protocol inspector, fault injection | first message that never arrives |
| Session 21 | Logic analyzer on CAN, error frames | first bus with more than two nodes |

Not delivered for this group: ST-Link breakpoints and single-stepping (planned
in S1, no debugger on the Black Pill) and unit tests with CI (S5, replaced by
connectivity). Both are candidates for the next stream.

A tool handed over before the student has felt the need for it is a tool they
will not use.

---

## Open decisions

1. **Which optional blocks for this group - settled 2026-10-31.** S1 STM32
   (16-17), then connectivity on the ESP32-C3 (18-21): WiFi and web server,
   power modes + NTP + ESP-NOW, MQTT, CAN through TWAI. S5 dropped; 20-21
   booked to M5 through their diagnosis labs. See "Plan for sessions 16-25".
2. **Reference platform - settled 2026-10-05.** ESP32-C3 Super Mini for
   K1-K3 and session 11, Raspberry Pi Pico (RP2040) from session 12. K3
   material stays on the C3; K4 material from session 12 is written for the
   Pico. From session 18 the course returns to the ESP32-C3 for connectivity.
3. **Final project format - mostly settled 2026-10-31.** Students were told
   about the final project early and should already have ideas. Ideas are
   collected in session 18 and approved in session 22. Still open: individual
   or paired.
4. **Intro deck revision.** The existing deck promises hardware and tools that
   were not used: ESP32 Freenove, STM32 Discovery, Arduino IDE, RP2040 and
   Teensy. Slides 12 and 13 need to match the actual course before this group
   sees them.
5. **Pico pin map - settled 2026-10-07.** Buses on the Arduino-Pico default
   pins, so Serial1, Wire and SPI need no pin setup. UART0 GP0 / GP1 (pins
   1 / 2, GND pin 3); I2C0 GP4 SDA / GP5 SCL (6 / 7); SPI0 GP16 MISO / GP17 CS
   / GP18 SCK / GP19 MOSI (21 / 22 / 24 / 25); Nokia D/C GP20, RST GP21
   (26 / 27); RGB GP10 / 11 / 12 (14 / 15 / 16); buttons GP14 / GP15
   (19 / 20); potentiometer GP26 = ADC0 (31); on-board LED GP25. Copied into
   course-conventions.md 2026-10-07.
6. **Pico hardware before session 12.** Confirm the four boards, 8 x 20-pin
   male headers, and buy 1-2 spare boards: one for the instructor demo, one
   to replace a board that dies in class. Pico inventory and toolchain notes
   added to course-conventions.md 2026-10-07.
7. **AHT10 modules before session 13.** Measure whether the modules carry
   their own pull-ups and where they go (VIN or an on-board LDO output).
   Decides how the pull-up slide is shown. Library for the comparison step:
   Adafruit AHTX0. 2026-10-19: no longer needed for session 15 - the missing
   pull-up fault was dropped, and Arduino-Pico Wire turns on the RP2040
   internal pull-ups on SDA / SCL in any case.
8. **Logic analyzers - settled 2026-10-19.** Two analyzers: the instructor's
   and one student's. Session 15 runs two stations with students in pairs.
9. **PulseView on the students' Macs - mostly settled 2026-10-19.** Tested on
   the instructor's Apple Silicon Mac: PulseView 0.4.2 dmg from sigrok.org
   runs under Rosetta 2 after "Open Anyway" in Privacy & Security; the
   analyzer is detected with fx2lafw (shows as "Saleae Logic") and the AHT10
   I2C traffic decodes at 1 MHz. Still open: the highest stable sample rate
   (24 MHz is needed for SPI at 4 MHz). If it fails, the demo caps the SPI
   clock at 1 MHz and captures run at 8 MHz.
10. **C3 pin map for sessions 18-21 - I2C settled 2026-11-01.** I2C SDA
    GPIO 8 / SCL GPIO 9 (the Wire defaults of lolin_c3_mini, so most examples
    work without pin setup). RGB stays on 5 / 6 / 7, buttons on 0 / 1,
    potentiometer on 4 (K3 wiring). 8 and 9 are strapping pins; the AHT10
    module pull-ups keep them HIGH at reset, which is the normal boot. Must be
    said on the slides and in the README: GPIO 9 is the BOOT button (pressing
    it spoils a reading), anything holding SDA / SCL low at reset can stop the
    boot, and the on-board blue LED on GPIO 8 flickers with every transfer -
    used on purpose as an I2C activity indicator, not as a normal LED. The
    "LED" in session 18 is the green channel of the RGB LED. Free pins left
    for later sessions: 2 (strapping), 3, 10, 20, 21 - the 5110 (SCK, MOSI,
    CS, D/C, RST) or CAN TX / RX need them, or the buttons / pot are unplugged.
11. **CAN hardware - settled 2026-10-31.** Three TJA1050 modules with a
    divider on RXD. Check the divider values and the terminations on the
    bench before session 21.
12. **Session 18 bench test - done 2026-11-01.** l18-test (WiFi, web server,
    JSON, AHT10, MQTT on broker.hivemq.com, softAP fallback; secrets in a
    git-ignored include/secrets.h; envs c3 and s2) runs on the instructor's
    C3 esp-9494: page, /status, LED over HTTP and MQTT all work, MQTT stays up
    down to -96 dBm. Findings: the Super Mini receives 18-32 dB worse than a
    Lolin S2 Mini in the same spot; one C3 (esp-aab8 or the first board
    tested) barely sees any network. pioarduino (core 3.x) and the official
    platform conflict on one machine (shared framework-arduinoespressif32
    package, and plain "espressif32" resolves to pioarduino): the course stays
    on core 2.x with platform = platformio/espressif32.
13. **Session 19 bench test - open (2026-11-06).** Build all four M4_L19
    projects on the instructor's machine and check on the C3: (a)
    gpio_deep_sleep_hold_en() compiles on core 2.x and keeps the green LED on
    in deep sleep; (b) button A on GPIO 0 does not wake the board by itself
    (else an external 10k pull-up); (c) the node's channel change through
    esp_wifi_set_promiscuous + esp_wifi_set_channel works and node + gateway
    exchange packets (gateway on the Lolin S3). Fix the class router's channel
    for the evening.
14. **Secrets in the cohort repository - fixed 2026-11-06.**
    M4_L18/lab1-web/include/secrets.h with the class router password was
    committed, and that lab's .gitignore did not list secrets.h. Removed and
    git-ignored in commit 20b7174. The password stays in git history, so
    change it on the router if it matters.

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
buses + UART on the Pico, 13 I2C (AHT10, replaced the OPT4001 on
2026-10-12), 14 SPI (Nokia 5110), 15 logic analyzer. Details in the K4
section of the core session outline.

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
