# PISI - final test report (fill in measured results)

**Date:** 2026-11-27

**Firmware:** `PISI_demo_heat_restart_below_24C` version in this ZIP, unless changed later.

**Important:** This is a record sheet, NOT a report of completed tests. Only write PASS after you have tested and observed it yourself.

| Test | Expected result | Actual observation | Result |
|---|---|---|---|
| PlatformIO build | Builds without errors | Not recorded | Not tested here |
| Start without a laptop | Controller starts from USB power | Not recorded | Not recorded |
| Pump startup | ON immediately, then OFF at ~30 s | User reported system working after correcting polarity; exact timing not recorded | Verify |
| Pump next cycle | ON again at ~120 s | Not recorded | Verify |
| Grow lights | 120 s scheduled ON, 60 s OFF, 10 s fades | User reported system working; exact timings not recorded | Verify |
| RGB with pump only | Steady blue | Not recorded | Verify |
| RGB with lights only | Steady green | Not recorded | Verify |
| RGB both running | Blue 200/200, green 200/200 alternating | Not recorded | Verify |
| RGB both off | Green 100 ms every second | Not recorded | Verify |
| AHT10 disconnected | Lights OFF; red status | Not recorded | Verify safely |
| Overheat condition | Lights OFF at >=28 C | Not recorded | Do not heat electrical parts |
| Restart after overheat | Below 24 C AND at least 300 s AND light ON schedule | Not recorded | Verify only with safe method |
| Serial `s` command | Shows measured values and statuses | Not recorded | Verify |
| `p` and `w` | Stop pump/start new cycle | Not recorded | Verify |
| Pump and modules stay cool | No hot wires, connectors or motor | Prior overheating reported; needs careful recheck | IMPORTANT |
| Water circulation | No leak, overflow or dry running | Not recorded | Verify |
| Longest loop interval | Measured from `LOOP` output | **_____ us** | Fill actual value |
| Loop passes in 5 s | Measured from `LOOP` output | **_____ passes** | Fill actual value |

## Serial Monitor evidence

Copy 5-10 real lines from the Serial Monitor here (115200 baud):

```text
[PASTE YOUR REAL OUTPUT HERE]
```

## Conditions during the test

- Pump model and condition: __________________________
- Actual 12 V supply voltage across supply output: ______ V
- Temperature measured by AHT10: ______ C
- Time tested unattended: **0 minutes recommended** until fully inspected
- Wiring and polarity checked by: ______________________
- Comment / failure found: ____________________________

## Final status

- [ ] All power wiring checked with power OFF.
- [ ] No visible heat damage on pump or wires.
- [ ] Photos taken **after** the final wiring fix.
- [ ] Source code committed and pushed.
- [ ] Full 10-minute demonstration rehearsed.
- [ ] Backup video recorded.

**Tester / date:** ____________________
