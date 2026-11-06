# Lab 3 gateway - ESP-NOW in, web page out

Runs on the instructor's board during the lesson. Students read it; they
do not need to run it.

It joins the class router as a WiFi station, so it listens on the
router's channel. It receives `NodeMsg` packets from the sleeping nodes,
stamps them with the local time (NTP) and shows them on a web page.

## Before the lab

1. Copy `include/secrets.example.h` to `include/secrets.h`: the class
   router SSID and password.
2. Build, upload, open the monitor. Write the block it prints on the
   whiteboard:

```
---- for the whiteboard ----
CLASS_SSID  = "the-class-router"
GATEWAY_MAC = {0x58, 0xCF, 0x79, 0x00, 0x00, 0x00}
channel 6, page http://192.168.1.50/
----------------------------
```

3. Fix the router's channel in its settings for the evening (no "auto"),
   so it does not move during the lab.

## What it prints

```
19:12:27  esp-aab8   22.4 C  38.0 %  seq     2  awake   160 ms
```

## Notes for the lesson

- `WiFi.setSleep(false)`: with modem sleep the radio is off between
  beacons and packets are lost. Show what happens without it if there is
  time.
- `onRecv()` only copies into a queue under a critical section;
  `loop()` prints and updates the table.
- **Missed** counts gaps in `seq`: lost packets, or a node that was reset
  (its `seq` starts again).
- The page greys out a node not heard for more than 60 s.
- `msg.h` is the same file as in `lab3-node`.
