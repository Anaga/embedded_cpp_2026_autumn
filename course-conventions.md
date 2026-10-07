# Course Conventions

Working agreements for "Embedded Software Development in C/C++".
Carried over from the 2026 stream and kept current during the 2026 autumn
stream (last update 2026-10-07, after session 12). This file is the starting
context for a new course project - read it before producing any material.

---

## 1. Language and characters

- Course language is English. Every student-facing artifact is in English:
  slides, source code, comments, README files, homework text, commit messages.
- Conversation with the instructor may be in Russian. Deliverables never are.
- **ASCII only.** No emoji. No em-dashes. No box-drawing characters. No smart
  quotes. No Unicode arrows.
  - Use `->` not the arrow glyph, `-` not the em-dash, `"` not curly quotes.
  - This applies to Serial output and code comments as much as to slides.
  - Reason: mixed toolchains and terminals mangle non-ASCII, and students then
    debug the terminal instead of the firmware.
- Enforcement: `slides-theme.js` calls `assertAscii()` on every string that
  reaches a slide and throws on violation. For source files, grep before
  shipping:

```
grep -rPn '[^\x00-\x7F]' src/ include/ homework/ README.md
```

---

## 2. Hardware inventory

| Board | MCU | Notes |
|---|---|---|
| ESP32-C3 Super Mini | ESP32-C3 (RISC-V) | PlatformIO board id `lolin_c3_mini`. Reference platform for K1-K3 and session 11. |
| Raspberry Pi Pico | RP2040 (2x Cortex-M0+) | PlatformIO board id `pico`. Reference platform from session 12 (K4 on). Original board, no WiFi. |
| NUCLEO-F401RE | STM32F401 (Cortex-M4) | On-board ST-Link. Primary STM32 board. |
| NUCLEO-F334R8 | STM32F334 (Cortex-M4) | Used for USART / I2C work. |
| NUCLEO-F446RE | STM32F446 (Cortex-M4) | Used for SPI work. |
| TENSTAR RP2350-USB | RP2350A | Dongle form factor. No general-purpose on-board GPIO LED. |

Notes on the Picos used in the 2026 autumn stream:

- Four boards, shipped **without headers**. Students solder the 2x20 headers
  in session 12, using the breadboard as a jig, and check by eye and by
  continuity before plugging in USB.
- `INFO_UF2.TXT` on the boot drive reads "UF2 Bootloader v2.0", i.e. RP2040
  revision B1 (B2 reads v3.0). This is not a Pico 2. The bootloader lives in
  ROM and cannot be updated.
- Spare boards: buy 1-2 (instructor demo, replacement for a board that dies in
  class).

Peripherals and instruments: OPT4001 ambient light sensor (I2C), Nokia 5110
LCD (SPI), RGB LED 500RGB4E (common anode), 10k potentiometer, two push
buttons, WS2812 / NeoPixel strips and a 10-LED ring, multimeter, FX2LP-based
USB logic analyzer.

### Standard pinouts

Keep these stable across lessons. Students should not have to re-learn wiring
every session.

**ESP32-C3 Super Mini** (as wired in the 2026 autumn stream, K3 and session 11)

| Function | Pin | Notes |
|---|---|---|
| Button A | GPIO 0 | `INPUT_PULLUP`, button to GND, active low |
| Button B | GPIO 1 | `INPUT_PULLUP`, button to GND, active low |
| Potentiometer | GPIO 4 (ADC1) | 10k, with 3.3k in series to 3V3: without it the ADC clips at about 2.9 V |
| RGB LED R / G / B | GPIO 5 / 6 / 7 | Common anode to 3V3, 100 ohm per channel. LOW turns a channel ON |
| On-board LED | GPIO 8 | Blue, active low. **Strapping pin** |
| BOOT button | GPIO 9 | Active low. **Strapping pin** |

GPIO 2, 8 and 9 are strapping pins with boot-mode implications. Say so
wherever they appear in course material. The previous stream used GPIO 8 for
WS2812 data; it shares that pin with the on-board LED.

**Raspberry Pi Pico (RP2040)** - settled 2026-10-07 for K4 and beyond

The buses sit on the Arduino-Pico default pins, so `Serial1`, `Wire` and `SPI`
work without `setTX()` / `setSDA()`. Everything below can be wired at the same
time; no pin is used twice.

