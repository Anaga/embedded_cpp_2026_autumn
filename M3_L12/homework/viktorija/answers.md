# Lesson 12 homework - answers

## Questions

### 1. Why must the LED side not wait for the answer in a `while` loop? What
   exactly happens on this board if it does?

LED side has to give way and let the POT side happen. A while loop would keep the program stuck in that loop and the POT side would not have the opportunity to send the answer.

### 2. How long does `POT 1023` plus `\r\n` take on the wire at 115200 baud?
   Show the calculation.

Sorry I let the AI do this one.

- Characters: `P O T space 1 0 2 3 \r \n` = 10 bytes
- 8N1: 1 start bit + 8 data bits + 1 stop bit = 10 bits per byte
- 10 bytes x 10 bits = 100 bits
- 1 bit = 1 / 115200 s = 8.68 us
- 100 bits x 8.68 us = 868 us, about 0.87 ms

So the answer takes less than 1 ms. The 200 ms timeout is very generous.

### 3. What would change in your code if the pot side sent its value every
   100 ms on its own, without being asked?

I think it would also work fine.
The program might even be simpler.
And one wire is no longer necessary because it's only one-way communication.

### Pull out the GP0 -> GP9 wire. Within a third of a second: `no reply`, LED
   off. Put it back: blinking returns. (Why up to 300 ms and not 200?)

Because it's 100ms + 200ms

### Pull out the GP8 -> GP1 wire instead. What happens? Why does it look the
   same from the LED side as test 2?

In one case the answer is lost, in the other case both question and answer is lost, so not OK both times.
