# Schedule - 2026 Autumn Cohort

**Embedded Software Development in C/C++**
25 sessions x 4 academic hours, evenings.
Planned by ISO week number, not by fixed date. Anchor: week 35 is the week
starting Mon 2026-08-24.

Rhythm: one session per week in weeks 35-36 (Wed), two sessions in week 37
(Mon + Fri), then two sessions per week. From week 38 on, days are flexible:
the first session of a week lands on Mon/Wed, the second on Wed/Fri. Exact
days are agreed with the group week by week.

Fixed from week 44 on: Monday and Friday only. Wednesdays belong to another
group and another course.

Lesson IDs are per showcase module (M1_L1 = first lesson of module M1).
Because M4 and M5 interleave in the delivery plan, IDs are not strictly
sequential in calendar order. Delivery blocks refer to course-program.md.
Sessions marked "Optional block" are placeholders until the optional block
selection is decided (open decision 1 in the program).

---

## Session calendar

| #  | Lesson | Week | Day     | Delivery block                            |
|----|--------|------|---------|-------------------------------------------|
| 1  | M1_L1  | 35   | Wed     | K1: Orientation, MCU architecture, GitHub |
| 2  | M1_L2  | 36   | Wed     | K1: First firmware, PlatformIO, git flow  |
| 3  | M1_L3  | 37   | Mon     | K2: Electrical fundamentals, multimeter   |
| 4  | M1_L4  | 37   | Fri     | K2: Reading documentation, datasheets     |
| 5  | M2_L1  | 38   | Mon/Wed | K3: C - types, operators, control flow    |
| 6  | M2_L2  | 38   | Wed/Fri | K3: C - functions, pointers               |
| 7  | M2_L3  | 39   | Mon/Wed | K3: C - arrays, strings, structs, memory  |
| 8  | M3_L1  | 39   | Wed/Fri | K3: C++ - classes, wrapping a peripheral  |
| 9  | M3_L2  | 40   | Mon/Wed | K3: C++ - RAII, references, templates     |
| 10 | M3_L3  | 40   | Wed/Fri | K3: C++ - embedded restrictions, drivers  |
| 11 | M4_L1  | 41   | Mon/Wed | K4: GPIO in depth, PWM, ADC               |
| 12 | M4_L2  | 41   | Wed/Fri | K4: Timers, interrupts, ISR discipline    |
| 13 | M4_L3  | 42   | Mon/Wed | K4: UART, I2C, SPI with sensors           |
| 14 | M5_L1  | 42   | Wed/Fri | K4: Logic analyzer, bus diagnosis         |
| 15 | M3_L15 | 43   | Mon/Wed | K4: Logic analyzer, bus diagnosis         |
| 16 | M4_L16 | 44   | Mon     | S1: STM32 Black Pill, CubeMX and HAL      |
| 17 | M4_L17 | 44   | Fri     | S1: STM32, the ready-made project         |
| 18 | M4_L18 | 45   | Mon     | S3: ESP32-C3 WiFi, web server, JSON       |
| 19 | M4_L19 | 45   | Fri     | S3: Power modes, NTP, ESP-NOW             |
| 20 | M5_L20 | 46   | Mon     | S3: MQTT and network diagnosis            |
| 21 | M5_L21 | 46   | Fri     | CAN (TWAI) and bus diagnosis              |
| 22 | M4_L22 | 47   | Mon     | Z1: Cross-platform comparison             |
| 23 | M6_L23 | 47   | Fri     | Z2: Final project                         |
| 24 | M6_L24 | 48   | Mon     | Z2: Final project                         |
| 25 | M6_L25 | 48   | Fri     | Z2: Final project, demo day (2026-11-27)  |

---

## Milestones

| Week | Milestone                                            |
|------|------------------------------------------------------|
| 37   | Core K1 + K2 complete: toolchain, electronics, docs  |
| 40   | Core K3 complete: C and C++ fundamentals             |
| 42   | Core K4 complete: all peripherals and buses, halfway |
| 44   | Optional block S1 (STM32) complete                   |
| 46   | Connectivity (sessions 18-21) complete               |
| 47   | Z1 done, final project ideas approved                |
| 48   | Demo day, course ends                                |

---

## Notes

- Total span: weeks 35-48, just over 13 weeks. The course description says
  9-12 weeks; accepted as-is for this cohort.
- No Estonian public holidays fall in the Sep - Nov range.
- Updated 2026-11-06: sessions 1-18 delivered, session 19 today. Rows
  1-14 keep the original plan; the real lesson folders are named by session
  number (M2_L5 .. M2_L10 for K3, M3_L11 .. M3_L15 for K4, M4_L16 and
  M4_L17 for STM32, M4_L18 and M4_L19 for connectivity), and sessions
  11-15 shifted by one against the rows above after the Pico switch.
  Optional block choice and hour booking: see course-program.md.
- Optional blocks as taught (settled 2026-10-31): S1 STM32 in sessions
  16-17, then connectivity on the ESP32-C3 in sessions 18-21. S6 was not
  used for the half-block slot; session 19 became the second S3 session
  and was widened on 2026-11-06 to power modes, NTP and ESP-NOW.
- Slippage buffer is used up: the closing section (sessions 22-25) is not
  touched.
- Release discipline per course-conventions.md: each session's materials
  reach the cohort repository the day of the lesson or the day before,
  never earlier.
