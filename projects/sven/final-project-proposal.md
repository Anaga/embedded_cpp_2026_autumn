# Final project: Sounder

Author: Sven Kirsimae
Date: 2026-11-16
Status: draft

## 1. One-sentence summary

Sounder is a theremin-style instrument that uses two ultrasonic distance sensors to let my kids shape synthesized sounds with their hands and hear them through a Bluetooth speaker.

## 2. Why this project

I want to give my kids a playful way to experiment with sound.

## 3. Platform

Freenove ESP32-WROVER (already on hand). Demo sounds already play over Bluetooth on the JBL Flip 4.

## 4. Hardware

| Part | Interface | Have it? | Source |
|------|-----------|----------|--------|
| Freenove ESP32-WROVER | GPIO, Bluetooth | Yes | Own |
| 2 x HC-SR04-style ultrasonic sensors | GPIO (Trig/Echo) | Yes | Own |
| Resistors for 2 Echo voltage dividers | GPIO level shifting | Yes | Own |
| Prototype wiring and USB cable | Power, GPIO | Yes | Own |
| JBL Flip 4 speaker | Bluetooth audio | Yes | Own |
| Pushbutton (stretch) | GPIO | Yes | Own |

## 5. Pin budget

Planned assignments for the Freenove ESP32-WROVER.

| Pin | Function | Notes |
|-----|----------|-------|
| GPIO32 | Pitch sensor Trig | Output |
| GPIO33 | Pitch sensor Echo | Input via voltage divider |
| GPIO13 | Volume sensor Trig | Output |
| GPIO14 | Volume sensor Echo | Input via voltage divider |
| GPIO25 | Sound switch (stretch) | Button to GND, internal pull-up |
| 5V | Both sensors VCC | USB-powered board |
| GND | Both sensors and dividers | Common ground |

## 6. Behaviour

1. Connect to the speaker on startup. Stay silent until both sensors have a valid reading.
2. Read the sensors in turn. One controls volume, and the other controls pitch for the first sound.
3. When a hand leaves range, keep the last valid value for that sensor. If the speaker disconnects, wait for a connection.

## 7. Interfaces and protocols

| Channel | Direction | Format |
|---------|-----------|--------|
| GPIO Trig | Out | High pulse of at least 10 us |
| GPIO Echo | In | High pulse width gives the round-trip time |
| Bluetooth Classic A2DP | Out | SBC audio stream to the JBL Flip 4 |

Build on the existing [Bluetooth audio firmware](README.md), which uses Arduino in PlatformIO with AudioTools 1.2.6 and ESP32-A2DP 1.8.11.

## 8. Minimum and stretch

Minimum:
- Play one synth sound on the JBL Flip 4 with hand-controlled pitch and volume.

Stretch:
- Add a second synth sound, then use the pushbutton to switch between sounds.

## 9. Open questions and risks

- Test whether holding the last value feels right when a hand leaves range.
- Check for interference between sensors and test Bluetooth latency while playing.

## 10. Plan for the three sessions

| Session | Date | Goal at the end of the evening |
|---------|------|--------------------------------|
| 23 | Fri 2026-11-20 | Wire both sensors and test their readings individually and together. |
| 24 | Mon 2026-11-23 | Play one synth sound over Bluetooth with both hand controls. |
| 25 | Fri 2026-11-27 | Demo Sounder in 5 minutes and explain what was hard. |
