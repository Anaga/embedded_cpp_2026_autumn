# Lesson 12 homework - one Pico, two UARTs

In class two Picos talked: the LED board asked for the potentiometer value,
the pot board answered, and the LED blinked at that speed.

At home you have one Pico. So it plays both roles at once. The RP2040 has two
hardware UARTs: the pot side talks through UART0, the LED side through UART1,
and two wires connect them on the same board.

## Wiring

**Potentiometer** - exactly as in class:

| Pot pin            | Pico                    |
|--------------------|-------------------------|
| middle pin (wiper) | GP26 / ADC0 (pin 31)    |
| outer pin          | AGND (pin 33)           |
| other outer pin    | 3V3 OUT (pin 36)        |

**UART loop** - two wires on the same board:

| From                       | To                         | Carries                         |
|----------------------------|----------------------------|---------------------------------|
| GP8 = UART1 TX (pin 11)    | GP1 = UART0 RX (pin 2)     | `POT?` from LED side to pot side |
| GP0 = UART0 TX (pin 1)     | GP9 = UART1 RX (pin 12)    | `POT n` from pot side to LED side |

No GND wire this time - both UARTs are on the same board.

In Arduino-Pico, `Serial1` is UART0 on GP0 / GP1 and `Serial2` is UART1 on
GP8 / GP9. Those are the default pins, so `Serial1.begin(...)` and
`Serial2.begin(...)` are all you need. `Serial` stays the USB connection to
your PC, for status messages only - the protocol never goes there.

## The protocol - the same as in class

| Who                         | Sends           | When                          |
|-----------------------------|-----------------|-------------------------------|
| LED side (master), UART1    | `POT?`          | every 100 ms                  |
| Pot side (slave), UART0     | `POT <0..1023>` | only as the answer to `POT?`  |

- Both UARTs at 115200 baud.
- Every message is one line: it ends with `\n`, a `\r` is ignored, at most
  31 characters.
- The slave never talks first. Anything other than `POT?` is ignored.
- The master maps the value to a blink half-period of 50 .. 500 ms.
- The master never sends a new `POT?` while it is still waiting for an
  answer.
- No answer within 200 ms: the master prints `no reply` on `Serial` and
  switches the LED off. When answers come back, blinking continues.

## The task

Write `<your-name>/src/main.cpp`, with any headers in the same folder.

1. **Pot side** on `Serial1`: read lines, answer `POT?` with `POT` and the
   current `analogRead()` value, ignore everything else.
2. **LED side** on `Serial2`: a state machine with the states IDLE and
   WAITING, exactly as on the slide "The master is a state machine".
3. **Blink** the on-board LED (GP25) at the half-period from the last answer.
   No `delay()` anywhere.
4. **Status** once per second on `Serial` (USB), for example:

   ```
   pot=512 half=275 ms
   no reply
   ```

5. **Two buffers.** Each UART needs its own line buffer - the global
   `s_line` from the slide is not enough any more. Write a small class
   `LineReader` that owns a buffer and a length and has
   `bool readLine(Stream& in)`, and create one object per UART.
6. **Nothing blocks.** Both sides run in the same `loop()`. The LED side must
   not wait for the answer in a loop - it checks, and comes back next time.
7. Course style: named constants at the top, `uint32_t`, `U` suffixes, ASCII
   only - also in comments and Serial output.

## Test it

1. Turn the potentiometer. The blink rate follows.
2. Pull out the GP0 -> GP9 wire. Within a third of a second: `no reply`, LED
   off. Put it back: blinking returns. (Why up to 300 ms and not 200?)
3. Pull out the GP8 -> GP1 wire instead. What happens? Why does it look the
   same from the LED side as test 2?

**Bonus:** switch the LED off only after 3 missed answers in a row. Which
rule is better - one miss or three - and what does each one cost?

## Answer in writing

Create `<your-name>/answers.md` with short answers:

1. Why must the LED side not wait for the answer in a `while` loop? What
   exactly happens on this board if it does?
2. How long does `POT 1023` plus `\r\n` take on the wire at 115200 baud?
   Show the calculation.
3. What would change in your code if the pot side sent its value every
   100 ms on its own, without being asked?

## Done when

- [ ] Turning the pot changes the blink rate
- [ ] Pulling a UART wire gives `no reply` and a dark LED; putting it back
      recovers without a reset
- [ ] `LineReader` is a class, used once per UART
- [ ] No `delay()` in the program
- [ ] `<your-name>/src/` and `<your-name>/answers.md` are committed and pushed
      (not `platformio.ini`)