| Function | GPIO | Physical pin | First used |
|---|---|---|---|
| UART0 (`Serial1`) TX / RX | GP0 / GP1 | 1 / 2 | session 12 |
| GND next to the UART | - | 3 | session 12 |
| I2C0 (`Wire`) SDA / SCL - OPT4001 | GP4 / GP5 | 6 / 7 | session 13 |
| UART1 (`Serial2`) TX / RX | GP8 / GP9 | 11 / 12 | session 12 homework |
| RGB LED R / G / B | GP10 / GP11 / GP12 | 14 / 15 / 16 | later |
| Button A / B | GP14 / GP15 | 19 / 20 | later |
| SPI0 MISO / CS / SCK / MOSI - Nokia 5110 | GP16 / 17 / 18 / 19 | 21 / 22 / 24 / 25 | session 14 |
| Nokia 5110 D/C / RST | GP20 / GP21 | 26 / 27 | session 14 |
| On-board LED | GP25 | - | session 11 homework |
| Potentiometer (ADC0) | GP26 | 31 | session 12 |
| AGND / 3V3 OUT for the potentiometer | - | 33 / 36 | session 12 |

- No 3.3k series resistor on the Pico potentiometer: the RP2040 ADC reads the
  full 0 .. 3.3 V. The resistor was a C3 workaround.
- Common anode RGB LED goes to 3V3 OUT (pin 36), as on the C3.

**RP2350 (TENSTAR)**

| Function | Pin |
|---|---|
| On-board WS2812 | GP22 |
| 10-LED NeoPixel ring | GP15 |

**STM32 NUCLEO**

USART2 on PA2 / PA3 is the ST-Link virtual COM port and is pre-configured by
CubeMX. Do not remap it.

---

## 3. Toolchain

- **Primary:** VS Code + PlatformIO. This is what students use for everything.
- **ESP32-C3, RP2040 and RP2350:** Arduino framework.
- **STM32:** HAL, polling mode preferred over interrupts for first exposure.
  STM32CubeIDE 2.1.1 for the instructor demo; students use the STM32 VS Code
  Extension.
- **Logic analysis:** PulseView with the `fx2lafw` driver. Budget AliExpress
  analyzers are near-universally Cypress FX2LP clones. On Windows, students
  need Zadig to replace the driver before PulseView sees the device. Budget a
  full session slot the first time this comes up, or make the install
  homework the session before.

### ESP32-C3 (`platform = espressif32`)

- PlatformIO ships **Arduino-ESP32 core 2.x**, not 3.x. Most examples found
  online are 3.x and do not compile here - translate them, do not paste them.
  - PWM uses channels: `ledcSetup` / `ledcAttachPin` / `ledcWrite(channel,
    duty)`. `ledcAttach` does not exist in 2.x.
  - Hardware timers: `timerBegin(num, divider, countUp)`,
    `timerAttachInterrupt(timer, fn, false)`, `timerAlarmWrite`,
    `timerAlarmEnable`. Two timers (0 and 1). Pass `false` for edge: edge
    interrupts are not supported and the core falls back to level anyway.
- The default C++ standard is gnu++11; set `build_unflags = -std=gnu++11` and
  `build_flags = -std=gnu++17`.
- `lolin_c3_mini` already enables USB CDC on boot, so `Serial` is USB with no
  extra flags.

### RP2040 and RP2350

- **earlephilhower Arduino-Pico core via maxgerhardt's platform fork.** Native
  pico-sdk support in PlatformIO is not stable enough for a classroom. This
  choice is deliberate - do not "fix" it back to pico-sdk without re-testing
  on a clean machine.

```
[env:pico]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = pico
framework = arduino
board_build.core = earlephilhower
monitor_speed = 115200
```

- Arduino-Pico already compiles as gnu++17: no extra build flags.
- **Windows, before the first build:** enable long paths
  (`git config --system core.longpaths true` and the `LongPathsEnabled`
  registry value, then reboot), or the install fails with "Filename too
  long". PlatformIO needs git on PATH to fetch the platform.
- **First upload:** hold BOOTSEL while plugging in USB. Later uploads reset the
  board through its USB serial port.
- **Windows, RP2040 only:** picotool needs a driver for the boot interface,
  otherwise the upload stops with "picotool was unable to connect". Zadig ->
  **RP2 Boot (Interface 1)** -> WinUSB. Never change Interface 0 (that is the
  `RPI-RP2` drive). The RP2350 does not need this.
