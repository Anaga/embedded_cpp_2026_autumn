# Lesson 21 demo - CAN bus through TWAI

Two ESP32-C3 Super Mini boards, two TJA1050 transceiver modules, one
500 kbit/s CAN bus. Two stations with a pair of students each; the third pair
runs the logic analyzer. One firmware for all nodes; the node number comes from
the PlatformIO environment.

## What it does

| Frame       | ID               | DLC | Data                        | When                       |
|-------------|------------------|-----|-----------------------------|----------------------------|
| Heartbeat   | 0x100 + NODE_ID  | 4   | counter (2 B), pot (2 B)    | every node, every 1 s      |
| LED command | 0x200            | 4   | target node, R, G, B        | button A, colour from pot  |
| LED off     | 0x200            | 4   | target node, 0, 0, 0        | button B                   |

Button A and B always address the OTHER node: 1 -> 2, 2 -> 1. Multi-byte
values are big-endian. Every 5 s the node prints its TWAI status (state, TEC,
REC, bus error count). Alerts for error passive, bus off and recovery are
printed when they happen; bus-off recovery is automatic.

## Wiring

Everything from sessions 18-20 stays plugged in. New pins: GPIO 10 and GPIO 3.

| TJA1050 pin   | Connect to                        | Note                                      |
|---------------|-----------------------------------|-------------------------------------------|
| VCC           | C3 `5V` pin                       | 5 V part. Not 3V3                          |
| GND           | C3 `GND`                          | Also GND between the two boards           |
| TXD           | C3 GPIO 10                        | Direct (the module has 10 ohm R2 in series) |
| RXD           | 10k -> GPIO 3, 20k from GPIO 3 to GND | RXD swings to 5 V; the divider gives 3.3 V |
| CAN_H, CAN_L  | the bus                           | Same colour for CAN_H on every module     |

```
TJA1050 RXD --[ 10k ]--+-- C3 GPIO 3      5 V x 20k / (10k + 20k) = 3.33 V
                       |
                     [ 20k ]
                       |
                      GND
```

Check the divider output with the multimeter (power on, GPIO 3 NOT yet
connected): about 3.3 V when the bus is idle. The C3 pins are rated 3.6 V
maximum; 5 V on GPIO 3 kills the pin.

Termination: our modules have the 120 ohm resistor soldered in (R1, marked
121, next to the screw terminal). With two nodes that is exactly right: each
module is one end of the bus. Powered off, the multimeter reads about 120 ohm
on a single module and about 60 ohm once the two are wired together. Do not
add a third module without removing one of the resistors.

Note on TXD: the TJA1050 datasheet wants 0.7 x VCC = 3.5 V for a HIGH on
TXD, and the C3 delivers 3.3 V. In practice every module we have works with
it. Do not "fix" this with a pull-up to 5 V - that puts 5 V on a C3 pin.

## Build and run

```
pio run -e node1 -t upload      # node2 for the other station
pio device monitor
```

Expected on each node, once both are up:

```
TWAI started: node 1, TX GPIO 10, RX GPIO 3, 500 kbit/s, mode NORMAL
<- node 2: counter 17 pot 2048
STATUS RUNNING TEC 0 REC 0 busErr 0 txFail 0 rxMissed 0 arbLost 0
```

Press button A on node 1: the RGB LED on node 2 lights with a colour that
depends on node 1's potentiometer. Button B turns it off.

### Alone at home

```
pio run -e selftest -t upload
```

`TWAI_MODE_NO_ACK` plus the self-reception flag: one board with one
transceiver (or no transceiver at all - GPIO 10 wired to GPIO 3 directly)
hears its own heartbeats and its own LED commands. Never flash the selftest
env on a board that sits on the shared bus.

## Things to try

- Pull the RXD wire of one node. That node stops seeing frames, cannot see
  its own ACK, retransmits, TEC climbs by 8 per attempt: error passive at
  128. Put the wire back and watch TEC fall again.
- Unplug the other node. The remaining one is alone: `TX fail (heartbeat):
  ESP_ERR_TIMEOUT` as the TX queue fills, TEC stops at 128 (error passive
  does not go to bus off on ACK errors - find out why in the CAN spec).
- Change `TWAI_TIMING_CONFIG_500KBITS()` to `TWAI_TIMING_CONFIG_250KBITS()`
  on ONE node. Watch what the other one reports.
- Put the logic analyzer (D0) on the GPIO 3 side of the divider, 8 MHz,
  PulseView decoder CAN at 500000. Find the ACK bit. Then repeat the
  experiments above and compare with the good capture.
- Give the heartbeat a lower ID than the LED command and send both at once:
  who wins arbitration? Check `arbLost` in the status line.

## Known gotchas

- `#error NODE_ID is not defined`: you built the bare `[env]`. Pick node1,
  node2 or selftest.
- `ESP_ERR_INVALID_STATE` on transmit: the driver is stopped or in bus off.
  Wait for the recovery message or reset the board.
- GPIO 9 is the BOOT button and GPIO 8 the on-board LED (both strapping
  pins): they are still the AHT10 bus from session 18, and they still
  flicker. Nothing to do with CAN.
- Nothing received, TEC 0, REC 0 and no errors at all: the transceiver has
  no 5 V. Measure VCC on the module.
