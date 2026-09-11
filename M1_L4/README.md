# Lesson 04 - Soldering and Documentation

Block K2, session 4 of 25. Showcase module M1. This session closes block K2.

Two halves. First you solder, with your hands, on a real board. Then you go
looking for the numbers behind the rules we have been using since lesson 03.

---

## Goals

By the end of this session you can:

- solder header pins to a board and produce joints that hold
- tell a good joint from a cold joint and from a bridge
- check your own soldering with a multimeter before power is applied
- say which of datasheet, reference manual and application note answers which
  kind of question
- find one specific number in an 800-page datasheet
- use an AI assistant as a search aid without trusting the numbers it produces

---

## What you need

- A spare ESP32-C3 Super Mini and a set of header pins (provided)
- Multimeter
- Your lesson 03 homework answers
- A laptop with a PDF reader for the datasheet part

Soldering stations, solder and tip cleaner are shared. Three stations for the
group, so everyone works at the same time.

---

## Session plan

| Time | Block |
|---|---|
| 15 min | Homework review - Ohm's law |
| 15 min | Soldering: technique and safety |
| 60 min | Soldering: hands on, plus continuity check |
| 25 min | Datasheet, reference manual, application note |
| 35 min | Finding one answer in 800 pages |
| 30 min | AI as a documentation aid, and where it misleads |

---

## 1. Homework review

| Problem | Answer |
|---|---|
| 1 - resistor for red LED | 130 ohm |
| 2 - resistor for blue LED | 20 ohm |
| 3 - power in the resistor | 0.013 W (13 mW) |
| 4 - resistance from measurement | 1031 ohm |
| 5 - minimum pull-up value | 6600 ohm (6.6 kohm) |

Keep problem 5 in mind. In the second half we look up what pull-up resistor the
ESP32-C3 actually has inside it, and compare.

---

## 2. Soldering

### Materials

| Property | Value |
|---|---|
| Solder alloy | Sn99 Cu0.7 Ag0.3, lead free |
| Wire diameter | 0.7 mm |
| Flux | Rosin core, 3 percent, inside the wire |
| Melting point | about 227 C |
| Iron temperature | 350 C, up to 370 C if flow is poor |

The flux is already inside the solder wire. For header pins on a clean board
you do not need to add any.

### Lead-free is not the solder in most tutorials

Lead-free melts at about 227 C. The older tin-lead alloy melted at 183 C. Three
consequences:

- The iron runs hotter. 350 C, not 300 C.
- The solder flows less eagerly and gives you a narrower working window.
- **A finished joint looks matte and slightly grainy, not mirror shiny.** Most
  soldering photos online show tin-lead solder. Do not re-melt a perfectly good
  lead-free joint because it looks dull.

### Making one joint

1. Wipe the tip and tin it. A dry, oxidised tip transfers almost no heat, and
   this is the single most common reason soldering "does not work".
2. Touch the tip to the pin and the pad at the same time.
3. Wait about one second, then feed solder into the joint, not onto the tip.
4. Let it flow around the pin. Remove the solder first, then the iron.
5. Total contact time 2 to 3 seconds per pin.

If the solder is not flowing, do not hold the iron there longer. Raise the
temperature toward 370 C instead. A cool iron held on a pad for ten seconds is
how pads come off the board.

### Three outcomes

| Outcome | What it looks like | Why it matters |
|---|---|---|
| Good | Solder has wetted both pin and pad, smooth cone, matte | Holds |
| Cold | Blob sitting on the pad without wetting it, lumpy or cracked | Works today, fails in a month |
| Bridge | Solder joining two neighbouring pins | Short circuit, easy to find |

A cold joint is the dangerous one. It passes the first test and then fails
later, somewhere else, for no visible reason.

### Safety

- The tip is at 350 C. It burns on contact and stays hot long after switch off.
- The iron lives in its holder. Never on the bench, never over an edge.
- The smoke is flux, not solder. Do not lean over the joint while it melts.
- Wash your hands before eating, even with lead-free solder.
- If you drop the iron, let it fall. Do not catch it.
- Burn: cold running water, then tell the instructor.

### The task

One spare ESP32-C3 Super Mini and one header strip per person.

