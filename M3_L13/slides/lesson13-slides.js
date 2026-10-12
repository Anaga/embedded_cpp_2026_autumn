/*
 * lesson13-slides.js
 * Lesson 13 - I2C and the AHT10 (Raspberry Pi Pico, Arduino-Pico core).
 *
 * Build:
 *     cp ../../tools/slides-theme.js .     (or keep a copy next to this file)
 *     npm install pptxgenjs react react-dom react-icons sharp
 *     node lesson13-slides.js              -> lesson13.pptx
 *
 * ASCII only. Every string goes through the theme's assertAscii().
 */

const T = require("./slides-theme");
const { FaThermometerHalf } = require("react-icons/fa");

const {
  NAVY, DARK_BG, CARD_BG, MID, ICE, WHITE, GRAY, GREEN, AMBER, RED,
  HEADING, BODY, CODE, MARGIN, CONTENT_W, BODY_TOP,
} = T;

const OUT = "lesson13.pptx";

// ---------------------------------------------------------------------------
// Local drawing helpers (diagrams the theme does not provide)
// ---------------------------------------------------------------------------

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "speaker notes"));
}

/** Filled box with centred text. */
function box(pres, slide, x, y, w, h, text, o) {
  const opt = o || {};
  slide.addShape(pres.shapes.RECTANGLE, {
    x: x, y: y, w: w, h: h,
    fill: { color: opt.fill || CARD_BG },
    line: { color: opt.line || MID, width: opt.lineW || 1 },
  });
  if (text !== undefined && text !== null && text !== "") {
    slide.addText(T.assertAscii(text, "box text"), {
      x: x, y: y, w: w, h: h, margin: 0,
      fontSize: opt.fontSize || 12, fontFace: opt.font || BODY,
      color: opt.color || WHITE, bold: !!opt.bold,
      align: "center", valign: "middle",
    });
  }
}

/** Plain text run. */
function text(slide, s, x, y, w, h, o) {
  const opt = o || {};
  slide.addText(T.assertAscii(s, "text"), {
    x: x, y: y, w: w, h: h, margin: 0,
    fontSize: opt.fontSize || 12, fontFace: opt.font || BODY,
    color: opt.color || ICE, bold: !!opt.bold, italic: !!opt.italic,
    align: opt.align || "left", valign: opt.valign || "middle",
  });
}

/** Straight line; w = 0 for vertical, h = 0 for horizontal. */
function line(pres, slide, x, y, w, h, color, width, arrow) {
  const ln = { color: color || ICE, width: width || 1.5 };
  if (arrow) { ln.endArrowType = "triangle"; }
  slide.addShape(pres.shapes.LINE, { x: x, y: y, w: w, h: h, line: ln });
}

/**
 * Digital waveform from an array of 0/1 levels, one entry per time step.
 * Draws horizontal segments and vertical edges.
 */
function waveform(pres, slide, levels, x0, yHigh, step, swing, color) {
  levels.forEach((lv, i) => {
    const y = lv ? yHigh : yHigh + swing;
    line(pres, slide, x0 + i * step, y, step, 0, color, 2);
    if (i > 0 && levels[i - 1] !== lv) {
      line(pres, slide, x0 + i * step, yHigh, 0, swing, color, 2);
    }
  });
}

// ---------------------------------------------------------------------------
// Deck
// ---------------------------------------------------------------------------

