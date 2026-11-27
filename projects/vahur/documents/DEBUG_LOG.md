# PISI - problems found and lessons learned

## Issue 1: Wrong positive/negative power connection

**What happened:** During hardware testing, the pump did not run. The owner reported that the motor became very hot and insulation melted. Later a wiring check found that one positive (+) cable was connected to a negative (-) terminal. The owner reported that after correcting the connection the system was working.

**What caused it:** A wrong power connection was identified. We have **not** independently checked whether the previously overheated pump, MOSFET modules or wiring were damaged. The old pump must not be used if any insulation or electrical part was damaged.

**What I learned:** Never identify positive and negative only by wire color. Before connecting power, check the terminal labels (`VIN+`, `VIN-`, `OUT+`, `OUT-`) and use a multimeter with all power OFF. Check motor and wire temperatures during initial tests.

**Status:** Wiring mistake reportedly corrected. Inspection of any heat-damaged parts is still required.

## Issue 2: MOSFET behavior seemed incorrect

**What happened:** During earlier tests, voltage seemed to be present at both MOSFET outputs even when the ESP32 was off. The RGB LED and MOSFET indicator seemed to react together.

**Explanation:** Some MOSFET modules switch the **negative side**. Therefore the output `OUT+` can still be at +12 V when the load is not actually switched on. The right test is voltage **across the connected load**, and whether the pump/light physically runs. An input may also float when the ESP32 is unpowered.

**Status:** Exact electrical readings not recorded. Do not claim a specific module fault without measurements.

## Issue 3: Light schedule and temperature guard

**What happened:** A restart temperature below 24 C was requested.

**Solution in code:** Lights switch off at 28 C and may resume only with temperature **strictly below 24 C** after at least 300 s of thermal pause. The normal light schedule still determines whether they actually turn on.

**Status:** Implemented in code; actual sensor test result should be entered in `TEST_RESULTS_TO_FILL.md`.

## Possible future improvements

- Add fuse and physical low-water protection.
- Test and document MOSFET input behavior when the ESP32 loses USB power.
- Build a protective case for the electronics away from water.
- Add a calibrated sensor for plant light intensity.
