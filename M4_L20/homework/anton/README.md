# Homework 20 - Automation logic and feedback

Sensor + actuator loop with fail-safe.

## Task

- Read AHT10 every 2 s (non-blocking schedule).
- Keep temperature within a band around the setpoint.
- If the sensor stops answering: heater OFF, report on Serial and display.

## Build

```bash
cd M4_L20/homework/anton
pio run
pio run -t upload
```

## Hand-in

Commit `homework/anton/M4_L20` and push.