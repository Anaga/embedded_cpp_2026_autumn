/*
 * lesson14-slides.js - Lesson 14: SPI and the Nokia 5110
 *
 * Build:  node lesson14-slides.js   ->  lesson14.pptx
 * Needs slides-theme.js next to this file and
 *   npm install pptxgenjs react react-dom react-icons sharp
 *
 * ASCII only. assertAscii() in the theme throws on anything else.
 */

const T = require("./slides-theme");
const { FaMicrochip } = require("react-icons/fa");

const OUT = "lesson14.pptx";
const {
  MARGIN, CONTENT_W, BODY_TOP, NAVY, CARD_BG, MID, ICE, WHITE, GRAY,
  GREEN, AMBER, RED, CODE, BODY, HEADING,
} = T;

let n = 0;
function next() { n += 1; return n; }

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "speaker notes"));
}

function label(slide, text, x, y, w, h, opts) {
  const o = Object.assign({
    x: x, y: y, w: w, h: h, fontSize: 12, fontFace: BODY, color: ICE, margin: 0,
  }, opts || {});
  slide.addText(T.assertAscii(text, "label"), o);
}

function line(pres, slide, x, y, w, h, color, width, dash) {
  const opts = { x: x, y: y, w: w, h: h, line: { color: color, width: width || 1.5 } };
  if (dash) { opts.line.dashType = "dash"; }
  slide.addShape(pres.shapes.LINE, opts);
}

function box(pres, slide, x, y, w, h, fill, border, borderW) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x: x, y: y, w: w, h: h,
    fill: { color: fill }, line: { color: border || fill, width: borderW || 1 },
  });
}

function dot(pres, slide, x, y, color) {
  const r = 0.05;
  slide.addShape(pres.shapes.OVAL, {
    x: x - r, y: y - r, w: 2 * r, h: 2 * r, fill: { color: color }, line: { color: color },
  });
}

