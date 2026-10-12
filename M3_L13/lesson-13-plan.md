# Lesson 13 - I2C and the AHT10: lesson plan

Instructor document. Not published to the cohort repository.

- Session 13 of 25, K4, booked to M4. 4 academic hours = 180 min.
- Platform: Raspberry Pi Pico (RP2040), Arduino-Pico core.
- Sensor: AHT10 temperature and humidity, I2C address 0x38.
  3 modules: one per pair (2 pairs), one for the instructor.
- Replaces the OPT4001 plan (sensor no longer available, 2026-10-12).

## Learning goals

By the end of the session a student can:

1. Explain why I2C lines are open-drain and why the bus needs pull-ups, and
   check with a multimeter whether a module already has them.
2. Write out an I2C write and an I2C read transaction byte by byte: start,
   address + R/W, ACK / NACK, data, stop.
3. Tell a register-based device (pointer + repeated start) from a
   command-based one, and say which kind the AHT10 is.
4. Find a device with an I2C scanner and diagnose an empty scan.
5. Read the AHT10 by hand with `Wire`: init, trigger, wait without
   `delay()`, read 6 bytes, unpack two 20-bit fields, convert.
6. Use a library for the same job and name what it hides.

## Instructor preparation (before the session)

- [ ] Open decision 7: measure the pull-ups on all 3 modules (unpowered,
      SDA -> VIN and SCL -> VIN). Note the value and whether there is an LDO
      (3-pin SOT-23 near the header).
- [ ] If any module has no pull-ups: bring 4.7k resistors, two per module,
      to 3V3 OUT.
- [ ] Run one module end to end on a Pico: scanner finds 0x38, by-hand read
      and library read agree.
- [ ] Check each module answers at 0x38. If the calibrated bit never sets
      after `0xE1`, the chip may be an AHT20 sold as AHT10: try `0xBE`.
- [ ] Build the demo once on the classroom network so PlatformIO has
      fetched Adafruit AHTX0 and its dependencies (BusIO, Unified Sensor).
- [ ] Release lesson-13 to the cohort repository the day of the session or
      the day before.

## Timeline

| Time | Min | Block | Mode |
|---|---|---|---|
| 0:00 | 20 | Homework check: two UARTs on one Pico | discussion |
| 0:20 | 25 | Open-drain and pull-ups, measure the module | slides + hands-on |
| 0:45 | 10 | Break | |
| 0:55 | 30 | The I2C protocol on paper | slides + paper exercise |
| 1:25 | 15 | Wiring and the I2C scanner | pairs |
| 1:40 | 10 | Break | |
| 1:50 | 50 | Lab: AHT10 by hand | pairs |
| 2:40 | 15 | Library version and what it hides | demo + pairs |
| 2:55 | 5 | Summary and homework | slides |

### 0:00 - Homework check (20 min)

Session 12 homework: the POT? / POT protocol on one Pico, pot side on
`Serial1`, LED side on `Serial2`.

- Who got it running? Who saw it freeze?
- The hidden lesson: a blocking wait for the reply deadlocks, because the
  side that should answer runs in the same `loop()` and never gets a turn.
- The fix is the same `enum class` state machine as in class: send, return,
  check for the reply on the next pass. Keep this in mind: the AHT10 needs
  the same pattern today.

### 0:20 - Open-drain and pull-ups (25 min)

Content:

- Push-pull output (what `digitalWrite` does) vs open-drain: a pin that can
  only pull low or let go.
- Two push-pull outputs on one wire, one HIGH and one LOW: a short circuit.
  Open-drain makes "anyone may pull low" safe. That is why I2C uses it.
- Let go -> nobody drives the line -> it floats. The pull-up resistor makes
  "let go" mean HIGH.
- Tie back to session 3 and to `INPUT_PULLUP` on the K3 buttons: same idea.
- Pull-up value is a trade-off: smaller = faster edges, more current when
  low. 4.7k is the usual starting point at 100 kHz.

Hands-on (pairs, 10 min):

- Module unpowered. Multimeter on resistance: SDA -> VIN, SCL -> VIN.
- Around 4.7k or 10k -> pull-ups on board. Open -> none, add external.
- Look for the LDO. If present, the pull-ups may go to its output, not VIN.
- Collect the results on the board. Safety rule from this: supply the module
  from 3V3 OUT only. With pull-ups to VIN and VIN at 5 V, SDA / SCL would sit
  at 5 V, and RP2040 pins are not 5 V tolerant.

### 0:55 - The I2C protocol on paper (30 min)

Content:

- Two wires, SDA and SCL, many devices, one master. Each device has a 7-bit
  address.
- Start and stop: SDA changes while SCL is high. Everywhere else SDA changes
  only while SCL is low.
- The first byte: 7-bit address shifted left, R/W in bit 0.
  0x38 -> 0x70 for write, 0x71 for read.
- ACK / NACK: the receiver pulls SDA low in the 9th clock. NACK on the
  address byte = nobody home.
- Register-based devices (most sensors): write the register pointer, repeated
  start, read. Shown as theory, with a generic diagram.
- Command-based devices: write a command, wait, read the raw result. The
  AHT10 is this kind. No register pointer, no repeated start.
- `Wire` mapping: `beginTransmission(addr)`, `write(byte)`,
  `endTransmission()` (returns 0 on ACK, non-zero on failure),
  `requestFrom(addr, n)` (returns the number of bytes received), `read()`.

Paper exercise (pairs, 8 min): write out on paper what goes over the wire
for "trigger a measurement" and for "read the result".

Answer (speaker notes):

