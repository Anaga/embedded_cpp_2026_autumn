# Lesson 12 - answers

## 1. Why must the LED side not wait for the answer in a `while` loop?

On this board the pot side and the LED side are the *same* program running in
the *same* `loop()`. The pot side is the only thing that can produce the
answer, and it only runs when `loop()` reaches `potSide()`. If the LED side
blocks in `while (Serial2.available() == 0) { ... }`, the rest of `loop()`
never gets a turn, so `potSide()` is never called, so the answer is never
sent, so the loop never exits. The board hangs forever: a self-deadlock.
The fix is the state machine: send, return from `loop()`, and check for the
answer on a later pass.

## 2. How long does `POT 1023` + `\r\n` take on the wire at 115200 baud?

`POT 1023` is 8 characters. With `\r\n` that is 8 + 2 = 10 bytes.

UART frames are 8 data bits with 1 start bit and 1 stop bit (no parity), so
each byte takes 10 bits on the wire:

```
10 bytes * 10 bits/byte = 100 bits
100 bits / 115200 baud  = 0.000868 s = about 0.87 ms
```

So a full answer takes roughly **0.9 ms** to transmit. The 200 ms timeout is
thousands of times longer than one frame; it is meant to catch a dead wire
or a dead peer, not to time a single transmission.

## 3. What would change if the pot side sent its value every 100 ms on its own?

The request/answer protocol disappears. Concretely:

- **Pot side:** no more `LineReader` or `strcmp` for `POT?`. It just
  `Serial1.print("POT ")` + `analogRead()` + `'\n'` on a 100 ms timer.
- **LED side:** no IDLE/WAITING state machine and no 200 ms "did anyone
  answer" timeout. It only has to *consume* lines as they arrive and update
  the blink half-period from the newest one. The interesting failure mode
  changes from "no reply to my question" to "no new value arrived for a
  while", which would be detected by the age of the last received value
  rather than by a per-request timer.
- **Wiring:** the GP8 -> GP1 wire (the request direction) would no longer
  carry anything, so it could be removed.

The trade-off: the pot side burns UART bandwidth even when nobody is asking,
and the LED side can no longer ask at its own pace.
