/*
 * lesson15.js - Lesson 15: Logic Analyzer and Bus Diagnosis
 *
 * Build:
 *     npm install pptxgenjs react react-dom react-icons sharp
 *     node lesson15.js            -> lesson15.pptx
 *
 * Needs slides-theme.js and aht10-capture-cropped.png next to this file.
 * Speaker notes carry timing. They are published with the deck, so they
 * must never contain the mystery firmware answer key (that lives in
 * solution/ only).
 */

const fs = require("fs");
const T = require("./slides-theme");
const { MARGIN, CONTENT_W, BODY_TOP, NAVY, CARD_BG, MID, ICE, WHITE, GRAY,
  GREEN, AMBER, RED, BODY, HEADING, CODE } = T;

const FOOTER = "Lesson 15  |  Logic Analyzer and Bus Diagnosis";
const CAPTURE_PNG = "aht10-capture-cropped.png";
const CAPTURE_PX_W = 971;
const CAPTURE_PX_H = 299;

function notes(slide, lines) {
  slide.addNotes(T.assertAscii(lines.join("\n"), "speaker notes"));
}

// Small plain text box (ASCII-checked).
function text(slide, s, opts) {
  slide.addText(T.assertAscii(s, "text"), Object.assign({
    fontFace: BODY, fontSize: 12, color: ICE, margin: 0, valign: "top",
  }, opts));
}

// Digital waveform from a list of levels (0/1), one per bit cell.
// Draws horizontal runs and vertical edges as thin rectangles.
function waveform(pres, slide, levels, labels, x, y, cellW, h, color) {
  const lineT = 0.035;
  const c = color || ICE;
  levels.forEach((lv, i) => {
    const x0 = x + i * cellW;
    const yy = lv ? y : y + h;
    slide.addShape(pres.shapes.RECTANGLE, {
      x: x0, y: yy - lineT / 2, w: cellW, h: lineT, fill: { color: c }, line: { color: c, width: 0 },
    });
    if (i > 0 && levels[i - 1] !== lv) {
      slide.addShape(pres.shapes.RECTANGLE, {
        x: x0 - lineT / 2, y: y, w: lineT, h: h, fill: { color: c }, line: { color: c, width: 0 },
      });
    }
    // Cell grid for the bit cells that carry a label.
    if (labels[i]) {
      slide.addShape(pres.shapes.RECTANGLE, {
        x: x0, y: y - 0.12, w: 0.01, h: h + 0.24, fill: { color: MID }, line: { color: MID, width: 0 },
      });
      text(slide, labels[i], {
        x: x0, y: y + h + 0.12, w: cellW, h: 0.25, align: "center", fontSize: 11, color: GRAY,
      });
      text(slide, String(lv), {
        x: x0, y: y - 0.38, w: cellW, h: 0.25, align: "center", fontSize: 11,
        color: lv ? WHITE : AMBER, bold: true,
      });
    }
  });
}