```
Write: S | 0x70 A | 0xAC A | 0x33 A | 0x00 A | P
Read:  S | 0x71 A | status A | b1 A | b2 A | b3 A | b4 A | b5 N | P
```

In the read, the master ACKs every byte except the last one, then NACKs to
say "enough".

### 1:25 - Wiring and the I2C scanner (15 min)

Wiring (from the pin map, course-conventions.md):

| AHT10 | Pico |
|---|---|
| VIN | 3V3 OUT (pin 36) via the breadboard rail |
| GND | GND |
| SDA | GP4 (pin 6) |
| SCL | GP5 (pin 7) |

- Scanner: try every address 1..126, report the ones that ACK. It is in the
  demo; students run it as-is.
- Expected: one device at 0x38.
- Empty scan, ask the class for the checklist before showing it: power
  (measure 3.3 V at VIN), GND shared, SDA / SCL swapped, no pull-ups, wrong
  pins in code.

### 1:50 - Lab: AHT10 by hand (50 min, pairs)

Not in the demo: this is the point of the session.

Stage 1 - init and status (10 min)
- Send `0xE1 0x08 0x00`, wait, read 1 byte (status).
- Print the status in hex and as bits. Bit 3 = calibrated, bit 7 = busy.
- If bit 3 is not set: re-check wiring, soft reset `0xBA` and retry.

Stage 2 - trigger, wait, read (15 min)
- Send `0xAC 0x33 0x00`, wait at least 80 ms, read 6 bytes, print them in
  hex.
- The wait is an `enum class` state machine, not `delay()`:
  IDLE -> (send trigger, remember millis) -> MEASURING -> (80 ms passed)
  -> read -> IDLE. One measurement per second.
- Busy bit still set after 80 ms: stay in MEASURING and try again.

Stage 3 - unpack and convert (20 min)
- Humidity raw: 20 bits = b1, b2, high nibble of b3.
- Temperature raw: 20 bits = low nibble of b3, b4, b5.
- RH = raw / 2^20 * 100, T = raw / 2^20 * 200 - 50.
- Use `uint32_t` for the raw values. Shifts and masks, as in K3.
- Check: breathe on the sensor (humidity jumps within seconds), hold it
  between two fingers (temperature climbs slowly).

Stretch (fast pairs):
- Compute in integer hundredths instead of float (the RP2040 has no FPU).
  Watch for overflow: raw * 20000 does not fit in 32 bits.

Trap to call out before the lab starts: most AHT10 examples online are
really AHT20 code (`0xBE`, 7 bytes with CRC). Translate, do not paste.

### 2:40 - Library version and what it hides (15 min)

- `lib_deps = adafruit/Adafruit AHTX0` in `platformio.ini`. The library sends
  `0xE1`, so it handles the AHT10.
- Same reading in a few lines. Compare the numbers with the by-hand version.
- Open the library source together: `getEvent()` waits for the busy bit
  with `delay(10)` in a loop, no timeout.
- Measure it, using the method from session 11: `micros()` before and after
  `getEvent()`. Expect about 80 ms during which `loop()` does nothing else.
- Discussion: when is that acceptable (a simple logger) and when is it not
  (blinking, UART protocol, display refresh). Which is why the homework
  wraps the by-hand code in a non-blocking class.

### 2:55 - Summary and homework (5 min)

- Open-drain + pull-ups, address byte and ACK, command vs register devices,
  the scanner, the state-machine wait, the library trade-off.
- Next session: SPI and the Nokia 5110; the AHT10 reading goes on the
  display.

## Homework

Real hardware. Modules go home with the students who do the homework.

Task: wrap the by-hand code into a class `Aht10`.

- Non-blocking interface: `begin()`, `start()`, `poll()` returning an
  `enum class` status (for example BUSY / READY / NO_SENSOR), getters for the
  last temperature and humidity.
- A reading every 2 s on Serial, `loop()` never blocks.
- Sensor unplugged (address NACK): print "sensor missing" once, retry every
  2 s, recover by itself when the sensor is plugged back in. Same fail-safe
  idea as "no reply" in session 12.
- Header and source in the student's own `<name>/src` folder.

## Package mapping

| Folder | Contents |
|---|---|
| slides/ | deck as outlined below |
| demo/ | I2C scanner and the library reading with the `micros()` timing; no by-hand driver |
| homework/ | README.md with the task above, platformio.ini with the four commented src_dir lines (no library: the class is built on `Wire`) |
| solution/ | (materials repo only, optional) reference `Aht10` class |

## Draft slide outline (about 18 slides)

1. Title
2. Goals
3. Homework recap: the deadlock and the fix
4. Section: Pull-ups
5. Push-pull vs open-drain
6. Why I2C needs pull-ups (and how to check your module)
7. Section: The protocol
8. Bus, addresses, start and stop
9. The address byte: 0x38 -> 0x70 / 0x71
10. ACK and NACK
11. Register-based vs command-based devices
12. Exercise: write out the AHT10 transactions
13. Wire API in five calls
14. Wiring and the scanner
15. AHT10: commands and the 6-byte result
16. Unpacking the 20-bit fields
17. The library: what it hides (blocking wait, measured)
18. Summary and homework

## Risks and fallbacks

| Risk | Fallback |
|---|---|
| Module has no pull-ups | 4.7k to 3V3 OUT on SDA and SCL |
| Calibrated bit never sets | Soft reset `0xBA`; if still not set, try the AHT20 init `0xBE` |
| Empty scan | Checklist from the scanner block; swap the module with the instructor one |
| PlatformIO cannot fetch the library in class | Pre-fetched during preparation; otherwise skip to the timing discussion on the instructor build |
| Lab runs long | Stage 3 conversion finishes as part of the homework |
