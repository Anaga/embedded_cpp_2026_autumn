# PISI - final submission checklist

This checklist follows the course Lesson 24 presentation: 10-minute demo + 5-minute questions, English GitHub README, pin table, build steps, timing table, measured loop time, known problems and a photo if available.

## Code and GitHub

- [ ] `pio run` builds successfully using PlatformIO (pio = PlatformIO).
- [ ] The actual pin connections match `include/Config.h`.
- [ ] Both MOSFET modules, the pump and the lights run safely.
- [ ] `src/main.cpp`, `include/*.h`, `platformio.ini` and `README.md` are committed and pushed to GitHub.
- [ ] No passwords, tokens or local `.pio/` files are committed.

## Documents in this ZIP

- [x] `README.md` — purpose, parts, build instructions, behavior, timing and problems.
- [x] `docs/WIRING_AND_SAFETY.md` — wiring and safety notes.
- [x] `docs/WIRING_DIAGRAM.svg` — functional wiring picture.
- [x] `docs/PROJECT_ARCHITECTURE.md` — classes and timing.
- [x] `docs/DEMO_SCRIPT_10_MIN.md` — talk plan.
- [x] `docs/DEBUG_LOG.md` — problem and fix.
- [x] `docs/TEST_PLAN.md` — tests to do.
- [x] `docs/TEST_RESULTS_TO_FILL.md` — report sheet; **real results missing until filled**.
- [x] `docs/PHOTOS_AND_EVIDENCE.md` — evidence checklist.

## Evidence to complete yourself

- [ ] Real loop-time measurement from Serial Monitor.
- [ ] Real test results and dates filled in.
- [ ] Final device photo after wiring correction.
- [ ] 30-second backup video.
- [ ] Cold-start test with no laptop/Serial Monitor.
- [ ] Rehearsed 10-minute live demo.

**Safety note:** If there is any heat-damaged insulation, motor or electronic module, do not power it again. Replace damaged parts and recheck all polarity before testing.