(async () => {
  const pres = T.newDeck(
    "Lesson 14 - SPI and the Nokia 5110",
    "Lesson 14  |  SPI and the Nokia 5110"
  );
  const chip = await T.icon(FaMicrochip, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  let s = T.titleSlide(pres, {
    tag: "LESSON 14",
    title: "SPI and the Nokia 5110",
    subtitle: "Four wires, no ACK, and a display you drive byte by byte",
    course: "Embedded Software Development in C/C++",
    meta: "Raspberry Pi Pico (RP2040)  |  PCD8544  |  RAII",
    iconData: chip,
  });
  next();
  notes(s,
    "Session 14 of 25, K4. Last bus of the three: UART (12), I2C (13), SPI today.\n" +
    "Everyone needs: Pico, AHT10 from last time, Nokia 5110, breadboard, wires.");

  // -------------------------------------------------------------------------
  // 2. Goals
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Today", next());
  T.addBullets(s, [
    "Say how SPI differs from I2C: push-pull, chip select, full duplex, no ACK",
    "Read a clock mode off a timing diagram: the PCD8544 is mode 0",
    "Understand the display memory: 6 banks of 84 bytes, one byte = 8 pixels",
    "Bring the display up from reset with commands and data",
    "Wrap chip select in a RAII class: the K3 idea on real hardware",
    "Show the AHT10 reading on your own display, without delay()",
  ], MARGIN, BODY_TOP, CONTENT_W, 3.0);
  T.addCallout(pres, s, "info", "Every lab stage is individual today",
    "Everyone has their own Pico, AHT10 and Nokia 5110. Work next to a partner, help each other, build your own.",
    MARGIN, 4.2, CONTENT_W, 0.8);
  notes(s, "1 min. Point out the last line: no sharing hardware today.");

  // -------------------------------------------------------------------------
  // 3. Homework check
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Homework Check: the Aht10 Class", next());
  T.addBullets(s, [
    "Show it running: a new reading every 2 s on Serial",
    "Unplug the sensor: \"sensor missing\" once, then quiet",
    "Plug it back in: readings come back by themselves",
    "Where does loop() wait for the 75 ms conversion?",
  ], MARGIN, BODY_TOP, CONTENT_W, 2.4);
  T.addCallout(pres, s, "ok", "Keep the class open",
    "Your Aht10 class goes onto the display in lab 3. Today's driver gets the same non-blocking treatment.",
    MARGIN, 3.9, CONTENT_W, 0.85);
  notes(s,
    "0:00 - 15 min. Answer to the question: nowhere. loop() starts the measurement, " +
    "returns, and checks millis() on later passes - the IDLE / WAITING pattern from session 12.\n" +
    "If someone blocks for 75 ms: it works, but nothing else in loop() runs meanwhile.");

  // -------------------------------------------------------------------------
  // 4. Section: SPI
  // -------------------------------------------------------------------------
  s = T.sectionSlide(pres, { kicker: "PART 1", title: "SPI on Paper", note: "Four wires and a clock that the master owns" });
  next();
  notes(s, "0:15 - 20 min for part 1.");

  // -------------------------------------------------------------------------
  // 5. SPI vs I2C
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "SPI Next to I2C", next());
  T.addTable(s,
    ["", "I2C (session 13)", "SPI (today)"],
    [
      ["Wires", "SDA, SCL", "SCK, MOSI, MISO, CS"],
      ["Outputs", "open-drain + pull-ups", "push-pull, no pull-ups"],
      ["Who listens", "7-bit address in the first byte", "the device whose CS is low"],
      ["Speed", "100 / 400 kHz", "MHz (PCD8544: up to 4 MHz)"],
      ["Direction", "one way at a time", "both ways on every clock"],
      ["Acknowledge", "ACK / NACK after every byte", "none"],
      ["Another device", "same two wires", "one more CS wire"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [2.0, 3.5, 3.5], 13);
  label(s, "Datasheets rename the wires: SCLK, SDI / SDO, SS, CE, SCE. Same signals.",
    MARGIN, 4.2, CONTENT_W, 0.35, { fontSize: 12, color: GRAY, italic: true });
  notes(s,
    "Contrast every row with last session. Push-pull is why SPI is fast: nobody waits " +
    "for a pull-up resistor to drag the line high.\n" +
    "The last row is the price: no addresses, so every device costs a GPIO.");

  // -------------------------------------------------------------------------
  // 6. Four wires diagram
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Four Wires, One CS per Device", next());
  {
    const mx = 0.6, mw = 1.8, my = 1.3, mh = 3.3;
    box(pres, s, mx, my, mw, mh, CARD_BG, ICE, 1.5);
    label(s, "Pico\n(master)", mx, my + 1.2, mw, 0.8, { fontSize: 15, bold: true, color: WHITE, align: "center", fontFace: HEADING });

    const sx = 6.8, sw = 2.6;
    box(pres, s, sx, 1.3, sw, 1.5, CARD_BG, GREEN, 1.5);
    label(s, "Nokia 5110", sx, 1.75, sw, 0.4, { fontSize: 15, bold: true, color: WHITE, align: "center", fontFace: HEADING });
    label(s, "write-only", sx, 2.15, sw, 0.3, { fontSize: 11, color: GRAY, align: "center" });
    box(pres, s, sx, 3.1, sw, 1.5, CARD_BG, MID, 1);
    label(s, "Another SPI device", sx, 3.55, sw, 0.4, { fontSize: 14, bold: true, color: WHITE, align: "center", fontFace: HEADING });
    label(s, "(SD card, flash, ...)", sx, 3.95, sw, 0.3, { fontSize: 11, color: GRAY, align: "center" });

    const x0 = mx + mw;
    const sig = [
      { name: "SCK ->", y1: 1.6, y2: 3.4, bx: 4.6 },
      { name: "MOSI ->", y1: 1.9, y2: 3.7, bx: 5.0 },
      { name: "<- MISO", y1: 2.2, y2: 4.0, bx: 5.4 },
    ];
    sig.forEach((g, i) => {
      const toFirst = i === 2 ? null : sx;
      line(pres, s, x0, g.y1, g.bx - x0, 0, ICE, 1.5);
      if (toFirst) {
        line(pres, s, g.bx, g.y1, sx - g.bx, 0, ICE, 1.5);
      } else {
        line(pres, s, g.bx, g.y1, sx - g.bx, 0, RED, 1.25, true);
      }
      line(pres, s, g.bx, g.y1, 0, g.y2 - g.y1, ICE, 1.5);
      line(pres, s, g.bx, g.y2, sx - g.bx, 0, ICE, 1.5);
      dot(pres, s, g.bx, g.y1, ICE);
      label(s, g.name, x0 + 0.1, g.y1 - 0.27, 1.3, 0.25, { fontSize: 11, color: ICE, fontFace: CODE });
    });
    label(s, "no MISO pin", 5.55, 2.24, 1.25, 0.24, { fontSize: 10, color: RED, italic: true });

    line(pres, s, x0, 2.5, sx - x0, 0, GREEN, 2);
    label(s, "CS1 -> (GP17)", x0 + 0.1, 2.5 - 0.27, 1.8, 0.25, { fontSize: 11, color: GREEN, fontFace: CODE });
    line(pres, s, x0, 4.3, sx - x0, 0, AMBER, 2);
    label(s, "CS2 ->", x0 + 0.1, 4.3 - 0.27, 1.3, 0.25, { fontSize: 11, color: AMBER, fontFace: CODE });

    label(s, "SCK, MOSI and MISO are shared. CS is per device: LOW = \"you, listen\". All other devices ignore the clock.",
      MARGIN, 4.72, CONTENT_W, 0.35, { fontSize: 12, color: ICE, align: "center" });
  }
  notes(s,
    "Draw attention to CS: active low, one wire per device. A device with CS high must " +
    "leave MISO alone (it goes high-impedance).\n" +
    "The 5110 has no MISO at all: the Pico can talk to it but never hear back. GP16 stays free.\n" +
    "D/C and RST are 5110-specific extras, not part of SPI - they come in part 2.");

  // -------------------------------------------------------------------------
  // 7. One byte on the wire
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "One Command Byte on the Wire", next());
  {
    const x0 = 1.5, x1 = 9.4, b0 = 2.05, p = 0.85, amp = 0.32;
    const rows = [
      { name: "CS", y: 1.35 },
      { name: "SCK", y: 2.0 },
      { name: "MOSI", y: 2.65 },
      { name: "D/C", y: 3.3 },
    ];
    rows.forEach((r) => label(s, r.name, MARGIN, r.y - 0.02, 0.9, 0.35, { fontSize: 13, bold: true, color: WHITE, fontFace: CODE }));
    const csDown = 1.85, csUp = b0 + 8 * p + 0.25;
    const yH = (r) => r.y, yL = (r) => r.y + amp;

    // CS: high, low for the whole byte, high again
    let r = rows[0];
    line(pres, s, x0, yH(r), csDown - x0, 0, GREEN, 2);
    line(pres, s, csDown, yH(r), 0, amp, GREEN, 2);
    line(pres, s, csDown, yL(r), csUp - csDown, 0, GREEN, 2);
    line(pres, s, csUp, yH(r), 0, amp, GREEN, 2);
    line(pres, s, csUp, yH(r), x1 - csUp, 0, GREEN, 2);

    // SCK: idle low, low half then high half in every bit
    r = rows[1];
    line(pres, s, x0, yL(r), b0 - x0, 0, ICE, 2);
    for (let i = 0; i < 8; i++) {
      const bx = b0 + i * p, mid = bx + p / 2;
      line(pres, s, bx, yL(r), p / 2, 0, ICE, 2);
      line(pres, s, mid, yH(r), 0, amp, ICE, 2);
      line(pres, s, mid, yH(r), p / 2, 0, ICE, 2);
      line(pres, s, bx + p, yH(r), 0, amp, ICE, 2);
    }
    line(pres, s, b0 + 8 * p, yL(r), x1 - (b0 + 8 * p), 0, ICE, 2);

    // MOSI: 0x21 = 0010 0001, MSB first
    r = rows[2];
    const bits = [0, 0, 1, 0, 0, 0, 0, 1];
    let prev = 0;
    line(pres, s, x0, yL(r), b0 - x0, 0, CODE_FG_SAFE(), 2);
    bits.forEach((b, i) => {
      const bx = b0 + i * p;
      if (b !== prev) { line(pres, s, bx, yH(r), 0, amp, CODE_FG_SAFE(), 2); }
      line(pres, s, bx, b ? yH(r) : yL(r), p, 0, CODE_FG_SAFE(), 2);
      label(s, String(b), bx, r.y + amp + 0.03, p, 0.28, { fontSize: 13, bold: true, color: b ? AMBER : GRAY, align: "center", fontFace: CODE });
      prev = b;
    });
    if (prev !== 0) { line(pres, s, b0 + 8 * p, yH(r), 0, amp, CODE_FG_SAFE(), 2); }
    line(pres, s, b0 + 8 * p, yL(r), x1 - (b0 + 8 * p), 0, CODE_FG_SAFE(), 2);

    // D/C: low for a command
    r = rows[3];
    line(pres, s, x0, yL(r), x1 - x0, 0, AMBER, 2);
    label(s, "LOW = command", x1 - 1.6, r.y - 0.02, 1.6, 0.3, { fontSize: 11, color: AMBER, align: "right" });

    // sample marks on rising edges
    for (let i = 0; i < 8; i++) {
      const mid = b0 + i * p + p / 2;
      line(pres, s, mid, rows[1].y - 0.1, 0, rows[2].y + amp - rows[1].y + 0.1, AMBER, 0.75, true);
    }
    label(s, "bit 7 (MSB)", b0 - 0.1, 3.75, 1.2, 0.25, { fontSize: 10, color: GRAY });
    label(s, "bit 0", b0 + 7 * p, 3.75, p, 0.25, { fontSize: 10, color: GRAY, align: "center" });
  }
  T.addCallout(pres, s, "info", "0x21 = 0010 0001: function set, extended instructions",
    "The PCD8544 samples MOSI on every rising SCK edge (dashed lines). D/C is read on the 8th edge.",
    MARGIN, 4.1, CONTENT_W, 0.85);
  notes(s,
    "Walk the bits left to right. The master changes MOSI while SCK is low and the " +
    "display reads it on the rising edge - that is mode 0.\n" +
    "8 clocks = 1 byte. At 4 MHz one byte takes 2 us; the whole 504-byte screen about 1 ms.");

  // -------------------------------------------------------------------------
  // 8. Clock modes
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Clock Modes: CPOL and CPHA", next());
  T.addTable(s,
    ["Mode", "CPOL", "CPHA", "SCK idle", "Sampled on"],
    [
      ["0", "0", "0", "low", "rising edge"],
      ["1", "0", "1", "low", "falling edge"],
      ["2", "1", "0", "high", "falling edge"],
      ["3", "1", "1", "high", "rising edge"],
    ],
    MARGIN, BODY_TOP + 0.05, 4.6, [0.7, 0.75, 0.75, 1.0, 1.4], 13);
  T.addBullets(s, [
    "CPOL: the level SCK rests at",
    "CPHA: first or second edge samples",
    "The device decides. The datasheet timing diagram says which",
    "PCD8544: mode 0, MSB first, max 4 MHz",
  ], 5.4, BODY_TOP, 4.1, 2.2, 14);
  T.addCode(s, [
    "const SPISettings LCD_SPI(4000000U, MSBFIRST, SPI_MODE0);",
    "",
    "SPI.beginTransaction(LCD_SPI);   // speed, order, mode",
    "SPI.transfer(byte);              // 8 clocks",
    "SPI.endTransaction();",
  ].join("\n"), MARGIN, 3.25, CONTENT_W, 1.75, 11);
  notes(s,
    "Wrong mode does not fail loudly: the device reads every bit half a clock off and " +
    "you get garbage, or nothing. Session 15 breaks a bus on purpose with a wrong mode.\n" +
    "Question: what if we run 8 MHz? Above the rated 4 MHz - may work on one module, fail on the next.");

  // -------------------------------------------------------------------------
  // 9. No ACK
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "No ACK, No Scanner", next());
  T.addCards(pres, s, [
    { title: "I2C", body: "Every byte gets an ACK or a NACK. A missing device is a NACK on the address. The scanner from session 13 lists who answers.", accent: GREEN },
    { title: "SPI", body: "The master clocks bytes out whether anyone listens or not. A loose wire, a dead display, the wrong mode: no error at all.", accent: RED },
    { title: "So what?", body: "If the screen stays blank, the firmware cannot tell you why. You check wiring, reset, contrast - or you look at the wires.", accent: AMBER },
  ], 1.35, 2.6);
  T.addCallout(pres, s, "info", "Session 15",
    "The logic analyzer shows what actually travels on SCK, MOSI, CS and D/C. Install PulseView at home this week.",
    MARGIN, 4.15, CONTENT_W, 0.85);
  notes(s,
    "This is the motivation for next session. Today, a blank screen means a checklist: " +
    "power, RST, CE, DC, then contrast. That checklist is on the lab slide.");

  // -------------------------------------------------------------------------
  // 10. Section: PCD8544
  // -------------------------------------------------------------------------
  s = T.sectionSlide(pres, { kicker: "PART 2", title: "The PCD8544", note: "The controller inside the Nokia 5110, read from its datasheet" });
  next();
  notes(s, "0:35 - 20 min for part 2. Have the PCD8544 datasheet open on the projector.");

  // -------------------------------------------------------------------------
  // 11. Memory map
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "84 x 48 Pixels in 504 Bytes", next());
  {
    const gx = 1.4, gy = 1.55, gw = 4.62, gh = 2.64, bank = gh / 6;
    box(pres, s, gx, gy, gw, gh, CARD_BG, ICE, 1.25);
    for (let b = 0; b < 6; b++) {
      if (b > 0) { line(pres, s, gx, gy + b * bank, gw, 0, MID, 0.75); }
      label(s, "bank " + b, MARGIN, gy + b * bank + 0.08, 0.85, 0.3, { fontSize: 11, color: GRAY });
    }
    label(s, "x = 0", gx, gy - 0.3, 0.8, 0.25, { fontSize: 11, color: GRAY });
    label(s, "x = 83", gx + gw - 0.8, gy - 0.3, 0.8, 0.25, { fontSize: 11, color: GRAY, align: "right" });
    const colX = gx + 10 * (gw / 84), colW = gw / 84 * 2;
    box(pres, s, colX, gy + bank, colW, bank, AMBER, AMBER);
    label(s, "one byte: x = 10, bank 1", colX + colW + 0.12, gy + bank + 0.1, 2.4, 0.25, { fontSize: 11, color: AMBER });

    // zoom: one byte as 8 pixels, bit 0 on top, value 0x1F
    const zx = 6.75, zy = 1.25, cell = 0.36;
    label(s, "the byte 0x1F", zx - 0.25, zy - 0.02, 2.5, 0.3, { fontSize: 13, bold: true, color: WHITE });
    for (let i = 0; i < 8; i++) {
      const on = i < 5;
      box(pres, s, zx, zy + 0.35 + i * cell, cell, cell, on ? ICE : NAVY, MID, 1);
      label(s, "bit " + i + (on ? "  = 1" : "  = 0"), zx + cell + 0.12, zy + 0.38 + i * cell, 1.5, 0.3, { fontSize: 11, color: on ? ICE : GRAY, fontFace: CODE });
    }
    line(pres, s, colX + colW, gy + bank, zx - colX - colW, zy + 0.35 - gy - bank, AMBER, 0.75, true);
    line(pres, s, colX + colW, gy + 2 * bank, zx - colX - colW, zy + 0.35 + 8 * cell - gy - 2 * bank, AMBER, 0.75, true);
  }
  T.addBullets(s, [
    "6 banks x 84 columns = 504 bytes. Bit 0 is the top pixel",
    "After each data byte X moves right; after x = 83 it wraps to the next bank",
  ], MARGIN, 4.3, 6.0, 0.8, 13);
  notes(s,
    "The key picture of the session. Pixels are not stored row by row: one byte is a " +
    "vertical strip of 8 pixels.\n" +
    "Ask: how many bytes to clear the screen? 504. How to light the single pixel x = 10, y = 9? " +
    "Bank 1 (9 / 8), bit 1 (9 % 8), byte 0x02 - but you overwrite the other 7 pixels of that strip. " +
    "That problem is why libraries keep a framebuffer (part 4).");

  // -------------------------------------------------------------------------
  // 12. Commands
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The Commands You Need (D/C = 0)", next());
  T.addTable(s,
    ["Command", "Byte", "Set", "Meaning"],
    [
      ["Function set", "0x20 / 0x21", "both", "H = 0 basic / H = 1 extended"],
      ["Display control", "0x08 0x0C 0x09 0x0D", "basic", "blank, normal, all on, inverse"],
      ["Set Y (bank)", "0x40 | bank", "basic", "bank 0..5"],
      ["Set X (column)", "0x80 | x", "basic", "column 0..83"],
      ["Temperature coefficient", "0x04 .. 0x07", "extended", "TC 0..3"],
      ["Bias system", "0x10 .. 0x17", "extended", "n = 0..7"],
      ["Set Vop (contrast)", "0x80 | vop", "extended", "vop 0..127"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [2.4, 2.1, 1.2, 3.3], 12);
  T.addCallout(pres, s, "warn", "Same byte, two meanings",
    "0x80 | n is Set X in the basic set and Set Vop in the extended set. Always know which set you are in.",
    MARGIN, 4.15, CONTENT_W, 0.85);
  notes(s,
    "Show where these come from: the instruction table in the PCD8544 datasheet.\n" +
    "Data bytes (D/C = 1) go straight into display RAM at the current X / Y.");

  // -------------------------------------------------------------------------
  // 13. Init sequence
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Bringing It Up", next());
  T.addCode(s, [
    "constexpr uint8_t CMD_FUNC_EXT  = 0x21U; // H = 1",
    "constexpr uint8_t CMD_SET_VOP   = 0x80U; // | vop",
    "constexpr uint8_t CMD_TEMP_0    = 0x04U;",
    "constexpr uint8_t CMD_BIAS_4    = 0x14U;",
    "constexpr uint8_t CMD_FUNC_BAS  = 0x20U; // H = 0",
    "constexpr uint8_t CMD_DISP_NORM = 0x0CU;",
    "constexpr uint8_t VOP_START     = 0x31U; // tune",
    "",
    "writeCommand(CMD_FUNC_EXT);",
    "writeCommand(CMD_SET_VOP | VOP_START);",
    "writeCommand(CMD_TEMP_0);",
    "writeCommand(CMD_BIAS_4);",
    "writeCommand(CMD_FUNC_BAS);",
    "writeCommand(CMD_DISP_NORM);",
  ].join("\n"), MARGIN, 1.1, 5.0, 3.94, 10);
  T.addBullets(s, [
    "First a reset pulse: RST low for a few us, then high",
    "Without it the chip state after power-up is undefined",
    "Vop is the contrast. Every module wants a different value",
    "Blank at every Vop: check RST and CE. Dark blocks: lower Vop",
  ], 5.75, 1.15, 3.75, 3.6, 13);
  notes(s,
    "The reset pulse: digitalWrite(RST, LOW), delayMicroseconds(10U), HIGH. delayMicroseconds " +
    "in setup() is fine - the no-delay rule is about loop().\n" +
    "Vop 0x31 = 49 is a starting point; clones range roughly 0x28 .. 0x48. Bias 0x13 is the " +
    "datasheet value for a 1:48 display; 0x14 is what most hobby code uses. If a module looks " +
    "wrong at every Vop, try the other bias.");

  // -------------------------------------------------------------------------
  // 14. Wiring
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Wiring the 5110 to the Pico", next());
  T.addTable(s,
    ["Module pin", "Signal", "Pico GPIO", "Pico pin"],
    [
      ["RST", "reset, active low", "GP21", "27"],
      ["CE (SCE, CS)", "chip select, active low", "GP17", "22"],
      ["DC", "LOW = command, HIGH = data", "GP20", "26"],
      ["DIN", "MOSI", "GP19", "25"],
      ["CLK", "SCK", "GP18", "24"],
      ["VCC", "3.3 V", "3V3 OUT", "36"],
      ["BL / LIGHT", "backlight", "see whiteboard", "-"],
      ["GND", "ground", "GND", "any, e.g. 23"],
    ],
    MARGIN, BODY_TOP - 0.05, CONTENT_W, [2.0, 3.2, 1.9, 1.9], 12);
  T.addCallout(pres, s, "error", "3V3 OUT, never VBUS",
    "Wire with USB unplugged. Module labels and pin order differ between boards: go by the label, not the position.",
    MARGIN, 4.43, CONTENT_W, 0.62);
  notes(s,
    "Same pin map since session 12: SPI0 on the Arduino-Pico default pins, so SPI.begin() " +
    "needs no pin setup. AHT10 stays on GP4 / GP5 - both fit at once.\n" +
    "Backlight: tell them what you found when checking the modules (to GND or to 3.3 V, resistor or not).");

  // -------------------------------------------------------------------------
  // 15. Section: RAII
  // -------------------------------------------------------------------------
  s = T.sectionSlide(pres, { kicker: "PART 3", title: "RAII: ChipSelect", note: "The constructor takes the bus, the destructor gives it back" });
  next();
  notes(s, "1:05 - 15 min. This was deferred from K3 on purpose: here it solves a real problem.");

  // -------------------------------------------------------------------------
  // 16. The early return trap
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The Early-Return Trap", next());
  T.addCode(s, [
    "void writeData(const uint8_t* b, size_t n) {",
    "  digitalWrite(PIN_CS, LOW);",
    "  if (b == nullptr) {",
    "    return;          // CS stays LOW",
    "  }",
    "  digitalWrite(PIN_DC, HIGH);",
    "  for (size_t i = 0U; i < n; i++) {",
    "    SPI.transfer(b[i]);",
    "  }",
    "  digitalWrite(PIN_CS, HIGH);",
    "}",
  ].join("\n"), MARGIN, 1.1, 4.35, 3.2, 10, "By hand: every exit must remember CS");
  T.addCode(s, [
    "void writeData(const uint8_t* b, size_t n) {",
    "  ChipSelect cs(PIN_CS);",
    "  if (b == nullptr) {",
    "    return;          // CS released",
    "  }",
    "  digitalWrite(PIN_DC, HIGH);",
    "  for (size_t i = 0U; i < n; i++) {",
    "    SPI.transfer(b[i]);",
    "  }",
    "}                    // CS released",
  ].join("\n"), 5.15, 1.1, 4.35, 3.2, 10, "With a guard: the destructor runs on every exit");
  T.addCallout(pres, s, "warn", "CS stuck LOW gives no error",
    "The next transaction starts in the middle of a byte. The screen shows garbage much later, far from the bug.",
    MARGIN, 4.45, CONTENT_W, 0.6);
  notes(s,
    "Ask before showing the right side: what goes wrong on the left? Then: how many exits " +
    "does a real driver function have after a year of changes?\n" +
    "The guard ties CS to a scope. Leaving the scope - by return, by falling off the end - " +
    "releases it.");

  // -------------------------------------------------------------------------
  // 17. ChipSelect class
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The ChipSelect Guard", next());
  T.addCode(s, [
    "class ChipSelect {",
    "  const uint8_t pin_;",
    "public:",
    "  explicit ChipSelect(uint8_t pin) : pin_(pin) {",
    "    SPI.beginTransaction(LCD_SPI);",
    "    digitalWrite(pin_, LOW);",
    "  }",
    "  ~ChipSelect() {",
    "    digitalWrite(pin_, HIGH);",
    "    SPI.endTransaction();",
    "  }",
    "  ChipSelect(const ChipSelect&) = delete;",
    "  ChipSelect& operator=(const ChipSelect&) = delete;",
    "};",
  ].join("\n"), MARGIN, 1.1, 5.2, 3.94, 10);
  T.addBullets(s, [
    "Constructor: take the bus, select the chip",
    "Destructor: release in reverse order",
    "Copying deleted: two guards for one CS would release it twice",
    "Cost after inlining: the same two digitalWrite calls. No heap, no virtual",
  ], 5.95, 1.15, 3.55, 3.6, 13);
  notes(s,
    "Reverse order matters: CS high first, then end the transaction, so another library " +
    "on the same bus never sees our chip selected.\n" +
    "Discussion: what does the guard NOT protect against? Forgetting to create it. " +
    "Holding it across a long wait - other devices on the bus are locked out meanwhile.");

  // -------------------------------------------------------------------------
  // 18. Section: Lab
  // -------------------------------------------------------------------------
  s = T.sectionSlide(pres, { kicker: "PART 4", title: "Lab: Your Own Driver", note: "Checkerboard, then text, then the AHT10 on screen" });
  next();
  notes(s, "1:20. Three stages, all individual. Break between stage 1 and stage 2.");

  // -------------------------------------------------------------------------
  // 19. Lab 1
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Lab 1: Reset, Init, Checkerboard", next());
  T.addBullets(s, [
    "USB unplugged: wire the display as on the wiring slide",
    "pinMode for CS, DC, RST; CS starts HIGH; SPI.begin()",
    "writeCommand(uint8_t) and writeData(uint8_t), each with a ChipSelect",
    "Reset pulse, then the init sequence",
    "Clear the screen: 504 data bytes of 0x00",
    "Fill it with 0xAA, 0x55, 0xAA, ... and sweep Vop until it is crisp",
  ], MARGIN, BODY_TOP, 5.9, 3.3, 14);
  {
    const px = 6.75, py = 1.35, c = 0.11, cols = 24, rws = 16;
    box(pres, s, px - 0.1, py - 0.1, cols * c + 0.2, rws * c + 0.2, "B8C4A0", MID, 1);
    for (let yy = 0; yy < rws; yy++) {
      for (let xx = 0; xx < cols; xx++) {
        if ((xx + yy) % 2 === 0) {
          s.addShape(pres.shapes.RECTANGLE, { x: px + xx * c, y: py + yy * c, w: c, h: c, fill: { color: "1F2A1A" }, line: { color: "1F2A1A", width: 0 } });
        }
      }
    }
    label(s, "Checkpoint before the break", 6.4, 3.3, 3.1, 0.3, { fontSize: 12, bold: true, color: GREEN, align: "center" });
  }
  T.addCallout(pres, s, "info", "Blank screen checklist",
    "3V3 and GND -> RST pulse in setup() -> CE on GP17 -> DC on GP20 -> sweep Vop. Write your Vop value down.",
    MARGIN, 4.4, CONTENT_W, 0.65);
  notes(s,
    "1:20 - 25 min. Goal: a checkerboard on every desk before the break.\n" +
    "Why does 0xAA, 0x55 make a checkerboard? 0xAA = 10101010: alternate pixels in the strip; " +
    "the next column is shifted by one.\n" +
    "Common bugs: forgot pinMode on CS, so it floats; DC never set HIGH for data; Vop far too high.");

  // -------------------------------------------------------------------------
  // 20. Lab 2
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Lab 2: Text from a Font Table", next());
  {
    const glyph = [0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00];
    const gx = 0.95, gy = 1.45, c = 0.36;
    label(s, "'A' in lab/font5x7.h", MARGIN, 1.05, 3.0, 0.3, { fontSize: 13, bold: true, color: WHITE });
    glyph.forEach((byte, col) => {
      for (let bit = 0; bit < 8; bit++) {
        const on = (byte >> bit) & 1;
        box(pres, s, gx + col * c, gy + bit * c, c, c, on ? ICE : NAVY, col === 5 ? GRAY : MID, 0.75);
      }
      label(s, col === 5 ? "-" : byte.toString(16).toUpperCase().padStart(2, "0"),
        gx + col * c, gy + 8 * c + 0.05, c, 0.25, { fontSize: 10, color: col === 5 ? GRAY : AMBER, align: "center", fontFace: CODE });
    });
    label(s, "bit 0", gx - 0.45, gy + 0.02, 0.45, 0.25, { fontSize: 9, color: GRAY });
    label(s, "bit 7", gx - 0.45, gy + 7 * c + 0.02, 0.45, 0.25, { fontSize: 9, color: GRAY });
    label(s, "column bytes, hex", gx - 0.2, gy + 8 * c + 0.32, 2.6, 0.25, { fontSize: 10, color: GRAY, italic: true });
  }
  T.addBullets(s, [
    "Characters 0x20 .. 0x7E, 5 bytes each: row n is character 0x20 + n",
    "Same layout as the display RAM: one byte per column, bit 0 on top",
    "5 columns + 1 blank = 6 pixels: 14 characters x 6 lines",
    "gotoXY(col, line), drawChar(char), drawString(const char*)",
    "Outside 0x20 .. 0x7E: draw '?', never read past the table",
    "Stretch: inverse text by sending ~byte",
  ], 3.55, BODY_TOP, 5.95, 3.6, 14);
  notes(s,
    "1:55 - 30 min. The font is supplied: it is data, not the point of the lab. " +
    "Students copy lab/font5x7.h into their own src folder.\n" +
    "gotoXY: Set X = col * 6, Set Y = line. If text comes out mirrored or upside down, " +
    "they are reading the table rows instead of the columns, or bit 7 is treated as the top.");

  // -------------------------------------------------------------------------
  // 21. Lab 3
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Lab 3: The AHT10 on Your Display", next());
  T.addBullets(s, [
    "One sketch: your Aht10 class and your display code",
    "A new reading every 2 s: millis(), no delay()",
    "Format each line into char line[15]: 14 characters and the terminator",
    "Redraw a line only when its text changed",
    "Breathe on the sensor and watch both numbers move",
  ], MARGIN, BODY_TOP, 5.4, 3.0, 14);
  {
    const lx = 6.3, ly = 1.3, lw = 3.2, lh = 2.4;
    box(pres, s, lx - 0.12, ly - 0.12, lw + 0.24, lh + 0.24, "2B2F36", MID, 1);
    box(pres, s, lx, ly, lw, lh, "B8C4A0", "B8C4A0", 1);
    const scr = ["AHT10 -> 5110", "", "T     23.4 C", "RH    41.7 %", "", "ok  up 12 min"];
    s.addText(T.assertAscii(scr.join("\n"), "lcd mockup"), {
      x: lx + 0.15, y: ly + 0.12, w: lw - 0.3, h: lh - 0.24,
      fontFace: CODE, fontSize: 14, color: "1F2A1A", valign: "top", margin: 0, lineSpacingMultiple: 1.0,
    });
    label(s, "14 x 6 characters", lx, ly + lh + 0.18, lw, 0.3, { fontSize: 11, color: GRAY, align: "center", italic: true });
  }
  T.addCallout(pres, s, "info", "Tip: fixed point",
    "Keep tenths in an int32_t (234 = 23.4 C) and print the two parts with %ld. No float formatting needed.",
    MARGIN, 4.3, CONTENT_W, 0.75);
  notes(s,
    "2:25 - 20 min. snprintf is bounded: it never writes past the 15 bytes. " +
    "Negative temperatures with the fixed-point tip: watch the sign of the tenths part " +
    "(-0.5 C). A good homework question.\n" +
    "Redraw only changed lines: compare with the previous text, keep the old buffer.");

  // -------------------------------------------------------------------------
  // 22. The library version
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The Same Screen through a Library", next());
  T.addCode(s, [
    "Adafruit_PCD8544 lcd(PIN_DC, PIN_CS, PIN_RST);",
    "",
    "lcd.begin();",
    "lcd.setContrast(LCD_CONTRAST);",
    "lcd.clearDisplay();      // RAM buffer only",
    "lcd.setCursor(0, 16);",
    "lcd.print(\"T  \");",
    "lcd.print(temp.temperature, 1);",
    "lcd.display();           // now it is sent",
  ].join("\n"), MARGIN, 1.1, 4.9, 2.7, 10, "demo/src/main.cpp, shortened");
  T.addBullets(s, [
    "A 504-byte framebuffer in RAM: draw into it, then display()",
    "display() sends the changed rectangle; after clearDisplay() that is all 504 bytes",
    "CS, D/C and SPI settings handled inside",
    "Init sequence behind begin(): bias, Vop, no temperature coefficient",
    "Three libraries: PCD8544, GFX, BusIO",
  ], 5.65, 1.15, 3.85, 3.05, 13);
  T.addCallout(pres, s, "info", "Is 504 bytes of RAM a problem?",
    "Not on the RP2040 (264 KB). On an ATmega328 (2 KB) it was a quarter of all RAM.",
    MARGIN, 4.3, CONTENT_W, 0.72);
  notes(s,
    "2:45 - 10 min. Run the demo. The framebuffer solves the problem from the memory map " +
    "slide: setPixel can change one bit and keep the other seven.\n" +
    "begin() first shows the Adafruit splash image that lives in the buffer at start-up; " +
    "the demo clears it.");

  // -------------------------------------------------------------------------
  // 23. Summary
  // -------------------------------------------------------------------------
  const sum = T.summarySlide(pres, next(), [
    "SPI: push-pull, one CS per device, full duplex, no ACK - and so no errors",
    "The device picks the clock mode; the PCD8544 is mode 0, up to 4 MHz",
    "PCD8544 RAM: 6 banks x 84 bytes, one byte = 8 vertical pixels",
    "ChipSelect: CS follows the scope, on every exit",
    "A library buys a framebuffer and fonts for 504 bytes of RAM",
  ],
    "Dashboard on your own driver: T, RH, min / max, SENSOR MISSING and recovery. Install PulseView and check your analyzer. Details: homework/README.md.");
  notes(sum,
    "2:55 - 5 min. Next session: the logic analyzer. Your UART, I2C and SPI code from " +
    "sessions 12-14 is what we capture, so keep it working.");

  await T.save(pres, OUT);
  console.log("OK: " + OUT + " (" + n + " slides)");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});

function CODE_FG_SAFE() { return "A8D8FF"; }
