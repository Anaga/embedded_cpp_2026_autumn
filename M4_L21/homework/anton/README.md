# Homework 21 - State Machine architecture

A non-blocking finite state machine for autonomous devices.

## States

OFF -> IDLE -> MANUAL -> AUTONOMOUS -> SAFE

SAFE is a terminal state: the device stays there until explicitly reset to IDLE.

## Build

```bash
cd M4_L21/homework/anton
pio run
pio run -t upload
```

## Hand-in

Commit `homework/anton/M4_L21` and push.