(async () => {
  const pres = T.newDeck("Lesson 15 - Logic Analyzer and Bus Diagnosis", FOOTER);
  let n = 1;
  let s;

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  s = T.titleSlide(pres, {
    tag: "LESSON 15",
    title: "Logic Analyzer",
    subtitle: "UART, I2C and SPI on the wire - and finding what is broken",
    course: "Embedded Software Development in C/C++",
    meta: "Raspberry Pi Pico  |  PulseView + fx2lafw  |  Session 15",
  });
  notes(s, [
    "Timing (4 x 45 min, about 180 min):",
    "  0:00  Homework check, PulseView status            20 min",
    "  0:20  Part 1 - why SPI0 sits on GP16-19           15 min",
    "  0:35  Part 2 - the analyzer, wiring the harness   15 min",
    "  0:50  UART capture                                20 min",
    "  1:10  I2C capture                                 25 min",
    "  1:35  SPI capture and the library on the wire     30 min",
    "  2:05  Diagnosis lab, mystery firmware             45 min",
    "  2:50  Summary and homework                        10 min",
    "Two analyzers: two stations, students in pairs for every capture.",
    "Flash the demo on both station boards before the session.",
  ]);

  // -------------------------------------------------------------------------
  // 2. Goals
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Today", ++n);
  T.addBullets(s, [
    "Explain why SPI0 sits on GP16-19, and what happens to the hardware CS",
    "Set up a capture: channels, sample rate, duration, trigger",
    "Read UART, I2C and SPI traffic from your own board, with and without a decoder",
    "See what a library does on the bus that its API hides",
    "Find a broken bus by evidence, not by guessing",
  ], MARGIN, BODY_TOP, CONTENT_W, 2.9);
  T.addCallout(pres, s, "info", "Rule for today",
    "If you did not capture it, you do not know it.", MARGIN, 4.15, CONTENT_W, 0.75);
  notes(s, [
    "The analyzer closes the gap from session 14: SPI has no ACK, so a wrong",
    "wire gives no error. Today the wire itself becomes visible.",
  ]);

  // -------------------------------------------------------------------------
  // 3. Homework check
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Homework Check: Session 14", ++n);
  T.addCards(pres, s, [
    { title: "Button", body: "GP14, INPUT_PULLUP. Which debounce did you pick, and why?" },
    { title: "Screen 1", body: "Only the two values redrawn each second. How did you avoid a full redraw?" },
    { title: "Screen 2", body: "Bars = average of 5 readings, last 14, auto scale. Does the history keep filling on screen 1?" },
  ], 1.3, 1.9);
  T.addCallout(pres, s, "warn", "PulseView",
    "Installed, device found? If not, we fix it at the break - not during the capture labs.",
    MARGIN, 3.5, CONTENT_W, 0.85);
  notes(s, [
    "20 min. Each student shows both screens and the button.",
    "Screen 1 question leads into slide 19: drawing only what changed keeps",
    "the library's dirty rectangle small. Today we will see that on the wire.",
    "Collect who has PulseView working. Mac: Rosetta 2 and Open Anyway.",
  ]);

  // -------------------------------------------------------------------------
  // 4. Section: SPI0 pins
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 1", title: "Why SPI0 Sits on GP16-19", note: "A question from session 14" });
  n++;

  // -------------------------------------------------------------------------
  // 5. SPI0 signal table
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "SPI0: Four Positions per Signal", ++n);
  T.addTable(s,
    ["SPI0 signal", "Option 1", "Option 2", "Option 3", "Option 4"],
    [
      ["RX (MISO)", "GP0", "GP4", "GP16", "GP20"],
      ["CSn", "GP1", "GP5", "GP17", "GP21"],
      ["SCK", "GP2", "GP6", "GP18", "GP22"],
      ["TX (MOSI)", "GP3", "GP7", "GP19", "GP23"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [2.2, 1.7, 1.7, 1.7, 1.7], 14);
  T.addBullets(s, [
    "Each signal can sit on any of its four GPIOs, and you may mix them: SCK on GP18 with TX on GP3 is legal",
    "The pinout looks like five groups only because GND pins split GP0-7 on the header",
    "Source: RP2040 datasheet, GPIO function table (F1 = SPI)",
  ], MARGIN, 3.15, CONTENT_W, 1.8, 14);
  notes(s, [
    "Student question after session 14: why exactly 16-19?",
    "Point out: the RP2040 datasheet calls them RX / TX, not MISO / MOSI,",
    "because the block can be master or slave.",
  ]);

  // -------------------------------------------------------------------------
  // 6. Why 16-19
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Why We Chose GP16-19", ++n);
  T.addCards(pres, s, [
    { title: "Defaults", body: "Arduino-Pico puts SPI on GP16-19. No setRX() / setSCK() calls needed.", accent: GREEN },
    { title: "No collisions", body: "GP0-1 carry UART0 and GP4-5 carry I2C0. The GP0-7 positions are taken." },
    { title: "On the header", body: "GP23 is not on the Pico header, so the GP20-23 set is incomplete." },
    { title: "One edge", body: "GP16-21 sit together on one edge: SPI plus D/C and RST, short wires to the 5110." },
  ], 1.3, 2.1);
  notes(s, [
    "Same reasoning as the whole pin map from session 12: defaults first,",
    "then avoid collisions, then keep wiring short.",
  ]);

  // -------------------------------------------------------------------------
  // 7. Hardware CSn vs software CS
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Hardware CSn vs a GPIO as CS", ++n);
  T.addBullets(s, [
    "The SPI block (ARM PL022) has one CSn output",
    "In mode 0 it pulses CSn high between bytes - many devices do not accept that",
    "So drivers use any GPIO as CS: low for the whole transfer, one GPIO per device",
    "Our CS is GP17, the CSn position, but the library drives it as a plain GPIO",
  ], MARGIN, BODY_TOP, 5.4, 3.0, 15);
  T.addTable(s, ["Clip onto", "GPIO"], [
    ["SCK", "GP18"], ["MOSI", "GP19"], ["CS", "GP17"], ["D/C", "GP20"],
  ], 6.3, BODY_TOP + 0.1, 3.2, [1.6, 1.6], 14);
  T.addCallout(pres, s, "info", "Look for it on the capture",
    "Count CS pulses: the library opens and closes CS for every command byte and for every data block.",
    MARGIN, 4.15, CONTENT_W, 0.85);
  notes(s, [
    "PL022 with SPH = 0 (mode 0) toggles CSn between frames in continuous",
    "transfers. Software CS avoids that and allows several devices.",
  ]);

  // -------------------------------------------------------------------------
  // 8. Section: the analyzer
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 2", title: "The Logic Analyzer", note: "The instrument for the bus that stays silent" });
  n++;

  // -------------------------------------------------------------------------
  // 9. Three instruments
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Three Instruments", ++n);
  T.addTable(s,
    ["", "Multimeter", "Oscilloscope", "Logic analyzer"],
    [
      ["Shows", "one value: V, ohm, beep", "voltage over time", "0 / 1 over time"],
      ["Channels", "1", "2 - 4", "8 (ours)"],
      ["Good for", "supply, pull-ups, shorts", "edges, noise, levels", "protocols, many lines"],
      ["Blind to", "anything fast", "long sequences", "voltages, slow edges"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [1.5, 2.5, 2.5, 2.5], 13);
  T.addCallout(pres, s, "info", "The debugging thread",
    "Session 2 Serial output  ->  session 3 multimeter  ->  session 11 timing in firmware  ->  today the logic analyzer.",
    MARGIN, 3.6, CONTENT_W, 0.85);
  notes(s, [
    "A tool handed over before the need is felt is a tool nobody uses.",
    "The need: session 14, SPI wiring errors produce no error at all.",
  ]);

  // -------------------------------------------------------------------------
  // 10. Our analyzer
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Our Analyzer", ++n);
  T.addBullets(s, [
    "Cypress FX2LP clone: 8 channels, up to 24 MHz sample rate, USB 2.0",
    "PulseView names it \"Saleae Logic\": the clone reuses Saleae's USB ID. We do not use Saleae software",
    "On connect PulseView loads the open fx2lafw firmware into the chip, which then re-enumerates",
    "Inputs are for 3.3 V logic. A sample is only 0 or 1: you see levels, not voltages",
    "GND first, then the signal clips. Without a shared ground the data is random",
  ], MARGIN, BODY_TOP, CONTENT_W, 3.0, 15);
  T.addCallout(pres, s, "warn", "Clip with USB unplugged",
    "Header pins are 2.54 mm apart. One slipped clip can short 3V3 to a GPIO.",
    MARGIN, 4.15, CONTENT_W, 0.8);
  notes(s, [
    "Tested on the instructor Mac: PulseView 0.4.2 dmg from sigrok.org,",
    "Privacy & Security -> Open Anyway, driver fx2lafw, Scan. Runs under",
    "Rosetta 2 on Apple Silicon. USB-C-only Macs need an adapter.",
  ]);

  // -------------------------------------------------------------------------
  // 11. Sample rate and duration
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Sample Rate and Duration", ++n);
  T.addTable(s,
    ["Bus", "One bit", "Samples per bit at 1 MHz", "At 24 MHz", "Pick"],
    [
      ["UART 115200", "8.68 us", "8.7", "208", "2 MHz"],
      ["I2C 100 kHz", "10 us", "10", "240", "2 MHz"],
      ["SPI 4 MHz", "0.25 us", "0.25 - useless", "6", "24 MHz"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [1.9, 1.4, 2.5, 1.5, 1.7], 13);
  T.addBullets(s, [
    "Sample at least 4 times faster than the fastest signal; 10 times is comfortable",
    "Samples = rate x time. 1 M samples is 1 s at 1 MHz but only 42 ms at 24 MHz",
  ], MARGIN, 2.75, CONTENT_W, 1.0, 14);
  T.addCallout(pres, s, "warn", "24 MHz is the edge",
    "The FX2 streams every sample over USB as it goes. If a 24 MHz capture aborts, cap the demo SPI clock with SPI_CLOCK_CAP_HZ = 1 MHz and sample at 8 MHz.",
    MARGIN, 3.9, CONTENT_W, 1.05);
  notes(s, [
    "The FX2 sends one byte per sample for channels D0-D7, so disabling",
    "channels does not lower the USB load. Lower the signal rate instead.",
    "Check the maximum stable rate on both station laptops before class.",
  ]);

  // -------------------------------------------------------------------------
  // 12. PulseView workflow
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "PulseView in Six Steps", ++n);
  T.addCards(pres, s, [
    { title: "1  Connect", body: "Driver fx2lafw, Scan, pick the device", accent: ICE },
    { title: "2  Rate", body: "Sample rate and sample count from the table" },
    { title: "3  Channels", body: "Name them (D0 TX, D1 SDA...), hide the unused" },
  ], BODY_TOP + 0.1, 1.75);
  T.addCards(pres, s, [
    { title: "4  Trigger", body: "Falling edge on the line that starts the event: TX, SDA, CS" },
    { title: "5  Run, zoom", body: "Wheel zooms, drag pans, cursors measure time" },
    { title: "6  Decode", body: "Add UART / I2C / SPI, map channels, set options", accent: GREEN },
  ], BODY_TOP + 2.05, 1.75);
  notes(s, [
    "Demo this live on the projector with the instructor station before the",
    "pairs start. Save a capture as .sr and reopen it to show that files",
    "can be shared and decoded later.",
  ]);

  // -------------------------------------------------------------------------
  // 13. Probe map
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Probe Map for Today", ++n);
  T.addTable(s,
    ["Channel", "Signal", "Pico GPIO", "Pin", "Bus"],
    [
      ["D0", "UART0 TX", "GP0", "1", "UART"],
      ["D1", "SDA", "GP4", "6", "I2C"],
      ["D2", "SCL", "GP5", "7", "I2C"],
      ["D3", "SCK", "GP18", "24", "SPI"],
      ["D4", "MOSI", "GP19", "25", "SPI"],
      ["D5", "CS", "GP17", "22", "SPI"],
      ["D6", "D/C", "GP20", "26", "Nokia 5110"],
      ["D7", "RST", "GP21", "27", "Nokia 5110"],
      ["GND", "GND", "-", "3, 23 or 38", "-"],
    ],
    MARGIN, BODY_TOP, 6.2, [1.0, 1.4, 1.3, 1.2, 1.3], 12);
  text(s, "One harness, three buses. The demo uses all of them at once, so one capture can hold UART, I2C and SPI side by side.", {
    x: 7.0, y: BODY_TOP + 0.1, w: 2.5, h: 2.0, fontSize: 14,
  });
  notes(s, [
    "Wire the harness once per station and leave it on for the whole session.",
    "Same pins as the Pico pin map settled in session 12.",
  ]);

  // -------------------------------------------------------------------------
  // 14. Section: three buses
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 3", title: "Three Buses on the Wire", note: "UART, I2C and SPI from your own board" });
  n++;

  // -------------------------------------------------------------------------
  // 15. UART frame
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "UART: The 8N1 Frame, Measured", ++n);
  // 'T' = 0x54, LSB first: 0 0 1 0 1 0 1 0
  const levels = [1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1];
  const labels = ["", "Start", "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "Stop", ""];
  const cellW = 0.72;
  const wx = MARGIN + (CONTENT_W - cellW * levels.length) / 2;
  waveform(pres, s, levels, labels, wx, 1.65, cellW, 0.6, ICE);
  text(s, "idle", { x: wx, y: 1.3, w: cellW, h: 0.25, align: "center", fontSize: 11, color: GRAY });
  text(s, "'T' = 0x54 - the first byte of every demo line", {
    x: MARGIN, y: 2.75, w: CONTENT_W, h: 0.3, align: "center", fontSize: 12, color: GRAY, italic: true,
  });
  T.addBullets(s, [
    "Idle is high. Start bit low, 8 data bits LSB first, stop bit high",
    "Measure the narrowest pulse: 8.68 us -> 1 / 8.68 us = 115200 baud",
    "The decoder needs the baud rate. When you do not know it, the capture tells you",
  ], MARGIN, 3.1, CONTENT_W, 1.3, 14);
  text(s, "Capture: D0 = TX (GP0), 2 MHz, trigger on the falling edge of D0", {
    x: MARGIN, y: 4.55, w: CONTENT_W, h: 0.3, fontSize: 12, color: AMBER,
  });
  notes(s, [
    "This is the frame from the session 12 slide, now measured.",
    "Serial on USB cannot be captured - that is why the demo also prints",
    "the same line on Serial1 (GP0).",
    "Ask: why is the first falling edge a good trigger? (idle is high)",
  ]);

  // -------------------------------------------------------------------------
  // 16. I2C library capture
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "I2C: What the Library Does", ++n);
  const imgW = CONTENT_W;
  const imgH = imgW * CAPTURE_PX_H / CAPTURE_PX_W;
  const imgY = BODY_TOP;
  s.addImage({ data: "image/png;base64," + fs.readFileSync(CAPTURE_PNG).toString("base64"),
    x: MARGIN, y: imgY, w: imgW, h: imgH });
  const px = (p) => MARGIN + p / CAPTURE_PX_W * imgW;
  const labY = imgY + imgH + 0.05;
  text(s, "trigger + status", { x: px(277) - 0.8, y: labY, w: 1.6, h: 0.3, align: "center", fontSize: 11, color: AMBER });
  text(s, "status: busy, every 10 ms", { x: px(402), y: labY, w: px(773) - px(402), h: 0.3, align: "center", fontSize: 11, color: AMBER });
  text(s, "ready + 6 bytes", { x: px(898) - 0.8, y: labY, w: 1.6, h: 0.3, align: "center", fontSize: 11, color: AMBER });
  T.addCallout(pres, s, "warn", "About 50 ms inside getEvent()",
    "The library polls the busy bit every 10 ms; loop() is blocked meanwhile. Your session 13 Aht10 class does not block.",
    MARGIN, 4.25, CONTENT_W, 0.8);
  notes(s, [
    "Real capture from the instructor bench: AHT10 through Adafruit AHTX0,",
    "1 MHz sample rate, D0/D1 = SDA/SCL here.",
    "getEvent(): write AC 33 00, then read one status byte in a loop with",
    "delay(10) while bit 7 (busy) is set, then read 6 bytes.",
    "Conversion took about 50 ms here, faster than the 75 ms in the datasheet.",
    "Ask: what happens if the sensor is unplugged during the loop?",
    "(getStatus() returns 0xFF on error, busy bit set, the loop never ends.)",
  ]);

  // -------------------------------------------------------------------------
  // 17. I2C one transaction
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "I2C: Inside One Transaction", ++n);
  T.addTable(s,
    ["Piece", "On the wire", "What it tells you"],
    [
      ["START", "SDA falls while SCL is high", "a transaction begins"],
      ["Address + R/W", "0x38 + W is 0x70 on the wire", "decoder shows 0x38 in 7-bit mode"],
      ["ACK", "receiver holds SDA low on the 9th clock", "after the address: the device is there"],
      ["Data", "8 bits MSB first, each with an ACK", "AC 33 00 = trigger a measurement"],
      ["NACK", "master leaves SDA high after the last byte", "normal at the end of a read"],
      ["STOP", "SDA rises while SCL is high", "the bus is free again"],
    ],
    MARGIN, BODY_TOP, CONTENT_W, [1.8, 3.8, 3.4], 12);
  text(s, "Capture: D1 = SDA (GP4), D2 = SCL (GP5), 2 MHz, trigger on the falling edge of SDA", {
    x: MARGIN, y: 4.0, w: CONTENT_W, h: 0.3, fontSize: 12, color: AMBER,
  });
  text(s, "Zoom into the trigger write. Then into the 6-byte read: find the status byte and its busy bit.", {
    x: MARGIN, y: 4.35, w: CONTENT_W, h: 0.5, fontSize: 13,
  });
  notes(s, [
    "Note for the instructor: Arduino-Pico Wire.begin() also turns on the",
    "RP2040 internal pull-ups (tens of kilo-ohms) on SDA and SCL, in addition",
    "to any pull-ups on the module.",
    "Students decode the 6 bytes by hand once: status, 20-bit RH, 20-bit T,",
    "same shifts and masks as the session 13 lab.",
  ]);

  // -------------------------------------------------------------------------
  // 18. SPI capture
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "SPI: Commands and Data on the 5110", ++n);
  T.addBullets(s, [
    "D3 SCK, D4 MOSI, D5 CS, D6 D/C. 24 MHz, trigger on the falling edge of CS",
    "Decoder: SPI, mode 0 (CPOL 0, CPHA 0), MSB first, CS active low",
    "No MISO on the 5110: leave it unassigned",
    "The decoder does not know D/C. Read it off the trace: low = command, high = data",
  ], MARGIN, BODY_TOP, 4.2, 3.6, 14);
  T.addCode(s, [
    "D/C   bytes          meaning",
    "low   21 14 20       ext, bias 4, basic",
    "low   21 80|vop 20   ext, Vop, basic",
    "low   20 0C          basic, normal",
    "low   40 80          bank 0, X = 0",
    "high  84 bytes       bank 0 columns",
    "...                  banks 1 .. 5",
    "low   40             one more Set Y",
  ].join("\n"), 4.9, BODY_TOP, 4.6, 2.6, 11, "begin() in Adafruit PCD8544, as seen on the wire");
  notes(s, [
    "Order taken from the library source: setBias(), setContrast(), function",
    "set, display control, then display() pushes the splash image.",
    "Same commands as the course init sequence, different order.",
    "Reset the board with the capture armed to catch begin().",
  ]);

  // -------------------------------------------------------------------------
  // 19. Dirty rectangle
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The Dirty Rectangle, Visible", ++n);
  T.addTable(s,
    ["What happened", "Banks sent", "Data bytes", "Commands"],
    [
      ["clearDisplay(); display();", "6", "6 x 84 = 504", "6 x (Set Y, Set X) + 1"],
      ["one value changed; display();", "1", "changed columns only", "Set Y, Set X + 1"],
      ["nothing changed; display();", "0", "0", "1 (the final Set Y)"],
    ],
    MARGIN, BODY_TOP, CONTENT_W, [3.1, 1.4, 2.2, 2.3], 13);
  T.addBullets(s, [
    "display() sends one rectangle: the changed columns, in every bank the change touches",
    "The final Set Y has no data after it. The library source admits nobody knows why it is there",
    "The demo redraws a value only when it changes, so most seconds look like row 2 or row 3",
  ], MARGIN, 2.75, CONTENT_W, 1.4, 14);
  T.addCallout(pres, s, "info", "Try it",
    "Turn the potentiometer while T is also changing. Two lines change - how many banks, and which columns?",
    MARGIN, 4.2, CONTENT_W, 0.8);
  notes(s, [
    "Added after session 14 ran short on library time.",
    "Answer to Try it: the rectangle spans both banks and the union of the",
    "columns, even if the two values are short - it is one rectangle.",
    "Every drawPixel() grows the rectangle, even when the pixel does not",
    "change. That is why the demo compares before it redraws.",
  ]);

  // -------------------------------------------------------------------------
  // 20. Section: diagnosis
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 4", title: "Diagnosis", note: "Evidence first, conclusion second" });
  n++;

  // -------------------------------------------------------------------------
  // 21. Method
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A Method, Not a Guess", ++n);
  T.addCards(pres, s, [
    { title: "1 Symptom", body: "What does the user see? Write it down." },
    { title: "2 Hypothesis", body: "Which bus, which part of it?" },
    { title: "3 Capture", body: "Only the lines that test the hypothesis." },
    { title: "4 Compare", body: "Against a known-good capture." },
    { title: "5 Conclude", body: "Point at the samples that prove it.", accent: GREEN },
  ], BODY_TOP + 0.15, 2.3);
  T.addCallout(pres, s, "info", "Known-good first",
    "Capture the working demo before you touch a broken board. Without a reference every trace looks suspicious.",
    MARGIN, 3.95, CONTENT_W, 0.9);
  notes(s, [
    "Same method as the multimeter checks in session 3, one level up.",
    "Everyone saves the known-good captures from part 3 as .sr files.",
  ]);

  // -------------------------------------------------------------------------
  // 22. What good looks like
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "What Good Looks Like", ++n);
  T.addTable(s,
    ["Bus", "Check", "Known-good value"],
    [
      ["UART", "level when idle", "high"],
      ["UART", "width of one bit", "8.68 us (115200 baud)"],
      ["I2C", "address", "0x38"],
      ["I2C", "9th clock after the address", "ACK: SDA low"],
      ["SPI", "SCK when idle", "low (mode 0)"],
      ["SPI", "MOSI changes on", "falling SCK; sampled on rising"],
      ["SPI", "CS", "low around each byte or block"],
      ["SPI", "first bytes after reset", "21 14 20 21 ..."],
    ],
    MARGIN, BODY_TOP, CONTENT_W, [1.4, 3.6, 4.0], 12);
  notes(s, [
    "Students fill this in from their own known-good captures before the",
    "lab, then check the values against the table.",
  ]);

  // -------------------------------------------------------------------------
  // 23. Mystery firmware lab
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Lab: Mystery Firmware", ++n);
  T.addBullets(s, [
    "Four files in lab/: mystery-A.uf2 to mystery-D.uf2. Same wiring as the demo",
    "Each comes with a bug report from a user (lab/README.md)",
    "Not every file is broken. The firmware will not tell you which",
    "Pairs, two analyzer stations, about 8 minutes per file",
    "For each file: symptom, hypothesis, capture settings, evidence, conclusion",
  ], MARGIN, BODY_TOP, CONTENT_W, 2.9, 15);
  T.addCallout(pres, s, "error", "Not accepted as proof",
    "A guess from the symptom, a hex editor, or \"it works now\". The proof is a capture you can point at.",
    MARGIN, 3.75, CONTENT_W, 0.85);
  notes(s, [
    "45 min. Answer key is in embedded-cpp-materials, solution/ only.",
    "Flash: hold BOOTSEL, plug in, copy the .uf2 onto RPI-RP2.",
    "After the lab, flash the demo again: pio run -t upload in demo/.",
    "Debrief: each pair presents one file with the screenshot as evidence.",
  ]);

  // -------------------------------------------------------------------------
  // 24. Summary
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Summary", ++n);
  T.addBullets(s, [
    "SPI0 on GP16-19: library defaults, no collisions, one edge. CS is a plain GPIO",
    "Sample 4 - 10 times faster than the bit rate. Samples = rate x time. Trigger on the event",
    "UART: measure one bit. I2C: look for the ACK. SPI: mode, CS and D/C",
    "Libraries hide polling, blocking and extra commands. The analyzer shows them",
    "K4 is done: timers, UART, I2C, SPI, and a tool to see all of them",
  ], MARGIN, BODY_TOP, CONTENT_W, 2.9, 14);
  T.addCallout(pres, s, "warn", "Homework: room thermostat",
    "Start from the demo. Pot = setpoint, AHT10 = room, RGB LED = HEAT / OK / COOL with a +-0.5 C dead band, all on the 5110 and the UART line. Spec: homework/README.md",
    MARGIN, 4.0, CONTENT_W, 1.0);
  notes(s, [
    "The homework closes K4: three buses plus the RGB LED back on the Pico.",
    "RGB on GP10 / 11 / 12, common anode to 3V3 OUT, as in the pin map.",
    "Next: optional blocks from session 16.",
  ]);

  await T.save(pres, "lesson15.pptx");
  console.log("OK: lesson15.pptx, " + n + " slides");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