- **Fallback:** in BOOTSEL mode, copy `.pio/build/pico/firmware.uf2` onto the
  `RPI-RP2` drive by hand.
- `Serial` is USB CDC (baud ignored); `Serial1` = UART0 (GP0 / GP1);
  `Serial2` = UART1 (GP8 / GP9).
- `analogRead()` returns 10 bits (0 .. 1023) by default;
  `analogReadResolution(12)` for 0 .. 4095.
- `LED_BUILTIN` is GP25 on the original Pico. Without `pinMode(..., OUTPUT)`
  the LED stays dark while Serial keeps printing - the most common first bug.
- Clock: `F_CPU` (build time, from `board_build.f_cpu`), `rp2040.f_cpu()` (run
  time), `frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS)` (measured by the
  on-chip frequency counter, needs `<hardware/clocks.h>`).

### Serial monitor for interactive work

When students type into the monitor (UART lessons), add:

```
monitor_echo = yes
monitor_filters = send_on_enter
```

Without `send_on_enter` every key is sent as it is pressed. Lines end with
CR LF (`monitor_eol` default), so line parsers drop `\r` and split on `\n`.

---

## 4. Code deliverables

### Lesson package (since lesson 07, at the students' request)

```
lesson-NN/
  slides/             generator .js and built .pptx
  demo/               PlatformIO project, complete and working
    platformio.ini
    src/main.cpp
    README.md         what it does, wiring, how to run, things to try
  homework/
    README.md         the full task specification
    platformio.ini    one commented src_dir line per student
  README.md           short: what the lesson covers, pointers to the above
```

No scaffold and no solution in the package. If the instructor wants a
reference solution, it goes to `solution/` in `embedded-cpp-materials` only.

Demo code covers what the slides show. When the in-class lab is the point of
the session, the lab itself is not in the demo - otherwise it becomes
copy-paste.

### Homework project layout

- `homework/platformio.ini` has a `[platformio]` block with one `src_dir` line
  per student (`sven/src`, `viktorija/src`, `anton/src`, `vahur/src`), all
  commented out, placed BEFORE `[env:...]` so `build_flags` stay in the env
  section.
- Each student uncomments only their own line and does not commit
  `platformio.ini` - four different uncommented lines would conflict. They
  commit only their own folder.
- Students keep their headers in their own `<name>/src` folder, not in a
  shared `include/`.

### Style

- `stdint.h` fixed-width types throughout: `uint8_t`, `int32_t`, never `int`
  for anything hardware-facing.
- Explicit unsigned suffixes on constants: `500U`, `3U`.
- C++17 (see the toolchain notes per platform).
- No magic numbers in function bodies - named constants at the top of the file.
- `printf` with fixed-width types: `%lu` plus `static_cast<unsigned long>`.
  `uint32_t` is `unsigned long` on these compilers, so `%d` is wrong.
- No `delay()` in `loop()` once K3 is done; non-blocking `millis()` patterns
  and `enum class` state machines instead.

### Scaffold pattern (when a scaffold is used)

- The hardest infrastructure is pre-written and works out of the box: WiFi
  setup, driver init, display handling, JSON plumbing.
- Students implement the logic, marked by clearly labelled `// TODO:` comments.
- Scaffolds are minimal, not over-hinted. A TODO states the goal, not the
  implementation.
- Intentional bugs are used selectively as diagnostic exercises - for example
  `||` where `&&` belongs in dead-zone logic. Use sparingly; one per assignment
  at most, and only after students have a debugging tool that can find it.

---

## 5. Slide decks

Generated with PptxGenJS. Theme and helpers live in `slides-theme.js`.

- Theme name: **Midnight Executive**. Navy backgrounds, ice-blue accents,
  Trebuchet MS headings, Calibri body, Consolas for code.
- Typical deck length: 12-18 slides depending on topic depth, up to about 24
  with section dividers.
- Structure: title -> goals -> content sections -> summary with homework.
- Speaker notes carry timing, answers to in-slide questions and exercises.

### Known gotchas

1. **PptxGenJS does not clip overflowing text.** A too-tall code block silently
   runs off the slide. The XML is valid and nothing warns you. `addCode()`
   computes required height and throws instead of shipping a broken slide.
