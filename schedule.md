# Schedule - 2026 Autumn Cohort

**Embedded Software Development in C/C++**
25 sessions x 4 academic hours, evenings.
Planned by ISO week number, not by fixed date. Anchor: week 35 is the week
starting Mon 2026-08-24.

Rhythm: one session per week in weeks 35-36 (Wed), two sessions in week 37
(Mon + Fri), then two sessions per week. From week 38 on, days are flexible:
the first session of a week lands on Mon/Wed, the second on Wed/Fri. Exact
days are agreed with the group week by week.

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
| 15 | M4_L4  | 43   | Mon/Wed | Optional block 1                          |
| 16 | M4_L5  | 43   | Wed/Fri | Optional block 1                          |
| 17 | M4_L6  | 44   | Mon/Wed | Optional block 2                          |
| 18 | M4_L7  | 44   | Wed/Fri | Optional block 2                          |
| 19 | M4_L8  | 45   | Mon/Wed | Optional block 3 (or S6, 4 h)             |
| 20 | M5_L2  | 45   | Wed/Fri | S5: Testing and CI                        |
| 21 | M5_L3  | 46   | Mon/Wed | S5: Testing and CI                        |
| 22 | M4_L9  | 46   | Wed/Fri | Z1: Cross-platform comparison             |
| 23 | M6_L1  | 47   | Mon/Wed | Z2: Final project                         |
| 24 | M6_L2  | 47   | Wed/Fri | Z2: Final project                         |
| 25 | M6_L3  | 48   | Mon/Wed | Z2: Final project, demo day               |

---

## Milestones

| Week | Milestone                                            |
|------|------------------------------------------------------|
| 37   | Core K1 + K2 complete: toolchain, electronics, docs  |
| 40   | Core K3 complete: C and C++ fundamentals             |
| 42   | Core K4 complete: all peripherals and buses, halfway |
| 45   | Platform optional blocks complete                    |
| 46   | All taught material complete, project phase begins   |
| 48   | Demo day, course ends                                |

---

## Notes

- Total span: weeks 35-48, just over 13 weeks. The course description says
  9-12 weeks; accepted as-is for this cohort.
- No Estonian public holidays fall in the Sep - Nov range.
- Slippage buffer: sessions 15-19 (optional blocks). If the group falls
  behind, drop or shorten an optional block. The core (sessions 1-14) and
  the closing section (sessions 22-25) are not touched.
- Session 19 is a half-block slot: 5 optional platform sessions cover 20
  hours against two full 8-hour blocks. Either S6 (Actuators, 4 h) goes
  here, or the second session of a chosen block does. Decide together with
  the optional block selection.
- Release discipline per course-conventions.md: each session's materials
  reach the cohort repository the day of the lesson or the day before,
  never earlier.
