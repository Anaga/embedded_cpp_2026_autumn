**Note** I do homework myself without AI, because otherwise what's the point. However if I'm stuck I ask AI give me hints. Finally I ask AI to check the answers, then to commit and git push. That's why you see Claude Cowork in GitHub.

**Note2** The given link (https://www.espressif.com/en/support/documents/technical-documents) didn't work. Googling worked.

---

## Q1. What is the internal weak pull-up resistance (RPU) of a GPIO pin?

Chapter that seems most appropriate is 5. Electrical Characteristics
Ctrl+F for "pull-up" brings to 5.4 DC Characteristics (page 55)

**Answer**: 45kohms

## Q2. What is the minimum high-level input voltage (VIH)? Give the formula as
printed and the value for VDD = 3.3 V.

Same chapter, same table as above, second row.
In column "Min" it says "0.75xVDD" - so 0.75 multiplied by 3.3?

**Answer**: 0.75x3.3V

## Q3. What is the maximum low-level input voltage (VIL)? Again, formula and
the value for VDD = 3.3 V.

Same table, third row, column "Max"

**Answer**: 0.25x3.3V

## Q4. What is the default drive strength of GPIO 8?

Careful here. This one is not in the Electrical Characteristics section, and
several pins have a different value. Read the note, not just the table body.

What is "drive strength"?
Ctrl+F finds only 1 match on page 17.
It says default drive strength for "all other pins": 20mA
Pin 8 has to be "other".

**Answer**: 20mA

## Q5. What is the maximum cumulative IO output current for the whole chip -
the absolute maximum rating, across all pins together?

Back to electrical characteristics chapter.
Table of contents says 5.6 Current Consumption
Nope that's RF, not the same thing.
Ctrl+F "IO Output" gives 1 match - Chapter 5.1 Absolute Maximum Ratings
Max column says 1000mA
Note explains that at this power it's still functional with all IO pins high

**Answer**: 1000mA

## Q6. In the strapping pin table, which boot mode does the chip enter when
GPIO 9 is low at reset?

This one is a name, not a number. Give the mode name and the table.

Table "Boot mode control" on page 31. GPIO 9 is high by default, but when it is low, the boot mode is Joint Download Boot mode.

**Answer**: Joint Download Boot Mode


## One thing to think about

Compare your Q1 answer with what you calculated in lesson 03, problem 5: the
minimum pull-up value that keeps the wasted current under 0.5 mA.

Is the real internal pull-up above or below your floor? By how much? One short sentence on why a chip designer would pick the value they did.

**Sentence**: The real value (45kohms) is well above the floor (6.6kohms), which would reduce the current; I think it's because the chip is so tiny and there isn't that much current there to spare.