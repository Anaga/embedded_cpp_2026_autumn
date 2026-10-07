/*
 * lesson12-slides.js
 * Lesson 12 - UART on the Pico (Raspberry Pi Pico / RP2040, Arduino-Pico core)
 *
 * Build:  node lesson12-slides.js   ->  lesson12-uart-on-the-pico.pptx
 * Needs slides-theme.js next to this file.
 */

const T = require("./slides-theme");
const {
  FaUsb, FaLightbulb, FaMicrochip, FaExclamationTriangle,
} = require("react-icons/fa");

const OUT = "lesson12-uart-on-the-pico.pptx";

// ---------------------------------------------------------------------------
// Local drawing helpers
// ---------------------------------------------------------------------------

function seg(pres, slide, x1, y1, x2, y2, opts) {
  const o = opts || {};
  const ln = { color: o.color || T.ICE, width: o.width || 1.5 };
  if (o.dash) { ln.dashType = o.dash; }
  if (o.arrow) { ln.endArrowType = "triangle"; }
  slide.addShape(pres.shapes.LINE, {
    x: Math.min(x1, x2), y: Math.min(y1, y2),
    w: Math.abs(x2 - x1), h: Math.abs(y2 - y1),
    flipH: x2 < x1, flipV: y2 < y1,
    line: ln,
  });
}

function text(slide, str, opts) {
  slide.addText(T.assertAscii(str, "text"), Object.assign({
    fontFace: T.BODY, fontSize: 14, color: T.ICE, margin: 0, valign: "top",
  }, opts));
}

function box(pres, slide, x, y, w, h, border, fill, noShadow) {
  const o = {
    x: x, y: y, w: w, h: h,
    fill: { color: fill || T.CARD_BG },
    line: { color: border || T.MID, width: border ? 2 : 1 },
  };
  if (!noShadow) { o.shadow = T.makeShadow(); }
  slide.addShape(pres.shapes.RECTANGLE, o);
}

function numberedSteps(pres, slide, steps, x, y, w, rowH, fontSize) {
  steps.forEach((st, i) => {
    const yy = y + i * rowH;
    slide.addShape(pres.shapes.OVAL, {
      x: x, y: yy, w: 0.4, h: 0.4, fill: { color: T.CARD_BG }, line: { color: T.ICE, width: 1.5 },
    });
    text(slide, String(i + 1), {
      x: x, y: yy, w: 0.4, h: 0.4, fontSize: 13, bold: true, color: T.WHITE,
      align: "center", valign: "middle",
    });
    text(slide, st, { x: x + 0.55, y: yy - 0.02, w: w - 0.55, h: rowH - 0.04, fontSize: fontSize || 14 });
  });
}

function notes(slide, str) {
  slide.addNotes(T.assertAscii(str, "speaker notes"));
}

// ---------------------------------------------------------------------------
// Deck
// ---------------------------------------------------------------------------