1. Seat the header in the holes, board flat on the bench.
2. Solder **one corner pin only**.
3. Check the header sits straight and flush. If it is crooked, reheat that one
   pin and adjust.
4. Only then solder the rest, pin by pin.

Soldering the whole row before checking alignment means desoldering the whole
row. Speed is not the goal today.

### Checking your own work

Multimeter in continuity mode, the same tool from lesson 03.

- Between each pin and its pad: it must beep. No beep means a cold joint.
- Between neighbouring pins: it must stay silent. A beep means a bridge.
- Then look at every joint against the three outcomes above.

This is the same rule as Monday: measure instead of assuming. The difference is
that now the thing being measured is your own work, and the fault is found
before the board is ever powered.

---

## 3. Reading documentation

### Three documents, three jobs

| Document | Answers | Go here for |
|---|---|---|
| Datasheet | What the chip **is** | A number: pin, voltage, current, limit |
| Reference manual | How the chip **works** | Registers, peripherals, bit fields |
| Application note | How to **use** it for one task | Worked examples, recommended circuits |

For the ESP32-C3 the datasheet is about 75 pages and the Technical Reference
Manual is over 1000. Picking the right document first saves most of the search.

### Finding one answer in 800 pages

- Never read forward from page one. You are looking something up, not reading.
- Start with the table of contents, then the list of tables. Numbers live in
  tables.
- Search for the unit, not the concept: `mA`, `kohm`, `V` - not "how much
  current".
- Learn the standard symbols. They are the same across vendors:

| Symbol | Meaning |
|---|---|
| VIH / VIL | Minimum HIGH / maximum LOW input voltage |
| VOH / VOL | HIGH / LOW output voltage |
| IOH / IOL | HIGH-level source / LOW-level sink current |
| RPU / RPD | Internal weak pull-up / pull-down resistance |

- Note the document version. Values and page numbers move between revisions.
  We use **ESP32-C3 Series Datasheet v2.4**.

### Worked example, done together

Question from lesson 03: we said a GPIO pin drives "tens of milliamps". Where
is the real number?

1. Contents -> Section 5 Electrical Characteristics -> 5.4 DC Characteristics
2. Table 5-4 lists IOH, high-level source current: **40 mA typical**
3. But Table 2-1, note 4, gives the **default** drive strength: **20 mA** for
   GPIO 8

Two numbers, both correct. One is the ceiling with the drive strength turned up
(the 40 mA figure is quoted at PAD_DRIVER = 3), the other is what you get
without configuring anything. Reading only the first line of a table row is how
you end up designing against the wrong one.

### Monday's rules, now with sources

| We said | Datasheet says | Where |
|---|---|---|
| Logic is 3.3 V | VIH min = 0.75 x VDD = 2.48 V | Table 5-4 |
| LOW is near 0 V | VIL max = 0.25 x VDD = 0.83 V | Table 5-4 |
| Tens of milliamps | 20 mA default, 40 mA max | Table 2-1, Table 5-4 |
| GPIO 9 has a pull-up | RPU = 45 kohm | Table 5-4 |
| GPIO 8 and 9 are strapping pins | Boot mode control | Table 3-3 |

Note the last line of your homework. You calculated that a pull-up must be at
least 6.6 kohm to keep the wasted current under 0.5 mA. The chip uses 45 kohm -
comfortably above your floor, and chosen to waste far less.

---

## 4. AI as a documentation tool

### Where it helps

- Explaining an unfamiliar term
- Telling you which document and which section to open
- Summarising a concept you have no vocabulary for yet

### Where it misleads

- Exact numbers
- Pin assignments
- Chip variants. ESP32 and ESP32-C3 are different chips with different cores
  and different pinouts, and assistants mix them up constantly

The failure mode is confidence. A wrong answer arrives in exactly the same tone
as a right one, with no marker to tell you which you got.

### The working rule

```
AI        -> where to look
Datasheet -> what the answer is
```

- An answer without a section or table number is not an answer yet.
- If the assistant names a table, open that table and read it yourself.
- If it cannot name one, treat the number as a guess.

This is the same standard you would apply to a colleague answering from memory.
It is not a statement about AI being bad. It is about which step of the job you
are delegating.

---

## Homework

See `homework/README.md`. Six lookups in the ESP32-C3 datasheet. Each answer is
a number plus the table it came from.
