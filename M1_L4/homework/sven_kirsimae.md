# Lesson 04 - Homework Answers

Name: Sven Kirsimäe

GitHub username: `svennu`

## Reference document

- Required source: ESP32-C3 Series Datasheet, version 2.4
- Official lookup page: https://www.espressif.com/en/support/documents/technical-documents
- Working with document: https://documentation.espressif.com/esp32-c3_datasheet_en.pdf

## Q1 - Internal weak pull-up resistance

**Question:** What is the internal weak pull-up resistance (RPU) of a GPIO pin?

Q1: `45 kohm` (`Table 5-4. DC Characteristics (3.3 V, 25 °C)`, parameter `RPU`, column `Typ`, page `55`)

## Q2 - Minimum high-level input voltage

**Question:** What is the minimum high-level input voltage (VIH)? Give the
formula as printed and the value for VDD = 3.3 V.

Q2: `0.75 x VDD = 0.75 x 3.3 V = 2.475 V` (`Table 5-4. DC Characteristics (3.3 V, 25 °C), parameter VIH, min column`, page `55`)

## Q3 - Maximum low-level input voltage

**Question:** What is the maximum low-level input voltage (VIL)? Again, formula
and the value for VDD = 3.3 V.

Formula: `<formula as printed>`

Value for VDD = 3.3 V: `<value>`

Source: Table `<table number>`, page `<page number>`

## Q4 - Default drive strength of GPIO 8

**Question:** What is the default drive strength of GPIO 8?

Careful here. This one is not in the Electrical Characteristics section, and
several pins have a different value. Read the note, not just the table body.

Q4: `<value>`   (Table `<table number>`, page `<page number>`)

## Q5 - Maximum cumulative IO output current

**Question:** What is the maximum cumulative IO output current for the whole
chip - the absolute maximum rating, across all pins together?

Q5: `<value>`   (Table `<table number>`, page `<page number>`)

## Q6 - Boot mode when GPIO 9 is low at reset

**Question:** In the strapping pin table, which boot mode does the chip enter
when GPIO 9 is low at reset?

This one is a name, not a number. Give the mode name and the table.

Q6: `<mode name>`   (Table `<table number>`, page `<page number>`)

## Comparison with lesson 03, problem 5

Compare your Q1 answer with what you calculated in lesson 03, problem 5: the
minimum pull-up value that keeps the wasted current under 0.5 mA.

Is the real internal pull-up above or below your floor? By how much? One short
sentence on why a chip designer would pick the value they did.

`<Compare the internal pull-up with the calculated minimum value of 8.2 kohm
and briefly explain the design choice.>`
