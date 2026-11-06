# Lab 3 - A sleeping sensor node over ESP-NOW

Every 20 s the node wakes up, reads the AHT10, sends one packet to the
gateway over ESP-NOW, waits for the delivery status and sleeps again.
It never joins the WiFi network.

The gateway is the instructor's board (`lab3-gateway`). It is joined to
the class router and shows every node on a web page.

## Before the first build

Fill in `include/class_net.h` from the whiteboard:

```cpp
constexpr const char *CLASS_SSID = "the-class-router";
constexpr uint8_t GATEWAY_MAC[6] = {0x58, 0xCF, 0x79, 0x00, 0x00, 0x00};
```

`CLASS_SSID` is only used to find the router's WiFi channel. No password.

## Wiring

Session 18 wiring: AHT10 SDA GPIO 8, SCL GPIO 9, VIN 3V3, GND. Button A on
GPIO 0, RGB LED on 5 / 6 / 7. The LED works as in labs 1 and 2 (red blink =
woke up, blue = awake, green = asleep); three red blinks = FAIL, no ACK.

GPIO 8 and 9 are strapping pins. The AHT10 pull-ups keep them HIGH at
reset, which is the normal boot. GPIO 9 is also the BOOT button.

## Run

Expected output (your numbers differ):

```
Lesson 19 - Lab 3 - sleeping node esp-aab8
#1 cold
   AHT10 22.4 C 38.1 %
   scanning for "the-class-router"...
   ch 6  seq 1  -> SUCCESS
   awake 30 s - LED blue, upload now
   awake 2412 ms (+32300 waiting), sleeping 20 s
#2 timer
   AHT10 22.4 C 38.0 %
   ch 6  seq 2  -> SUCCESS
   awake 160 ms (+2300 waiting), sleeping 20 s
```

Then open the gateway page (address on the whiteboard) and find your
board. The **Awake ms** column is your previous wake.

## Things to try

- Compare a timer wake here with a timer wake in lab 2 that would have to
  join WiFi. Where does the time go now? (Hint: the AHT10 library waits
  for the conversion.)
- The instructor unplugs the gateway for a minute. What does your node
  print? After how many wakes does it scan again, and why?
- Put a wrong byte into `GATEWAY_MAC`. SUCCESS or FAIL? Who would have
  sent the ACK?
- Look at `include/msg.h`. What happens on the gateway if you add a field
  only on your side?

## Troubleshooting

- **"not found - check CLASS_SSID".** The name is case-sensitive. The
  router must be 2.4 GHz and in range.
- **Always FAIL.** Wrong `GATEWAY_MAC`, gateway off, or the gateway moved
  to another channel (the node rescans after 3 fails).
- **"AHT10 missing".** Check SDA / SCL. The node still sends, without a
  reading.