(async () => {
  const pres = T.newDeck(
    "Lesson 13 - I2C and the AHT10",
    "Lesson 13  |  I2C and the AHT10"
  );
  const iconTherm = await T.icon(FaThermometerHalf, "#CADCFC");
  let n = 0;
  let s;

  // 1 ------------------------------------------------------------------ title
  n++;
  s = T.titleSlide(pres, {
    tag: "LESSON 13",
    title: "I2C and the AHT10",
    subtitle: "Two wires, many devices, and a sensor that takes commands",
    course: "Embedded Software Development in C/C++",
    meta: "Raspberry Pi Pico  |  Arduino-Pico  |  Session 13 of 25",
    iconData: iconTherm,
  });
  notes(s,
    "Session 13 of 25, K4. 180 min with two 10-minute breaks.\n" +
    "Timeline: 0:00 homework check (20) - 0:20 pull-ups (25) - break - " +
    "0:55 protocol on paper (30) - 1:25 wiring and scanner (15) - break - " +
    "1:50 lab by hand (50) - 2:40 library (15) - 2:55 summary (5).\n" +
    "Hardware: one AHT10 module per pair, instructor keeps the third.");

  // 2 ------------------------------------------------------------------ goals
  n++;
  s = T.contentSlide(pres, "Today", n);
  T.addBullets(s, [
    "Why I2C lines are open-drain, and why the bus needs pull-ups",
    "Write out an I2C transaction byte by byte: start, address, ACK, data, stop",
    "Register-based vs command-based devices - the AHT10 is the second kind",
    "Find a device with an I2C scanner, and know what to check when it finds nothing",
    "Read the AHT10 by hand with Wire, waiting without delay()",
    "Use a library for the same job - and measure what it costs",
  ], MARGIN, BODY_TOP + 0.15, CONTENT_W, 3.7, 18);
  notes(s, "One minute. Point out that the last item comes back as homework.");

  // 3 -------------------------------------------------------- homework recap
  n++;
  s = T.contentSlide(pres, "Homework Recap: Why It Froze", n);
  T.addCode(s, [
    "Serial2.print(\"POT?\\n\");",
    "",
    "// wait for the answer",
    "while (!replyComplete()) {",
    "    // the pot side never runs:",
    "    // it lives in this same loop()",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 4.3, 2.9, 12, "Blocking wait: deadlock");
  T.addCode(s, [
    "switch (state) {",
    "case State::IDLE:",
    "    Serial2.print(\"POT?\\n\");",
    "    sentAt = millis();",
    "    state = State::WAITING;",
    "    break;",
    "case State::WAITING:",
    "    if (replyComplete()) { /* use it */ }",
    "    break;",
    "}",
  ].join("\n"), 5.2, BODY_TOP, 4.3, 2.9, 10, "State machine: loop() keeps turning");
  T.addCallout(pres, s, "info", "Keep this pattern in mind",
    "The AHT10 needs about 80 ms per measurement. Today we wait for it the same way.",
    MARGIN, 4.42, CONTENT_W, 0.62);
  notes(s,
    "0:00 - 20 min. Ask first: who got it running, who saw it freeze?\n" +
    "Let a student explain why: the answering side runs in the same loop(), " +
    "so while loop() is stuck in the while, nobody answers.\n" +
    "replyComplete() is shorthand for the line assembly they already wrote.\n" +
    "The fix is the same IDLE / WAITING machine as in the session 12 lab.");

  // 4 ---------------------------------------------------------------- section
  n++;
  s = T.sectionSlide(pres, {
    kicker: "PART 1", title: "Pull-ups",
    note: "Why a two-wire bus needs two resistors",
  });
  notes(s, "0:20 - start of the electrical block, 25 min including the measurement.");

  // 5 -------------------------------------------------- push-pull vs open-drain
  n++;
  s = T.contentSlide(pres, "Push-Pull vs Open-Drain", n);
  T.addCards(pres, s, [
    { title: "Push-pull", accent: RED,
      body: "Drives HIGH and drives LOW. This is what digitalWrite() does.\n\n" +
            "Two outputs on one wire, one HIGH and one LOW: a short circuit." },
    { title: "Open-drain", accent: GREEN,
      body: "Can only pull LOW, or let go.\n\n" +
            "Any number of devices can share one wire. Nobody can fight." },
    { title: "Let go = ?", accent: AMBER,
      body: "Nobody drives the line: it floats.\n\n" +
            "A pull-up resistor makes \"let go\" mean HIGH." },
  ], BODY_TOP + 0.15, 2.3);
  T.addCallout(pres, s, "info", "You have seen this before",
    "INPUT_PULLUP on the K3 buttons: the button pulls LOW, the internal pull-up makes released = HIGH.",
    MARGIN, 3.9, CONTENT_W, 0.85);
  notes(s,
    "Ask before showing card 3: if every device can only pull low or let go, " +
    "what is the line when everybody lets go? Answer: undefined, it floats.\n" +
    "Value trade-off if asked: smaller resistor = faster rising edges but more " +
    "current while low. 4.7k is the usual start at 100 kHz.");

  // 6 ------------------------------------------------- pull-up and measuring
  n++;
  s = T.contentSlide(pres, "One Resistor per Line - Check Your Module", n);
  {
    const railY = 1.45;
    const sdaY = 2.75;
    const resX = 2.55;
    text(s, "3V3", 0.5, railY - 0.3, 0.6, 0.3, { color: AMBER, bold: true });
    line(pres, s, 0.9, railY, 3.7, 0, AMBER, 2);
    line(pres, s, resX + 0.125, railY, 0, 0.2, ICE, 1.5);
    box(pres, s, resX, railY + 0.2, 0.25, 0.65, "", { fill: NAVY, line: ICE, lineW: 1.5 });
    text(s, "4.7k", resX + 0.35, railY + 0.3, 0.8, 0.4, { color: WHITE, bold: true, fontSize: 13 });
    line(pres, s, resX + 0.125, railY + 0.85, 0, sdaY - railY - 0.85, ICE, 1.5);
    line(pres, s, 0.9, sdaY, 3.7, 0, ICE, 2.5);
    text(s, "SDA", 0.4, sdaY - 0.35, 0.6, 0.3, { color: WHITE, bold: true });
    const devs = [{ x: 1.05, name: "Pico" }, { x: 3.15, name: "AHT10" }];
    devs.forEach((d) => {
      line(pres, s, d.x + 0.6, sdaY, 0, 0.5, ICE, 1.5);
      box(pres, s, d.x, sdaY + 0.5, 1.2, 0.8, d.name + "\npull LOW or let go",
        { fontSize: 11 });
    });
    text(s, "Same again for SCL", 0.9, 4.15, 3.7, 0.3, { color: GRAY, italic: true, align: "center" });
  }
  T.addBullets(s, [
    "Module unpowered, multimeter on ohms",
    "Measure SDA -> VIN and SCL -> VIN",
    "About 4.7k or 10k: pull-ups are on the module",
    "Open circuit: add 4.7k from each line to 3V3",
    "Small 3-pin part near the header? Maybe a regulator: pull-ups may go to its output",
  ], 5.0, BODY_TOP + 0.05, 4.5, 2.6, 13);
  T.addCallout(pres, s, "error", "3V3 OUT only - never VBUS",
    "Pull-ups to VIN and 5 V on VIN put 5 V on GP4 / GP5. RP2040 pins are not 5 V tolerant.",
    5.0, 3.85, 4.5, 1.1);
  notes(s,
    "Hands-on, pairs, about 10 min. Collect the measured values on the board, " +
    "one line per module.\n" +
    "Before class the instructor has measured all three (Open decision 7) - " +
    "use that to check the students' numbers, do not hand them the answer.\n" +
    "If a module has no pull-ups: hand out 4.7k resistors, two per module.");

  // 7 ---------------------------------------------------------------- section
  n++;
  s = T.sectionSlide(pres, {
    kicker: "PART 2", title: "The Protocol",
    note: "Start, address, ACK, data, stop",
  });
  notes(s, "0:55 - after the first break. 30 min including the paper exercise.");

  // 8 ------------------------------------------------- bus, start and stop
  n++;
  s = T.contentSlide(pres, "One Bus, Many Devices", n);
  {
    // bus sketch
    const sdaY = 1.45;
    const sclY = 1.75;
    line(pres, s, 1.6, sdaY, 7.8, 0, ICE, 2);
    line(pres, s, 1.6, sclY, 7.8, 0, ICE, 2);
    text(s, "SDA", 9.45, sdaY - 0.13, 0.5, 0.26, { color: WHITE, bold: true, fontSize: 11 });
    text(s, "SCL", 9.45, sclY - 0.13, 0.5, 0.26, { color: WHITE, bold: true, fontSize: 11 });
    box(pres, s, MARGIN, sdaY - 0.2, 1.1, 0.7, "Pico\nmaster", { fontSize: 11, line: ICE });
    const slaves = [
      { x: 2.6, t: "AHT10\n0x38" },
      { x: 4.8, t: "another device\nits own address" },
      { x: 7.0, t: "and more:\nthe same two wires" },
    ];
    slaves.forEach((d) => {
      line(pres, s, d.x + 0.4, sdaY, 0, 0.75, ICE, 1);
      line(pres, s, d.x + 0.8, sclY, 0, 0.45, ICE, 1);
      box(pres, s, d.x, 2.2, 1.6, 0.6, d.t, { fontSize: 11 });
    });

    // waveform: START, three bits, STOP
    const scl = [1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1];
    const sda = [1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1];
    const x0 = 1.4;
    const step = 0.55;
    const swing = 0.35;
    const yScl = 3.2;
    const ySda = 3.85;
    // highlight START (step 1) and STOP (step 12)
    s.addShape(pres.shapes.RECTANGLE, {
      x: x0 + 1 * step, y: yScl - 0.12, w: step, h: 1.2,
      fill: { color: NAVY, transparency: 100 }, line: { color: AMBER, width: 1.5, dashType: "dash" },
    });
    s.addShape(pres.shapes.RECTANGLE, {
      x: x0 + 12 * step, y: yScl - 0.12, w: step, h: 1.2,
      fill: { color: NAVY, transparency: 100 }, line: { color: AMBER, width: 1.5, dashType: "dash" },
    });
    waveform(pres, s, scl, x0, yScl, step, swing, ICE);
    waveform(pres, s, sda, x0, ySda, step, swing, GREEN);
    text(s, "SCL", MARGIN, yScl, 0.7, swing, { color: WHITE, bold: true, fontSize: 11 });
    text(s, "SDA", MARGIN, ySda, 0.7, swing, { color: WHITE, bold: true, fontSize: 11 });
    text(s, "START", x0 + 0.6 * step, 4.33, 1.2, 0.25, { color: AMBER, bold: true, fontSize: 10 });
    text(s, "STOP", x0 + 11.75 * step, 4.33, 1.2, 0.25, { color: AMBER, bold: true, fontSize: 10 });
    [4, 7, 9].forEach((i, k) => {
      text(s, "bit " + ["1", "0", "0"][k], x0 + i * step - 0.1, 2.88, step + 0.2, 0.25,
        { color: GRAY, fontSize: 10, align: "center" });
    });
    text(s,
      "Data changes only while SCL is LOW. SDA changing while SCL is HIGH means START or STOP.",
      MARGIN, 4.65, CONTENT_W, 0.35, { color: WHITE, fontSize: 13, align: "center" });
  }
  notes(s,
    "Two wires shared by everyone, each device has a 7-bit address. The master " +
    "(the Pico) drives the clock and starts every transaction.\n" +
    "Walk through the waveform left to right: idle both high; SDA falls while " +
    "SCL is high = START; then data bits, each one sampled while SCL is high; " +
    "SDA rises while SCL is high = STOP.\n" +
    "Ask: why can a START never be mistaken for data? Because data never " +
    "changes while SCL is high.");

  // 9 ---------------------------------------------------------- address byte
  n++;
  s = T.contentSlide(pres, "The First Byte: Address + R/W", n);
  {
    const bits = ["A6", "A5", "A4", "A3", "A2", "A1", "A0", "R/W"];
    const addr = [0, 1, 1, 1, 0, 0, 0];
    const bw = 0.62;
    const gap = 0.06;
    const x0 = 2.3;
    bits.forEach((b, i) => {
      text(s, b, x0 + i * (bw + gap), BODY_TOP + 0.15, bw, 0.3,
        { color: GRAY, fontSize: 11, align: "center" });
    });
    const rows = [
      { label: "Write", rw: 0, hex: "0x70", y: BODY_TOP + 0.5 },
      { label: "Read", rw: 1, hex: "0x71", y: BODY_TOP + 1.2 },
    ];
    rows.forEach((r) => {
      text(s, r.label, MARGIN, r.y, 1.6, 0.55, { color: WHITE, bold: true, fontSize: 15 });
      addr.concat([r.rw]).forEach((v, i) => {
        const isRw = i === 7;
        box(pres, s, x0 + i * (bw + gap), r.y, bw, 0.55, String(v), {
          font: CODE, fontSize: 16, bold: true,
          fill: isRw ? NAVY : CARD_BG, line: isRw ? AMBER : ICE,
          color: isRw ? AMBER : WHITE,
        });
      });
      text(s, "= " + r.hex, x0 + 8 * (bw + gap) + 0.15, r.y, 1.2, 0.55,
        { font: CODE, color: AMBER, bold: true, fontSize: 18 });
    });
    text(s, "address 0x38 = 0111000", x0, BODY_TOP + 1.85, 7 * (bw + gap), 0.3,
      { color: ICE, fontSize: 12, align: "center" });
  }
  T.addBullets(s, [
    "Wire takes the 7-bit address (0x38) and adds the R/W bit for you",
    "Some datasheets and forum posts write 0x70 / 0x71 - same device, 8-bit notation",
  ], MARGIN, 3.3, CONTENT_W, 0.9, 14);
  notes(s,
    "Do the shift on the board: 0x38 << 1 = 0x70, OR 1 for read = 0x71.\n" +
    "The 0x70 notation is a classic source of 'my sensor is not found': " +
    "someone passes 0x70 to Wire, which shifts it again.");

  // 10 ---------------------------------------------------------- ACK and NACK
  n++;
  s = T.contentSlide(pres, "The 9th Clock: ACK and NACK", n);
  T.addCards(pres, s, [
    { title: "ACK", accent: GREEN,
      body: "After every byte, the receiver pulls SDA LOW for one clock.\n\n\"Got it, go on.\"" },
    { title: "NACK on the address", accent: RED,
      body: "Nobody pulled SDA low: the pull-up wins, the line stays HIGH.\n\n" +
            "No device at that address - or a loose wire." },
    { title: "NACK at the end", accent: AMBER,
      body: "Reading: the master takes the last byte and does not ACK it.\n\n\"That is enough, stop sending.\"" },
  ], BODY_TOP + 0.15, 2.4);
  T.addCallout(pres, s, "info", "A missing device and a cut wire look the same",
    "Both give NACK. That is why the scanner is the first test, and why the analyzer comes in session 15.",
    MARGIN, 3.95, CONTENT_W, 0.8);
  notes(s,
    "Link back to the pull-ups: NACK is simply the absence of a pull. " +
    "Without a pull-up, even that answer is undefined.");

  // 11 ------------------------------------------ register vs command devices
  n++;
  s = T.contentSlide(pres, "Two Kinds of Devices", n);
  text(s, "Register-based (most sensors)", MARGIN, BODY_TOP + 0.05, 4.3, 0.35,
    { color: WHITE, bold: true, fontSize: 15, font: HEADING });
  T.addCode(s, [
    "S  addr+W A  reg A",
    "Sr addr+R A  data A  data N  P",
  ].join("\n"), MARGIN, BODY_TOP + 0.45, 4.3, 0.95, 12);
  T.addBullets(s, [
    "Write a register pointer, then read from it",
    "Sr = repeated start: turn around without releasing the bus",
  ], MARGIN, BODY_TOP + 1.65, 4.3, 1.2, 13);
  text(s, "Command-based (the AHT10)", 5.2, BODY_TOP + 0.05, 4.3, 0.35,
    { color: AMBER, bold: true, fontSize: 15, font: HEADING });
  T.addCode(s, [
    "S addr+W A  cmd A  arg A  arg A  P",
    "    ... wait ...",
    "S addr+R A  b0 A  ...  b5 N  P",
  ].join("\n"), 5.2, BODY_TOP + 0.45, 4.3, 1.25, 12);
  T.addBullets(s, [
    "Write a command, wait, read the raw result",
    "No register pointer, no repeated start",
  ], 5.2, BODY_TOP + 1.85, 4.3, 1.0, 13);
  T.addCallout(pres, s, "info", "Notation for today",
    "S start   Sr repeated start   P stop   A ack   N nack   addr+W / addr+R = the first byte",
    MARGIN, 4.2, CONTENT_W, 0.8);
  notes(s,
    "Register-based is theory today: the OPT4001 we planned for is gone, and " +
    "the AHT10 does not use registers. They will meet register devices in " +
    "final projects and in the analyzer session.\n" +
    "Ask: why is the repeated start needed? So no other master can grab the " +
    "bus between setting the pointer and reading.");

  // 12 ---------------------------------------------------------- exercise
  n++;
  s = T.contentSlide(pres, "On Paper: Write Out What Goes Over the Wire", n);
  T.addCards(pres, s, [
    { title: "1. Start a measurement", accent: AMBER,
      body: "Device 0x38.\nCommand bytes: 0xAC 0x33 0x00.\n\nWrite every byte, every A or N, start and stop." },
    { title: "2. Read the result", accent: AMBER,
      body: "Device 0x38.\nThe sensor sends 6 bytes.\n\nWho ACKs each byte? What happens after the last one?" },
  ], BODY_TOP + 0.15, 2.3);
  T.addCallout(pres, s, "warn", "Pairs, 8 minutes",
    "Use the notation from the previous slide. First byte in hex, not as addr+W.",
    MARGIN, 3.9, CONTENT_W, 0.85);
  notes(s,
    "ANSWER (show on the board after 8 min):\n" +
    "Write: S | 0x70 A | 0xAC A | 0x33 A | 0x00 A | P\n" +
    "Read:  S | 0x71 A | status A | b1 A | b2 A | b3 A | b4 A | b5 N | P\n" +
    "In the write, the sensor ACKs every byte. In the read, the master ACKs " +
    "bytes 0..4 and NACKs byte 5, then stops.\n" +
    "Common mistakes: 0x38 instead of 0x70 as the first byte; ACK after the " +
    "last read byte.");

  // 13 ---------------------------------------------------------- Wire API
  n++;
  s = T.contentSlide(pres, "The Wire API in Six Calls", n);
  T.addCode(s, [
    "Wire.begin();                    // GP4 SDA, GP5 SCL",
    "",
    "Wire.beginTransmission(0x38);    // start a write",
    "Wire.write(0xAC);                // queue bytes",
    "uint8_t err = Wire.endTransmission();",
    "                                 // send; 0 = all ACKed",
    "",
    "uint8_t got = Wire.requestFrom(0x38, 6);",
    "                                 // read 6 bytes",
    "uint8_t b0 = Wire.read();        // one byte at a time",
  ].join("\n"), MARGIN, BODY_TOP + 0.05, 5.5, 3.2, 11);
  T.addBullets(s, [
    "Nothing goes on the wire until endTransmission()",
    "err != 0 means a NACK or a bus error",
    "requestFrom() returns how many bytes arrived. Check that too",
    "Always the 7-bit address",
  ], 6.2, BODY_TOP + 0.05, 3.3, 3.2, 13);
  notes(s,
    "Map each call to the paper exercise: beginTransmission + write + " +
    "endTransmission = the whole write line; requestFrom = the whole read " +
    "line including the final NACK and stop.\n" +
    "In student code the address and command bytes become named constants at " +
    "the top of the file - the slide keeps literals for readability.");

  // 14 ------------------------------------------------ wiring and scanner
  n++;
  s = T.contentSlide(pres, "Wiring and the I2C Scanner", n);
  T.addTable(s, ["AHT10", "Pico"], [
    ["VIN", "3V3 OUT (pin 36)"],
    ["GND", "GND"],
    ["SDA", "GP4 (pin 6)"],
    ["SCL", "GP5 (pin 7)"],
  ], MARGIN, BODY_TOP + 0.1, 3.6, [1.2, 2.4], 13);
  T.addCode(s, [
    "for (uint8_t a = 1U; a < 127U; a++) {",
    "    Wire.beginTransmission(a);",
    "    if (Wire.endTransmission() == 0U) {",
    "        Serial.printf(\"found 0x%02X\\n\", a);",
    "    }",
    "}",
  ].join("\n"), 4.4, BODY_TOP + 0.1, 5.1, 2.05, 11, "Address only, no data: who answers with ACK?");
  T.addCallout(pres, s, "ok", "Expected: found 0x38",
    "Empty scan? Check, in this order: 3.3 V at VIN, shared GND, SDA / SCL swapped, pull-ups, pins in code.",
    MARGIN, 3.85, CONTENT_W, 1.0);
  notes(s,
    "1:25 - 15 min, pairs. The scanner is in the demo project; they run it as is.\n" +
    "Before showing the checklist in the callout, ask the class to build it " +
    "themselves: what could make the scan come back empty?");

  // 15 ------------------------------------------------ AHT10 commands
  n++;
  s = T.contentSlide(pres, "AHT10: A Sensor That Takes Commands", n);
  T.addTable(s, ["Command", "Bytes", "Then"], [
    ["Init / calibrate", "0xE1 0x08 0x00", "status bit 3 = calibrated"],
    ["Trigger measurement", "0xAC 0x33 0x00", "wait about 80 ms"],
    ["Soft reset", "0xBA", "wait 20 ms"],
  ], MARGIN, BODY_TOP + 0.05, CONTENT_W, [2.6, 2.7, 3.7], 13);
  {
    const states = [
      { t: "IDLE", c: ICE },
      { t: "MEASURING", c: AMBER },
      { t: "READ", c: GREEN },
    ];
    const bw = 1.9;
    const gap = 1.15;
    const y = 3.1;
    const x0 = MARGIN + 0.2;
    const labels = ["send 0xAC", "80 ms passed,\nbusy bit clear"];
    states.forEach((st, i) => {
      const x = x0 + i * (bw + gap);
      box(pres, s, x, y, bw, 0.6, st.t, { line: st.c, lineW: 2, bold: true, fontSize: 13 });
      if (i < states.length - 1) {
        line(pres, s, x + bw + 0.05, y + 0.3, gap - 0.1, 0, ICE, 1.5, true);
        text(s, labels[i], x + bw, y - 0.45, gap, 0.42, { color: GRAY, fontSize: 10, align: "center" });
      }
    });
    text(s, "then back to IDLE - one measurement per second, no delay()",
      x0, y + 0.66, 3 * bw + 2 * gap, 0.3, { color: GRAY, fontSize: 11, align: "center", italic: true });
  }
  T.addCallout(pres, s, "warn", "AHT10 is not AHT20",
    "Most examples online are AHT2x code: init 0xBE and a 7th CRC byte. Translate them, do not paste them.",
    MARGIN, 4.15, CONTENT_W, 0.85);
  notes(s,
    "Same trap family as Arduino-ESP32 core 3.x vs 2.x.\n" +
    "If a module never reports calibrated after 0xE1: soft reset 0xBA, try " +
    "again; if still not set, it may be an AHT20 sold as AHT10 - try 0xBE.\n" +
    "The state machine is the session 12 IDLE / WAITING pattern with one more state.");

  // 16 ------------------------------------------------ the 6-byte result
  n++;
  s = T.contentSlide(pres, "The 6-Byte Result", n);
  {
    const y = BODY_TOP + 0.55;
    const h = 0.6;
    const bw = 1.3;
    const gap = 0.1;
    const x0 = MARGIN + 0.1;
    const xOf = (i) => x0 + i * (bw + gap);
    // byte boxes; byte 3 is split into two nibbles
    [0, 1, 2, 4, 5].forEach((i) => {
      box(pres, s, xOf(i), y, bw, h, "byte " + i, {
        font: CODE, fontSize: 13, bold: true,
        line: i === 0 ? GRAY : (i < 3 ? ICE : AMBER),
      });
    });
    box(pres, s, xOf(3), y, bw / 2, h, "hi", { font: CODE, fontSize: 12, bold: true, line: ICE });
    box(pres, s, xOf(3) + bw / 2, y, bw / 2, h, "lo", { font: CODE, fontSize: 12, bold: true, line: AMBER });
    text(s, "byte 3", xOf(3), y - 0.32, bw, 0.28, { color: GRAY, fontSize: 11, align: "center" });
    text(s, "status", xOf(0), y + h + 0.1, bw, 0.3, { color: GRAY, fontSize: 12, align: "center" });
    // brackets
    const hx = xOf(1);
    const hw = xOf(3) + bw / 2 - hx;
    const tx = xOf(3) + bw / 2;
    const tw = xOf(5) + bw - tx;
    s.addShape(pres.shapes.RECTANGLE, { x: hx, y: y + h + 0.12, w: hw, h: 0.06, fill: { color: ICE } });
    s.addShape(pres.shapes.RECTANGLE, { x: tx, y: y + h + 0.12, w: tw, h: 0.06, fill: { color: AMBER } });
    text(s, "humidity: 20 bits", hx, y + h + 0.22, hw, 0.3, { color: ICE, bold: true, fontSize: 13, align: "center" });
    text(s, "temperature: 20 bits", tx, y + h + 0.22, tw, 0.3, { color: AMBER, bold: true, fontSize: 13, align: "center" });
  }
  T.addBullets(s, [
    "Status: bit 7 = busy, bit 3 = calibrated",
    "Raw values go into uint32_t - build them with shifts and masks",
  ], MARGIN, 2.85, 4.4, 1.2, 13);
  T.addCode(s, [
    "RH = raw / 2^20 * 100",
    "T  = raw / 2^20 * 200 - 50",
  ].join("\n"), 5.1, 2.85, 4.4, 1.0, 13, "2^20 = 1048576");
  T.addCallout(pres, s, "info", "Writing the shifts and masks is the lab",
    "Check your result: breathe on the sensor - humidity jumps within seconds.",
    MARGIN, 4.2, CONTENT_W, 0.8);
  notes(s,
    "Do not give the unpacking code: it is stage 3 of the lab.\n" +
    "For the instructor: hum = (b1 << 12) | (b2 << 4) | (b3 >> 4); " +
    "temp = ((b3 & 0x0F) << 16) | (b4 << 8) | b5. Cast to uint32_t before shifting.\n" +
    "Sanity values: room air 30..60 %RH, 20..25 C.");

  // 17 ---------------------------------------------------------- lab
  n++;
  s = T.contentSlide(pres, "Lab: The AHT10 by Hand", n);
  T.addCards(pres, s, [
    { title: "1. Init", accent: ICE,
      body: "Send the init command, read the status byte.\n\nPrint it in hex and as bits. Is bit 3 set?" },
    { title: "2. Measure", accent: AMBER,
      body: "Trigger, wait 80 ms in a state machine, read 6 bytes.\n\nPrint the raw bytes in hex, once per second." },
    { title: "3. Convert", accent: GREEN,
      body: "Unpack both 20-bit fields, apply the formulas.\n\nBreathe on it. Hold it between two fingers." },
  ], BODY_TOP + 0.15, 2.3);
  T.addCallout(pres, s, "warn", "Rules",
    "No delay() in loop(). Named constants for the address and the command bytes. Check every Wire return value.",
    MARGIN, 3.9, CONTENT_W, 0.8);
  notes(s,
    "1:50 - 50 min, pairs. Stage 1 about 10 min, stage 2 about 15, stage 3 about 20.\n" +
    "Stretch for fast pairs: integer hundredths instead of float (the RP2040 " +
    "has no FPU). Watch the overflow: raw * 20000 does not fit in 32 bits.\n" +
    "If time runs short, stage 3 finishes as part of the homework.");

  // 18 ---------------------------------------------------------- library
  n++;
  s = T.contentSlide(pres, "The Library: Same Numbers, Hidden Cost", n);
  T.addCode(s, [
    "#include <Adafruit_AHTX0.h>",
    "",
    "Adafruit_AHTX0 aht;      // aht.begin() in setup()",
    "",
    "sensors_event_t hum, temp;",
    "uint32_t t0 = micros();",
    "aht.getEvent(&hum, &temp);",
    "uint32_t dt = micros() - t0;",
  ].join("\n"), MARGIN, BODY_TOP + 0.05, 5.0, 2.6, 11,
  "platformio.ini: lib_deps = adafruit/Adafruit AHTX0");
  T.addBullets(s, [
    "Uses 0xE1, so it works with the AHT10",
    "Inside getEvent(): while busy -> delay(10). No timeout",
    "Measure dt. How long is loop() frozen?",
  ], 5.75, BODY_TOP + 0.05, 3.75, 2.6, 13);
  T.addCallout(pres, s, "warn", "Fine for a logger. Not next to a blink, a UART protocol or a display.",
    "Homework: wrap your by-hand code in a class that never blocks.",
    MARGIN, 4.2, CONTENT_W, 0.8);
  notes(s,
    "2:40 - 15 min. Open the library source together (Adafruit_AHTX0.cpp) and " +
    "find the busy loop - reading other people's drivers is part of the job.\n" +
    "Expected dt: about 80 ms. Same micros() method as session 11.\n" +
    "Compare the numbers with the by-hand version: they should agree within " +
    "rounding.");

  // 19 ---------------------------------------------------------- summary
  n++;
  s = T.summarySlide(pres, n, [
    "Open-drain lines + pull-ups: anyone may pull low, nobody can fight",
    "First byte = 7-bit address + R/W; ACK is a pull, NACK is its absence",
    "Register devices use a pointer; the AHT10 takes commands",
    "Scanner first, then the driver; wait in a state machine, not in delay()",
    "Libraries are convenient - measure what they hide",
  ],
  "Class Aht10 with a non-blocking start() / poll(). A reading every 2 s. " +
  "Sensor unplugged -> \"sensor missing\" once, and it recovers by itself. Details in homework/README.md.");
  notes(s,
    "2:55 - 5 min. Modules go home with the students who do the homework.\n" +
    "Next session: SPI and the Nokia 5110; the AHT10 reading goes on the display.");

  await T.save(pres, OUT);
  console.log("OK: " + OUT + " (" + n + " slides)");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