2. **LibreOffice renders Consolas taller than nominal.** Effective line height
   exceeds what the point size implies, so heights calculated for PowerPoint
   come out short. `CODE_LINE_FACTOR` in the theme file compensates. It is
   deliberately conservative - it errs toward too much space.
3. **QA by rendering, always.** Valid XML proves nothing about layout:

```
soffice --headless --convert-to pdf deck.pptx --outdir /tmp/qa
pdftoppm -png -r 70 /tmp/qa/deck.pdf /tmp/qa/slide
```

Then look at every slide. Iterating on layout fixes is routine, not a sign
something went wrong.

4. **Card grids must be computed from index**, never hand-placed. Hand-placed
   cards drift the instant the card count changes.
5. **Code below 10 pt is hard to read on a projector.** If `addCode()` only
   fits at 9 pt, consider splitting the slide.

### Dependencies

The slide environment resets between days: copy `slides-theme.js` next to the
generator and install the dependencies before building.

```
npm install pptxgenjs react react-dom react-icons sharp
```

Verify the theme still works after any edit: `node slides-theme.js` writes
`theme-selftest.pptx` exercising every helper.

---

## 6. Distribution

Two repositories. Materials outlive a cohort; a cohort's repository does not.

| Repository | Visibility | Contents | Lifetime |
|---|---|---|---|
| `embedded-cpp-materials` | Private, instructors only | All lessons including unwritten ones, plus solutions | Permanent |
| `embedded-cpp-2026-autumn` | Students as collaborators | Only what has already been taught, plus student work | One cohort, then archived |

```
embedded-cpp-materials/
  program/          course-program.md, course-conventions.md
  tools/            slides-theme.js, release.sh (later)
  lessons/
    lesson-NN/
      slides/       generator .js and built .pptx
      demo/         PlatformIO project, complete
      homework/     README.md task spec, platformio.ini
      solution/     NEVER published
      README.md

embedded-cpp-2026-autumn/
  README.md
  lesson-NN/        slides.pptx, README.md, demo/, homework/
  students/<username>/
```

### Release discipline

Content reaches the cohort repository **the day of the lesson or the day
before**, never earlier. Students who can see two months of homework start
optimising for the wrong thing.

Three constraints make this work:

1. **Git history is irreversible.** A future lesson pushed by accident and
   deleted a minute later is still in the history and still retrievable by any
   student. The only real fix is a force push that rewrites history, which
   breaks every existing clone. Protection happens before the commit, not
   after.
2. **No git link between the repositories.** Not a fork, not a submodule, not a
   shared remote. A fork carries the whole history, which means the whole
   course. Transfer is a plain file copy followed by a commit in the cohort
   repository. Deliberately boring.
3. **`solution/` is never published**, not even after a deadline. The same
   tasks go to the next cohort, and published solutions are indexed by search
   engines within weeks.

### Cohort repository setup

- Created live in front of the students during session 1, with the "Add a
  README file" option checked. That creates the `main` branch immediately, so
  the first push does not turn into a discussion about `main` vs `master`.
- Students are added under Settings -> Collaborators and must accept the email
  invitation. This is the step people get stuck on.
- Collect GitHub usernames before anyone leaves session 1.
- Students authenticate with SSH keys; HTTPS plus a personal access token is
  the documented fallback.
- **GitHub Classroom was evaluated and not used.** For a group of three it adds
  more configuration than value, and manual review was the right call at this
  size.
- A `release.sh` script to automate the copy is deferred until the manual
  workflow has run a few times and it is clear what actually repeats.

### Naming

`embedded-cpp-<year>-<season>` for cohorts. Not a bare year: two cohorts can
run in the same calendar year, and the previous stream already used
`embedded-course-2026`. Season rather than month because a course starting in
August runs into November.

## 7. Working process

- **Scope is confirmed before anything is generated.** Structured multi-select
  questions first, artifacts second. No speculative decks.
- Each session starts by discussing the lesson plan against
  `course-program.md`; the program is updated when the plan changes.
- The phrase "as usual" means the standard package: slides, then demo code,
  then the homework task (`homework/README.md` + `platformio.ini`), then the
  lesson README.
- Order matters: slides first, code second. The deck fixes the scope; writing
  code first tends to expand it.
- Materials get revised after being taught. Real classroom problems are folded
  back into student-facing documentation rather than tracked separately - if
  three students hit the same wall, that wall belongs in the README.
