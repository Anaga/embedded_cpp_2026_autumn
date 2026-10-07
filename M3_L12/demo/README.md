# Lesson 12 demo - UART bridge between two Picos

Two Raspberry Pi Picos talk over UART. What you type in your serial monitor
goes out on UART0 to your partner's Pico and appears in your partner's
monitor - and the other way round.

This is the code from the slide "The bridge: what you type goes to your
partner", ready to build. It is stage 1 of the lab. Stage 2, the command
protocol, is yours to write.

## Wiring

Work in pairs: Pico A and Pico B, three wires.

| Pico A          | Pico B          | Note                         |
|-----------------|-----------------|------------------------------|
| GP0 TX (pin 1)  | GP1 RX (pin 2)  | A talks, B listens           |
| GP1 RX (pin 2)  | GP0 TX (pin 1)  | B talks, A listens           |
| GND (pin 3)     | GND (pin 3)     | shared reference             |

TX always goes to RX - the two data wires cross.

**Never connect 3V3 or VBUS between the boards.** Each Pico is powered by its
own USB cable; only GND is shared. The Pico uses 3.3 V logic: do not connect
it to 5 V signals or to an RS-232 port.

## Build and run

1. Open the `demo` folder in VS Code.
2. Upload to both boards (one laptop each).
3. Open the serial monitor on both laptops.
4. Type a line and press Enter. It appears on your partner's screen.

The monitor settings in `platformio.ini` matter here:

| Setting                          | What it does                                     |
|----------------------------------|--------------------------------------------------|
| `monitor_echo = yes`             | you see what you type                            |
| `monitor_filters = send_on_enter`| the whole line is sent when you press Enter      |

Without `send_on_enter` the monitor sends every key the moment you press it.
Lines end with CR LF (`\r\n`) - remember that when you write the protocol.

## Nothing arrives?

Check in this order:

1. TX and RX crossed? (TX of one board to RX of the other)
2. GND connected between the boards?
3. Same `UART_BAUD` on both boards?
4. Monitor open on the right COM port?

## Try this

Pull out one of the two data wires. Which direction stops working, and why
only that one?