(async () => {
  const pres = T.newDeck(
    "Lesson 12 - UART on the Pico",
    "Lesson 12  |  UART on the Pico"
  );
  let n = 0;
  const next = () => ++n;

  const iconChip = await T.icon(FaMicrochip, "#CADCFC");
  const iconUsb = await T.icon(FaUsb, "#FFB300");
  const iconLed = await T.icon(FaLightbulb, "#FFB300");
  const iconChipAmber = await T.icon(FaMicrochip, "#FFB300");
  const iconWarn = await T.icon(FaExclamationTriangle, "#FF5252");

  // ---- 1. Title -----------------------------------------------------------
  next();
  const title = T.titleSlide(pres, {
    tag: "LESSON 12",
    title: "UART on the Pico",
    subtitle: "Solder the headers, compare three buses, make two boards talk",
    course: "Embedded Software Development in C/C++",
    meta: "Raspberry Pi Pico (RP2040)  |  Arduino-Pico core  |  2026-10-07",
    iconData: iconChip,
  });
  notes(title, "First session on the Pico. The toolchain was homework, so tonight starts with a short check, then soldering, then buses. Porting the K3 classes is deliberately not tonight.");

  // ---- 2. Tonight ---------------------------------------------------------
  {
    const s = T.contentSlide(pres, "Tonight", next());
    const steps = ["Homework check", "Solder headers", "Three buses", "UART", "Lab: chat", "Lab: pot -> blink"];
    const stepW = T.CONTENT_W / steps.length;
    const d = 0.7;
    const cy = 1.95;
    seg(pres, s, T.MARGIN + stepW / 2, cy, T.MARGIN + stepW * (steps.length - 0.5), cy, { color: T.MID, width: 2 });
    steps.forEach((label, i) => {
      const cx = T.MARGIN + i * stepW + stepW / 2;
      const lab = i >= 4;
      s.addShape(pres.shapes.OVAL, {
        x: cx - d / 2, y: cy - d / 2, w: d, h: d,
        fill: { color: lab ? T.AMBER : T.CARD_BG },
        line: { color: lab ? T.AMBER : T.ICE, width: 2 },
      });
      text(s, String(i + 1), {
        x: cx - d / 2, y: cy - d / 2, w: d, h: d,
        fontSize: 20, bold: true, color: lab ? T.DARK_BG : T.WHITE,
        align: "center", valign: "middle", fontFace: T.HEADING,
      });
      text(s, label, {
        x: cx - stepW / 2 + 0.05, y: cy + 0.5, w: stepW - 0.1, h: 0.6,
        fontSize: 14, color: T.WHITE, bold: true, align: "center",
      });
    });
    T.addCallout(pres, s, "info", "Goal for tonight",
      "One Pico reads a potentiometer, the other blinks at the speed it sets - over two wires and a shared ground.",
      T.MARGIN, 3.6, T.CONTENT_W, 0.9);
    notes(s, "Rough timing: homework check 10 min, soldering 40, three buses 20, UART theory 30, chat lab 20, protocol lab 50, summary 10.");
  }

  // ---- 3. Section: homework ----------------------------------------------
  next();
  T.sectionSlide(pres, { kicker: "PART 1", title: "Homework check", note: "What the first Pico taught us" });

  // ---- 4. Three traps -----------------------------------------------------
  {
    const s = T.contentSlide(pres, "Three traps from the first Pico", next());
    T.addCards(pres, s, [
      {
        title: "Upload stops",
        body: "\"picotool was unable to connect\" on Windows. Zadig -> RP2 Boot (Interface 1) -> WinUSB. Never touch Interface 0.",
        iconData: iconUsb, accent: T.AMBER,
      },
      {
        title: "LED stays dark",
        body: "Serial prints, the LED does nothing. pinMode(LED_BUILTIN, OUTPUT) is missing - the pin is not an output yet.",
        iconData: iconLed, accent: T.AMBER,
      },
      {
        title: "\"v2.0\" is not Pico 2",
        body: "INFO_UF2.TXT says v2.0: that is the boot ROM of RP2040 revision B1. The bootloader is in ROM and cannot be updated.",
        iconData: iconChipAmber, accent: T.AMBER,
      },
    ], 1.25, 3.3);
    text(s, "Your pico-setup.md: anything else that bit you?", {
      x: T.MARGIN, y: 4.7, w: T.CONTENT_W, h: 0.35, fontSize: 14, color: T.GRAY, italic: true, align: "center",
    });
    notes(s, "Go round the table: one sentence each from pico-setup.md. Problems that more than one person hit go into the homework README for the next stream.");
  }

  // ---- 5. Section: solder -------------------------------------------------
  next();
  T.sectionSlide(pres, { kicker: "PART 2", title: "Solder the headers", note: "The breadboard is your jig" });

  // ---- 6. Breadboard jig --------------------------------------------------
  {
    const s = T.contentSlide(pres, "The breadboard is your jig", next());
    numberedSteps(pres, s, [
      "Push both 20-pin headers into the breadboard, long pins down, so the Pico will straddle the centre gap",
      "Lay the Pico on top. Check the pins come through every hole",
      "Solder one corner pin on each side. Is the board flat? Fix it now, not later",
      "Solder the other 38 pins: 350 C, 2-3 s per joint. Matte joints are normal with lead-free solder",
      "Let it cool, then lift the Pico out straight",
    ], T.MARGIN, T.BODY_TOP + 0.05, 5.0, 0.75, 14);

    // Diagram: breadboard, centre gap, two header rows, Pico on top.
    const bx = 6.0;
    const by = 1.3;
    const bw = 3.4;
    const bh = 3.5;
    s.addShape(pres.shapes.RECTANGLE, { x: bx, y: by, w: bw, h: bh, fill: { color: "E6E6E6" }, line: { color: T.GRAY, width: 1 } });
    const gapX = bx + bw / 2 - 0.09;
    s.addShape(pres.shapes.RECTANGLE, { x: gapX, y: by, w: 0.18, h: bh, fill: { color: "BDBDBD" }, line: { color: "BDBDBD", width: 0 } });
    // Holes.
    for (let col = 0; col < 10; col++) {
      const hx = col < 5 ? bx + 0.25 + col * 0.26 : gapX + 0.18 + 0.17 + (col - 5) * 0.26;
      for (let row = 0; row < 12; row++) {
        s.addShape(pres.shapes.RECTANGLE, {
          x: hx, y: by + 0.2 + row * 0.27, w: 0.06, h: 0.06, fill: { color: "9E9E9E" }, line: { color: "9E9E9E", width: 0 },
        });
      }
    }
    // Pico board (semi-transparent) and its two header rows.
    const pw = 1.25;
    const ph = 2.9;
    const px = bx + bw / 2 - pw / 2;
    const py = by + 0.3;
    s.addShape(pres.shapes.RECTANGLE, {
      x: px, y: py, w: pw, h: ph, fill: { color: "2E7D32", transparency: 25 }, line: { color: "1B5E20", width: 1.5 },
    });
    s.addShape(pres.shapes.RECTANGLE, {
      x: px + pw / 2 - 0.2, y: py - 0.12, w: 0.4, h: 0.22, fill: { color: "B0BEC5" }, line: { color: "78909C", width: 1 },
    });
    [px + 0.08, px + pw - 0.2].forEach((hx) => {
      s.addShape(pres.shapes.RECTANGLE, { x: hx, y: py + 0.15, w: 0.12, h: ph - 0.3, fill: { color: "212121" }, line: { color: "000000", width: 0 } });
    });
    text(s, "Pico on top", {
      x: px, y: py + ph / 2 - 0.2, w: pw, h: 0.4, fontSize: 12, bold: true, color: T.WHITE, align: "center", valign: "middle",
    });
    text(s, "USB end", { x: px - 0.3, y: by - 0.32, w: pw + 0.6, h: 0.25, fontSize: 11, color: T.GRAY, align: "center" });
    text(s, "headers in the breadboard, centre gap between them", {
      x: bx, y: by + bh + 0.06, w: bw, h: 0.3, fontSize: 11, color: T.GRAY, italic: true, align: "center",
    });
    notes(s, "Three stations plus personal irons, four students: two can start at once, the others place headers and check alignment meanwhile. The first two corner pins decide whether the board ends up flat. Do not hold the iron on the breadboard plastic.");
  }

  // ---- 7. Check before power ----------------------------------------------
  {
    const s = T.contentSlide(pres, "Check before you plug in USB", next());
    const colW = (T.CONTENT_W - 0.3) / 2;
    const cols = [
      {
        title: "Look", items: [
          "Every joint with the loupe",
          "A small cone around the pin, no ball, no gap",
          "No bridge between neighbouring pins",
        ],
      },
      {
        title: "Measure", items: [
          "Multimeter on continuity",
          "Neighbouring pins must not beep",
          "GND pins beep with each other: they are one net, that is correct",
        ],
      },
    ];
    cols.forEach((c, i) => {
      const x = T.MARGIN + i * (colW + 0.3);
      box(pres, s, x, 1.2, colW, 2.45, T.ICE);
      text(s, c.title, {
        x: x + 0.25, y: 1.32, w: colW - 0.5, h: 0.4, fontFace: T.HEADING, fontSize: 18, bold: true, color: T.WHITE,
      });
      T.addBullets(s, c.items, x + 0.25, 1.8, colW - 0.45, 1.8, 15);
    });
    T.addCallout(pres, s, "ok", "Then the real test",
      "Plug in USB. Your blink from the homework must still run. If the LED is dark or the board gets warm: unplug and look again.",
      T.MARGIN, 3.9, T.CONTENT_W, 0.95);
    notes(s, "Same continuity method as session 4. A bridge between 3V3 and GND would short the regulator, so the check comes before USB, not after.");
  }

  // ---- 8. Section: buses --------------------------------------------------
  next();
  T.sectionSlide(pres, { kicker: "PART 3", title: "Three buses", note: "UART, I2C, SPI - one table" });

  // ---- 9. Bus table -------------------------------------------------------
  {
    const s = T.contentSlide(pres, "UART vs I2C vs SPI", next());
    T.addTable(s,
      ["", "UART", "I2C", "SPI"],
      [
        ["Signal wires", "TX, RX", "SDA, SCL", "SCK, MOSI, MISO, CS"],
        ["Clock", "none - agreed baud rate", "SCL, from controller", "SCK, from controller"],
        ["Devices", "2, point to point", "many, by address", "many, one CS each"],
        ["Direction", "both at once", "one at a time", "both at once"],
        ["Typical speed", "9600 - 115200 baud", "100 / 400 kHz", "1 - tens of MHz"],
        ["Outputs", "push-pull", "open-drain + pull-ups", "push-pull"],
        ["In this course", "tonight: board to board", "session 13: OPT4001", "session 14: Nokia 5110"],
      ],
      T.MARGIN, 1.2, T.CONTENT_W, [2.0, 2.3, 2.3, 2.4], 13);
    text(s, "Plus GND on every bus: a voltage only means something against a shared reference.", {
      x: T.MARGIN, y: 4.25, w: T.CONTENT_W, h: 0.4, fontSize: 14, color: T.ICE, italic: true,
    });
    notes(s, "Do not go deep on I2C and SPI tonight; the table is the map for the next two sessions. Point at the Clock row: UART is the only one without a clock wire, which is why both sides must agree on the speed in advance.");
  }

  // ---- 10. Pin map --------------------------------------------------------
  {
    const s = T.contentSlide(pres, "The Pico pin map for the rest of K4", next());
    T.addTable(s,
      ["Function", "GPIO", "Pin", "When"],
      [
        ["UART0 (Serial1) TX / RX", "GP0 / GP1", "1 / 2", "tonight"],
        ["On-board LED", "GP25", "-", "tonight"],
        ["I2C0 SDA / SCL (OPT4001)", "GP4 / GP5", "6 / 7", "session 13"],
        ["SPI0 MISO / CS / SCK / MOSI", "GP16 / 17 / 18 / 19", "21 / 22 / 24 / 25", "session 14"],
        ["Nokia 5110 D/C / RST", "GP20 / GP21", "26 / 27", "session 14"],
        ["RGB LED R / G / B", "GP10 / GP11 / GP12", "14 / 15 / 16", "later"],
        ["Buttons A / B", "GP14 / GP15", "19 / 20", "later"],
        ["Potentiometer (ADC0)", "GP26", "31", "tonight"],
      ],
      T.MARGIN, 1.2, T.CONTENT_W, [3.3, 2.1, 2.1, 1.5], 13);
    text(s, "The buses sit on the Arduino-Pico default pins, so Serial1, Wire and SPI work without setTX() or setSDA().", {
      x: T.MARGIN, y: 4.45, w: T.CONTENT_W, h: 0.5, fontSize: 13, color: T.GRAY, italic: true,
    });
    notes(s, "GND for the UART wire is physical pin 3, right next to GP0 and GP1. Everything on this map can be wired at the same time - no pin is used twice.");
  }

  // ---- 11. Section: UART --------------------------------------------------
  next();
  T.sectionSlide(pres, { kicker: "PART 4", title: "UART", note: "Two wires, no clock" });

  // ---- 12. Frame ----------------------------------------------------------
  {
    const s = T.contentSlide(pres, "No clock wire: one byte on the line", next());
    const cells = [
      { l: "idle", v: 1 }, { l: "start", v: 0 },
      { l: "d0", v: 1 }, { l: "d1", v: 0 }, { l: "d2", v: 0 }, { l: "d3", v: 0 },
      { l: "d4", v: 0 }, { l: "d5", v: 0 }, { l: "d6", v: 1 }, { l: "d7", v: 0 },
      { l: "stop", v: 1 }, { l: "idle", v: 1 },
    ];
    const x0 = 1.3;
    const x1 = 9.3;
    const cw = (x1 - x0) / cells.length;
    const yHi = 1.6;
    const yLo = 2.5;
    // Data bit background.
    s.addShape(pres.shapes.RECTANGLE, {
      x: x0 + 2 * cw, y: yHi - 0.15, w: 8 * cw, h: yLo - yHi + 0.3, fill: { color: T.CARD_BG }, line: { color: T.CARD_BG, width: 0 },
    });
    text(s, "HIGH", { x: T.MARGIN, y: yHi - 0.15, w: 0.75, h: 0.3, fontSize: 11, color: T.GRAY });
    text(s, "LOW", { x: T.MARGIN, y: yLo - 0.15, w: 0.75, h: 0.3, fontSize: 11, color: T.GRAY });
    let prevY = null;
    cells.forEach((c, i) => {
      const xa = x0 + i * cw;
      const xb = xa + cw;
      const y = c.v ? yHi : yLo;
      const col = c.l === "start" || c.l === "stop" ? T.AMBER : T.GREEN;
      if (prevY !== null && prevY !== y) { seg(pres, s, xa, prevY, xa, y, { color: T.GREEN, width: 2.5 }); }
      seg(pres, s, xa, y, xb, y, { color: c.l.startsWith("d") ? T.GREEN : col, width: 2.5 });
      prevY = y;
      text(s, c.l, {
        x: xa, y: yLo + 0.25, w: cw, h: 0.3, fontSize: 11, align: "center",
        color: c.l.startsWith("d") ? T.WHITE : T.GRAY, fontFace: T.CODE,
      });
      if (c.l.startsWith("d")) {
        text(s, String(c.v), {
          x: xa, y: yLo + 0.55, w: cw, h: 0.3, fontSize: 13, bold: true, align: "center", color: T.GREEN, fontFace: T.CODE,
        });
      }
    });
    text(s, "'A' = 0x41 = 0100 0001, sent least significant bit first", {
      x: T.MARGIN, y: 3.55, w: T.CONTENT_W, h: 0.35, fontSize: 14, color: T.WHITE, bold: true, align: "center",
    });
    T.addCallout(pres, s, "info", "8N1",
      "8 data bits, No parity, 1 stop bit. The line idles HIGH; the falling edge of the start bit is the only timing reference the receiver gets.",
      T.MARGIN, 4.0, T.CONTENT_W, 0.95);
    notes(s, "Read the waveform with them bit by bit. Ask: how does the receiver know where d0 starts? Answer: it sees the falling edge of the start bit and then counts bit times from its own clock. That is why both sides must agree on the baud rate. Session 15 shows exactly this frame on the logic analyzer.");
  }

  // ---- 13. Numbers --------------------------------------------------------
  {
    const s = T.contentSlide(pres, "115200 baud in numbers", next());
    const chain = [
      { v: "115200", l: "bits per second" },
      { op: "->" },
      { v: "8.68 us", l: "one bit", accent: T.ICE },
      { op: "x" },
      { v: "10", l: "bits per byte (8N1)" },
      { op: "=" },
      { v: "86.8 us", l: "one byte", accent: T.AMBER },
    ];
    const statW = 1.75;
    const opW = (T.CONTENT_W - statW * 4) / 3;
    let x = T.MARGIN;
    chain.forEach((c) => {
      if (c.op) {
        text(s, c.op, {
          x: x, y: 1.4, w: opW, h: 1.25, fontSize: 26, bold: true, color: T.GRAY, align: "center", valign: "middle", fontFace: T.HEADING,
        });
        x += opW;
        return;
      }
      box(pres, s, x, 1.4, statW, 1.25, c.accent);
      text(s, c.v, {
        x: x, y: 1.55, w: statW, h: 0.6, fontSize: 24, bold: true, color: T.WHITE, align: "center", valign: "middle", fontFace: T.HEADING,
      });
      text(s, c.l, { x: x + 0.05, y: 2.2, w: statW - 0.1, h: 0.35, fontSize: 12, color: T.ICE, align: "center" });
      x += statW;
    });
    T.addBullets(s, [
      "At most 11520 bytes per second - a full line of text takes a few milliseconds",
      "Both sides count bit times on their own clock. If the two baud rates differ by more than a few percent, the receiver samples the wrong bits",
      "Typical rates: 9600, 115200. Pick one and use it on both boards",
    ], T.MARGIN, 3.0, T.CONTENT_W, 1.95, 15);
    notes(s, "1 / 115200 = 8.68 us. 10 bits per byte because start and stop are overhead: 20 percent of the line is framing.");
  }

  // ---- 14. Serial vs Serial1 ----------------------------------------------
  {
    const s = T.contentSlide(pres, "Serial is not Serial1", next());
    T.addCards(pres, s, [
      { title: "Serial", body: "USB CDC to your PC. Not a UART at all - the baud rate you pass is ignored.", accent: T.ICE },
      { title: "Serial1", body: "UART0, hardware. GP0 = TX, GP1 = RX. Tonight's wire to your partner.", accent: T.AMBER },
      { title: "Serial2", body: "UART1, hardware. Default GP8 = TX, GP9 = RX. Tonight's homework: two UARTs, one board.", accent: null },
    ], 1.25, 2.3);
    T.addCallout(pres, s, "info", "Two hardware UARTs",
      "The RP2040 has two UART blocks (ARM PL011) with small FIFOs. The pins can be moved with setTX() / setRX() before begin() - our pin map keeps the defaults.",
      T.MARGIN, 3.85, T.CONTENT_W, 1.0);
    notes(s, "Same idea as on the ESP32-C3: what the course called Serial so far was USB all along. Tonight is the first time a real UART pin carries data.");
  }

  // ---- 15. Wiring two boards ----------------------------------------------
  {
    const s = T.contentSlide(pres, "Wiring two boards", next());
    const ax = 0.8;
    const bxx = 6.6;
    const bw = 2.6;
    const top = 1.3;
    const h = 2.35;
    box(pres, s, ax, top, bw, h, T.ICE);
    box(pres, s, bxx, top, bw, h, T.ICE);
    text(s, "Pico A", { x: ax + 0.2, y: top + 0.12, w: 1.5, h: 0.35, fontFace: T.HEADING, fontSize: 16, bold: true, color: T.WHITE });
    text(s, "Pico B", { x: bxx + bw - 1.7, y: top + 0.12, w: 1.5, h: 0.35, fontFace: T.HEADING, fontSize: 16, bold: true, color: T.WHITE, align: "right" });
    const rows = [
      { a: "GP0  TX  (pin 1)", b: "(pin 1)  TX  GP0", y: top + 0.75 },
      { a: "GP1  RX  (pin 2)", b: "(pin 2)  RX  GP1", y: top + 1.3 },
      { a: "GND      (pin 3)", b: "(pin 3)      GND", y: top + 1.85 },
    ];
    rows.forEach((r) => {
      text(s, r.a, { x: ax + 0.15, y: r.y - 0.17, w: bw - 0.3, h: 0.34, fontFace: T.CODE, fontSize: 12, color: T.CODE_FG, align: "right", valign: "middle" });
      text(s, r.b, { x: bxx + 0.15, y: r.y - 0.17, w: bw - 0.3, h: 0.34, fontFace: T.CODE, fontSize: 12, color: T.CODE_FG, valign: "middle" });
    });
    const xa = ax + bw;
    const xb = bxx;
    // A.TX -> B.RX and B.TX -> A.RX, crossing in the middle.
    seg(pres, s, xa, rows[0].y, xb, rows[1].y, { color: T.AMBER, width: 2.5, arrow: true });
    seg(pres, s, xb, rows[0].y, xa, rows[1].y, { color: T.GREEN, width: 2.5, arrow: true });
    seg(pres, s, xa, rows[2].y, xb, rows[2].y, { color: T.GRAY, width: 3 });
    text(s, "TX goes to RX - crossed", {
      x: xa, y: top + 0.25, w: xb - xa, h: 0.3, fontSize: 12, color: T.ICE, italic: true, align: "center",
    });
    text(s, "shared ground", {
      x: xa, y: rows[2].y + 0.08, w: xb - xa, h: 0.3, fontSize: 12, color: T.GRAY, italic: true, align: "center",
    });
    s.addImage({ data: iconWarn, x: T.MARGIN + 0.2, y: 3.98, w: 0.4, h: 0.4 });
    text(s, "Never connect 3V3 or VBUS between the boards. Each Pico is powered by its own USB cable - only GND is shared. 3.3 V logic only: no 5 V, no RS-232.", {
      x: T.MARGIN + 0.8, y: 3.9, w: T.CONTENT_W - 0.8, h: 0.9, fontSize: 14, color: T.ICE, valign: "middle",
    });
    notes(s, "Physical pins 1, 2, 3 are next to each other on both boards, so three short jumpers do it. If a pair cannot reach, use the two breadboards side by side.");
  }

  // ---- 16. Bridge ---------------------------------------------------------
  {
    const s = T.contentSlide(pres, "The bridge: what you type goes to your partner", next());
    T.addCode(s, [
      "static const uint32_t USB_BAUD  = 115200U;  // ignored by USB",
      "static const uint32_t UART_BAUD = 115200U;  // both boards!",
      "void setup() {",
      "    Serial.begin(USB_BAUD);     // USB to your PC",
      "    Serial1.begin(UART_BAUD);   // UART0: GP0 TX, GP1 RX",
      "}",
      "void loop() {",
      "    while (Serial.available() > 0) {    // PC -> partner",
      "        Serial1.write(static_cast<uint8_t>(Serial.read()));",
      "    }",
      "    while (Serial1.available() > 0) {   // partner -> PC",
      "        Serial.write(static_cast<uint8_t>(Serial1.read()));",
      "    }",
      "}",
    ].join("\n"), T.MARGIN, T.BODY_TOP, 5.6, 3.85, 9.5);
    const rx = 6.3;
    const rw = 3.2;
    text(s, "platformio.ini", { x: rx, y: T.BODY_TOP, w: rw, h: 0.3, fontSize: 13, bold: true, color: T.WHITE });
    T.addCode(s, [
      "monitor_speed = 115200",
      "monitor_echo = yes",
      "monitor_filters = send_on_enter",
    ].join("\n"), rx, T.BODY_TOP + 0.35, rw, 1.1, 10);
    T.addBullets(s, [
      "echo: see what you type",
      "send_on_enter: send the whole line on Enter",
      "Lines end with CR LF - remember that for the protocol",
    ], rx, 2.75, rw, 2.2, 13);
    notes(s, "Without send_on_enter the monitor sends every key as you press it; without echo you type blind. monitor_eol defaults to CRLF, so the partner receives text followed by \\r\\n. The protocol stage has to deal with that.");
  }

  // ---- 17. Section: lab ---------------------------------------------------
  next();
  T.sectionSlide(pres, { kicker: "PART 5", title: "Lab", note: "Chat first, then a sensor on the line" });

  // ---- 18. Stage 1 --------------------------------------------------------
  {
    const s = T.contentSlide(pres, "Stage 1: chat", next());
    numberedSteps(pres, s, [
      "Pair up: one Pico A, one Pico B",
      "Wire TX -> RX, RX <- TX, GND - GND (pins 1, 2, 3)",
      "Flash the bridge on both boards, open both monitors",
      "Type a line and press Enter: it appears on your partner's screen",
      "Pull out one data wire. Which direction stops working - and why only that one?",
    ], T.MARGIN, T.BODY_TOP + 0.05, T.CONTENT_W, 0.56, 15);
    T.addCallout(pres, s, "warn", "Nothing arrives?",
      "Check in this order: TX and RX crossed? GND connected? Same baud on both? Monitor open on the right COM port?",
      T.MARGIN, 4.1, T.CONTENT_W, 0.85);
    notes(s, "Two pairs. Step 5 answer: each wire carries one direction only - UART is two independent one-way lines.");
  }

  // ---- 19. Who talks first ------------------------------------------------
  {
    const s = T.contentSlide(pres, "Who talks first?", next());
    text(s, "UART has no master: two independent wires, either side may talk at any time. The protocol decides who leads.", {
      x: T.MARGIN, y: 1.15, w: T.CONTENT_W, h: 0.6, fontSize: 15, color: T.ICE,
    });
    T.addCards(pres, s, [
      { title: "Stream", body: "The pot board sends POT 512 every 100 ms on its own. The LED board only listens. Simple - but who notices when it goes quiet?" },
      { title: "Poll", body: "The LED board asks POT?, the pot board answers POT 512. The one who needs the data sets the pace." },
      { title: "Command", body: "The pot board works out the period itself and sends PERIOD 300. Should the sensor own that maths?" },
    ], 1.9, 2.3);
    text(s, "Which board should lead - and why?", {
      x: T.MARGIN, y: 4.4, w: T.CONTENT_W, h: 0.45, fontFace: T.HEADING, fontSize: 18, bold: true, color: T.AMBER, align: "center",
    });
    notes(s, "Let them argue before the next slide. All three are real: GPS modules stream NMEA, I2C sensors are polled, actuators take commands. We pick Poll: the LED board needs the data, so it decides when to ask - and session 13 works exactly like this, with the I2C bus enforcing the roles.");
  }

  // ---- 20. Stage 2 spec ---------------------------------------------------
  {
    const s = T.contentSlide(pres, "Stage 2: the LED board leads", next());
    T.addTable(s,
      ["Who", "Sends", "When"],
      [
        ["LED board (master)", "POT?", "every 100 ms"],
        ["Pot board (slave)", "POT <0..1023>", "only as the answer to POT?"],
      ],
      T.MARGIN, 1.2, T.CONTENT_W, [3.0, 2.6, 3.4], 14);
    T.addBullets(s, [
      "Master: map the value to a blink half-period of 50 .. 500 ms and blink GP25 - no delay()",
      "Master: no answer within 200 ms -> print \"no reply\", LED off. Back to normal when answers return",
      "Slave: never talks first. Anything other than POT? is ignored",
      "Every message is one line: ends at '\\n', '\\r' ignored, at most 31 characters",
    ], T.MARGIN, 2.5, T.CONTENT_W, 2.5, 15);
    notes(s, "Pairs: one board is master, one is slave, then swap roles. A reply is about 10 bytes, under 1 ms at 115200, so 200 ms is a generous timeout. The master never sends a new POT? while it is still waiting.");
  }

  // ---- 21. Wiring the pot -------------------------------------------------
  {
    const s = T.contentSlide(pres, "Wiring the potentiometer (pot board only)", next());
    const lx = 0.8;
    const lw = 2.9;
    const rx = 6.3;
    const rw = 2.9;
    const top = 1.3;
    const h = 2.3;
    box(pres, s, lx, top, lw, h, T.ICE);
    box(pres, s, rx, top, rw, h, T.ICE);
    text(s, "Pico", { x: lx + 0.2, y: top + 0.1, w: 1.5, h: 0.35, fontFace: T.HEADING, fontSize: 16, bold: true, color: T.WHITE });
    text(s, "10k potentiometer", { x: rx + 0.2, y: top + 0.1, w: rw - 0.4, h: 0.35, fontFace: T.HEADING, fontSize: 16, bold: true, color: T.WHITE, align: "right" });
    const rows = [
      { a: "GP26 / ADC0  (pin 31)", b: "middle pin (wiper)", c: T.AMBER, y: top + 0.8 },
      { a: "AGND         (pin 33)", b: "outer pin", c: T.GRAY, y: top + 1.35 },
      { a: "3V3 OUT      (pin 36)", b: "other outer pin", c: T.RED, y: top + 1.9 },
    ];
    rows.forEach((r) => {
      text(s, r.a, { x: lx + 0.1, y: r.y - 0.17, w: lw - 0.2, h: 0.34, fontFace: T.CODE, fontSize: 12, color: T.CODE_FG, align: "right", valign: "middle" });
      text(s, r.b, { x: rx + 0.15, y: r.y - 0.17, w: rw - 0.3, h: 0.34, fontSize: 13, color: T.ICE, valign: "middle" });
      seg(pres, s, lx + lw, r.y, rx, r.y, { color: r.c, width: 2.5 });
    });
    T.addCallout(pres, s, "info", "No 3.3k resistor this time",
      "On the C3 it worked around the ADC range. The Pico ADC reads the whole 0 .. 3.3 V. analogRead() gives 0 .. 1023 by default (10 bits); analogReadResolution(12) gives 0 .. 4095.",
      T.MARGIN, 3.85, T.CONTENT_W, 1.0);
    notes(s, "Pins 31, 33 and 36 sit together near the bottom of the right-hand header. AGND is the ADC's own ground reference; any GND works, AGND is the clean choice. If the value runs backwards, swap the two outer wires.");
  }

  // ---- 22. Line assembly --------------------------------------------------
  {
    const s = T.contentSlide(pres, "Building a line, one byte at a time", next());
    T.addCode(s, [
      "static const size_t LINE_MAX = 31U;",
      "static char   s_line[LINE_MAX + 1U];   // + 1 for '\\0'",
      "static size_t s_len = 0U;",
      "static bool readLine(Stream& in) {     // true: s_line is complete",
      "    while (in.available() > 0) {",
      "        const char c = static_cast<char>(in.read());",
      "        if (c == '\\r') { continue; }   // CR LF -> drop CR",
      "        if (c == '\\n') {",
      "            s_line[s_len] = '\\0';   s_len = 0U;",
      "            return true;",
      "        }",
      "        if (s_len < LINE_MAX) { s_line[s_len++] = c; }",
      "    }",
      "    return false;",
      "}",
    ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.85, 9);
    T.addBullets(s, [
      "Bytes arrive one by one, at any time - loop() collects them",
      "A char array from session 7, no String, no heap",
      "Slave: strcmp(s_line, \"POT?\"). Master: strncmp(s_line, \"POT \", 4U), then strtoul() for the number",
      "A cut or garbled line is not a valid reply",
    ], 6.6, T.BODY_TOP, 2.9, 3.85, 13);
    notes(s, "Same function on both boards; both read Serial1. Stream& works for Serial, Serial1 and Serial2 - all derive from Stream. Homework hint for later: one board with two UARTs needs two buffers, which is a good reason for a small LineReader class.");
  }

  // ---- 23. Master state machine -------------------------------------------
  {
    const s = T.contentSlide(pres, "The master is a state machine", next());
    const bw = 2.3;
    const ax = 0.8;
    const bx = 9.2 - bw;
    const top = 1.3;
    const h = 2.1;
    const states = [
      { x: ax, t: "IDLE", sub: "blinking at the last period" },
      { x: bx, t: "WAITING", sub: "POT? sent, clock running" },
    ];
    states.forEach((st) => {
      box(pres, s, st.x, top, bw, h, T.ICE);
      text(s, st.t, { x: st.x, y: top + 0.55, w: bw, h: 0.45, fontFace: T.HEADING, fontSize: 20, bold: true, color: T.WHITE, align: "center" });
      text(s, st.sub, { x: st.x + 0.1, y: top + 1.05, w: bw - 0.2, h: 0.6, fontSize: 12, color: T.ICE, align: "center" });
    });
    const x1 = ax + bw;
    const x2 = bx;
    const arrows = [
      { y: top + 0.45, dir: 1, c: T.ICE, t: "100 ms since the last POT?  /  send POT?" },
      { y: top + 1.15, dir: -1, c: T.GREEN, t: "reply POT n  /  new blink period" },
      { y: top + 1.85, dir: -1, c: T.RED, t: "200 ms, no reply  /  LED off, print no reply" },
    ];
    arrows.forEach((a) => {
      if (a.dir > 0) { seg(pres, s, x1 + 0.05, a.y, x2 - 0.05, a.y, { color: a.c, width: 2, arrow: true }); }
      else { seg(pres, s, x2 - 0.05, a.y, x1 + 0.05, a.y, { color: a.c, width: 2, arrow: true }); }
      text(s, a.t, { x: x1 + 0.1, y: a.y - 0.32, w: x2 - x1 - 0.2, h: 0.28, fontSize: 12, color: a.c, align: "center" });
    });
    T.addCallout(pres, s, "ok", "Fail safe",
      "A LED that keeps blinking after its sensor is gone is lying - silence is information. And roles stop the ping-pong two equal boards can get into: the slave never talks first, the master never answers.",
      T.MARGIN, 3.7, T.CONTENT_W, 1.15);
    notes(s, "enum class MasterState { Idle, Waiting } and a switch, exactly as in K3. The blink runs independently of both states, driven by millis(). Discussion: should one missed reply already switch the LED off, or three in a row? Real links (an RC transmitter, for example) count several misses before they declare failsafe.");
  }

  // ---- 24. Summary --------------------------------------------------------
  {
    const s = T.summarySlide(pres, next(), [
      "Headers soldered, checked by eye and by continuity before power",
      "UART: no clock wire, both sides agree on the baud rate. 8N1 = 10 bits per byte",
      "Serial is USB; Serial1 = UART0 (GP0 / GP1); Serial2 = UART1 (GP8 / GP9)",
      "TX to RX crossed, GND shared, never 3V3 or VBUS between boards",
      "The protocol decides the roles: the master asks, the slave answers, silence means fail safe",
    ],
    "One Pico, two UARTs: the pot side answers on UART0, the LED side asks on UART1. Wires GP0 -> GP9 and GP8 -> GP1. Task in homework/README.md.");
    notes(s, "Everyone leaves with a soldered, tested board. Session 13 adds the OPT4001 light sensor on I2C (GP4 / GP5) - the same master/slave idea, this time enforced by the bus.");
  }

  await T.save(pres, OUT);
  console.log("OK: " + OUT + " (" + n + " slides)");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
