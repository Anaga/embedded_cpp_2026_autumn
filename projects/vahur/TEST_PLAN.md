# PISI demo test checklist

- [ ] Build with PlatformIO using Arduino-ESP32 core 2.x.
- [ ] Verify the GPIO map matches the actual wired board before powering 12 V loads.
- [ ] Confirm MOSFET control is active-HIGH at 3.3 V.
- [ ] Check the pump PWM 65%-100% ramp with the pump submerged.
- [ ] At t=0: pump runs, light starts 10-s fade if AHT10 is OK.
- [ ] At t=30s: pump OFF; grow lights ON -> steady GREEN.
- [ ] At t=120s: pump ON, grow lights OFF (start night). RGB BLUE steady.
- [ ] At t=150s: pump OFF, grow lights OFF. Green 100 ms heartbeat each second.
- [ ] At t=180s: grow lights restart with fade; green steady, until pump next starts t=240s.
- [ ] At t=240s: both ON; RGB BLUE 200ms -> OFF 200ms -> GREEN 200ms -> OFF 200ms.
- [ ] Observe light schedule at 0, 120, 180, 300, 360 seconds.
- [ ] At t=360s: pump starts and light starts new day; both-on pattern repeats.
- [ ] Verify `s` prints current values, `p` stops, `w` starts pump.
- [ ] Verify AHT10 missing -> red 200ms flashing, lights OFF.
- [ ] Measure longest loop interval from the Serial report.
- [ ] Cold boot works without laptop; USB Serial is not required for automation.

These timeline values assume no manual commands, normal temperature, working AHT10 and no faults. First light is off during the initial sensor detection and PWM fade.
