# Lesson 04 - Homework

Datasheet lookup. Six numbers, found by you, in the real document.

## The document

**ESP32-C3 Series Datasheet, version 2.4.**

Download it from Espressif rather than from a random mirror, and check the
version on the title page. Values and page numbers move between revisions, so
an answer from a different version may not match.

The official page is:

```
https://www.espressif.com/en/support/documents/technical-documents
```

## What to submit

For each question, give **two things**:

1. The value, with units
2. Where you found it - table number, and the page in your copy

An answer without a source is not an answer. That is the whole point of this
assignment: not the number, but being able to show where it came from.

Write your answers in this format:

```
Q1: <value>   (Table X-Y, page NN)
```

## Rules

- Find every number in the datasheet yourself.
- You may use an AI assistant to work out *which section* to open. You may not
  use it for the value. If it gives you a number, go and check that number in
  the document before writing it down - and if it named a table that does not
  exist, that is worth telling us about on Monday.
- Some answers are given as a formula involving VDD rather than a plain number.
  Where that happens, give both: the formula as printed, and the result for
  VDD = 3.3 V.

---

## Questions

**Q1.** What is the internal weak pull-up resistance (RPU) of a GPIO pin?

**Q2.** What is the minimum high-level input voltage (VIH)? Give the formula as
printed and the value for VDD = 3.3 V.

**Q3.** What is the maximum low-level input voltage (VIL)? Again, formula and
the value for VDD = 3.3 V.

**Q4.** What is the default drive strength of GPIO 8?

Careful here. This one is not in the Electrical Characteristics section, and
several pins have a different value. Read the note, not just the table body.

**Q5.** What is the maximum cumulative IO output current for the whole chip -
the absolute maximum rating, across all pins together?

**Q6.** In the strapping pin table, which boot mode does the chip enter when
GPIO 9 is low at reset?

This one is a name, not a number. Give the mode name and the table.

---

## One thing to think about

Compare your Q1 answer with what you calculated in lesson 03, problem 5: the
minimum pull-up value that keeps the wasted current under 0.5 mA.

Is the real internal pull-up above or below your floor? By how much? One short
sentence on why a chip designer would pick the value they did.

---

## Submitting

Commit to your own folder in the cohort repository:

```
students/<your-github-username>/lesson-04/homework.md
```

Commit message in English, as always.
