// Lesson 22 - Z1: Cross-Platform Comparison
// Build: node lesson22-slides.js  ->  lesson22.pptx

const T = require("./slides-theme");
const { FaMicrochip } = require("react-icons/fa");

const { MARGIN, CONTENT_W, BODY_TOP } = T;

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "speaker notes"));
}

// Simple text box helper for diagrams
function label(slide, text, x, y, w, h, o) {
  const opt = Object.assign({
    x, y, w, h, fontSize: 12, fontFace: T.BODY, color: T.ICE, margin: 0,
    align: "center", valign: "middle",
  }, o || {});
  slide.addText(T.assertAscii(text, "label"), opt);
}

function box(pres, slide, x, y, w, h, fill, line) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h, fill: { color: fill || T.CARD_BG }, line: { color: line || T.MID, width: 1 },
  });
}

function hline(pres, slide, x, y, w, color, t) {
  slide.addShape(pres.shapes.RECTANGLE, { x, y, w, h: t || 0.03, fill: { color: color }, line: { color: color, width: 0 } });
}
function vline(pres, slide, x, y, h, color, t) {
  slide.addShape(pres.shapes.RECTANGLE, { x, y, w: t || 0.03, h, fill: { color: color }, line: { color: color, width: 0 } });
}

// One WS2812 bit cell: high for `highFrac` of the cell, low for the rest.
function bitCell(pres, slide, x, yHigh, yLow, w, highFrac, color) {
  const hw = w * highFrac;
  const t = 0.035;
  vline(pres, slide, x, yHigh, yLow - yHigh + t, color, t);
  hline(pres, slide, x, yHigh, hw, color, t);
  vline(pres, slide, x + hw, yHigh, yLow - yHigh + t, color, t);
  hline(pres, slide, x + hw, yLow, w - hw, color, t);
}

(async () => {
  const pres = T.newDeck(
    "Lesson 22 - One Picture, Three Chips",
    "Lesson 22  |  Z1: Cross-Platform Comparison"
  );
  const chip = await T.icon(FaMicrochip, "#CADCFC");
  let n = 1;
  let s;

  // 1 ------------------------------------------------------------------
  s = T.titleSlide(pres, {
    tag: "LESSON 22",
    title: "One Picture, Three Chips",
    subtitle: "Cross-platform comparison and portability",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3  |  RP2040  |  RP2350  |  STM32F401",
    iconData: chip,
  });
  notes(s, "Session 22, Mon 2026-11-16. 180 min.\n" +
    "0-5 intro. 5-30 part 1 (where we have been, APIs, layers). 30-45 WS2812B and the code. " +
    "45-75 stations lab. 75-85 results and STM32. 85-95 break. 95-175 final projects. 175-180 homework.");

  // 2 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Today", ++n);
  T.addCards(pres, s, [
    { title: "Part 1  (30 min)", body: "Four chips you have used.\nSame task, different API.\nWhat carries over, what is HAL.", accent: T.ICE },
    { title: "Part 2  (55 min)", body: "WS2812B 8x8 panels.\nOne source, three boards.\nStations lab and the numbers.", accent: T.AMBER },
    { title: "Part 3  (80 min)", body: "Final projects.\nEach idea checked and approved.\nPlan for sessions 23-25.", accent: T.GREEN },
  ], 1.35, 2.7);
  T.addCallout(pres, s, "info", "The question for today",
    "If you move your code to another chip tomorrow - which lines change, and which stay?",
    MARGIN, 4.2, CONTENT_W, 0.8);
  notes(s, "Break between part 2 and part 3, around minute 85.");

  // 3 ------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 1", title: "Where We Have Been", note: "Four chips, three frameworks, one language" });
  n++;

  // 4 ------------------------------------------------------------------
  s = T.contentSlide(pres, "The Chips You Have Used", ++n);
  T.addTable(s,
    ["Chip", "CPU", "Clock", "RAM", "FPU", "Radio", "How we used it"],
    [
      ["ESP32-C3", "1x RISC-V", "160 MHz", "400 KB", "no", "WiFi, BLE", "Arduino-ESP32 core 2.x"],
      ["RP2040 (Pico)", "2x Cortex-M0+", "133 MHz", "264 KB", "no", "-", "Arduino-Pico"],
      ["RP2350 (Pico 2)", "2x Cortex-M33", "150 MHz", "520 KB", "yes", "-", "Arduino-Pico"],
      ["STM32F401 (Black Pill)", "1x Cortex-M4F", "84 MHz", "64 KB", "yes", "-", "CubeMX + HAL"],
    ],
    MARGIN, BODY_TOP + 0.15, CONTENT_W, [2.05, 1.45, 0.9, 0.85, 0.6, 1.0, 2.15], 11);
  T.addBullets(s, [
    "Three instruction sets: RISC-V, ARMv6-M, ARMv7E-M / ARMv8-M. The compiler hides this from you",
    "Flash: external SPI chip on ESP32-C3 and RP2040 / RP2350, inside the chip on the STM32",
    "No FPU on the C3 and the RP2040: float math works, but in software",
  ], MARGIN, 3.25, CONTENT_W, 1.6, 13);
  notes(s, "RP2350 can also boot its two Hazard3 RISC-V cores instead of the M33s - mentioned in session 1. " +
    "Ask: which of these did you find easiest to start with, and why? Usually the answer is about the toolchain, not the chip.");

  // 5 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Same Task, Different API", ++n);
  T.addTable(s,
    ["Task", "ESP32-C3 (Arduino)", "Pico (Arduino)", "STM32 (HAL)"],
    [
      ["Write a pin", "digitalWrite(pin, HIGH)", "digitalWrite(pin, HIGH)", "HAL_GPIO_WritePin(port, pin, SET)"],
      ["PWM", "ledcSetup + ledcAttachPin + ledcWrite", "analogWrite(pin, duty)", "timer channel + __HAL_TIM_SET_COMPARE"],
      ["Read the ADC", "analogRead -> 12 bits", "analogRead -> 10 bits", "HAL_ADC_Start + PollForConversion"],
      ["I2C", "Wire, SDA 8 / SCL 9", "Wire, SDA GP4 / SCL GP5", "HAL_I2C_Master_Transmit / Receive"],
      ["Timer interrupt", "timerBegin + IRAM_ATTR ISR", "repeating timer, no IRAM_ATTR", "HAL_TIM_PeriodElapsedCallback"],
      ["Critical section", "portENTER_CRITICAL", "noInterrupts()", "__disable_irq()"],
    ],
    MARGIN, BODY_TOP + 0.1, CONTENT_W, [1.4, 2.6, 2.2, 2.8], 10);
  T.addCallout(pres, s, "info", "Arduino is a HAL too",
    "On the C3 and the Pico the first row is identical. Arduino hides the chip - until PWM, timers and interrupts.",
    MARGIN, 4.05, CONTENT_W, 0.85);
  notes(s, "Every line here is something the group wrote or saw in sessions 7-17. " +
    "Point at the rows where the Arduino columns differ: those are exactly the places their code broke when moving from C3 to Pico in session 12.");

  // 6 ------------------------------------------------------------------
  s = T.contentSlide(pres, "What Carries Over, What Does Not", ++n);
  T.addCards(pres, s, [
    { title: "Carries over", accent: T.GREEN,
      body: "C and C++ itself\nYour classes: RingBuffer, Button, state machines\nProtocols: the AHT10 bytes are the same on any chip\nThe logic of your program" },
    { title: "Changes", accent: T.AMBER,
      body: "Pin numbers and pin functions\nPeripheral setup and API names\nADC resolution, voltage levels\nISR rules, upload method, drivers" },
    { title: "Traps you met", accent: T.RED,
      body: "core 3.x examples on core 2.x\nAHT20 code for an AHT10\nF411 project on an F401 board\n10-bit vs 12-bit analogRead" },
  ], 1.3, 3.55);
  notes(s, "Ask the group to add to the third column from memory before revealing it. " +
    "Every item in 'Traps' cost us real class time.");

  // 7 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Layers: Where the Chip Shows Through", ++n);
  const layers = [
    ["Application", "thermostat logic, animations", T.GREEN],
    ["Your drivers", "class Aht10, class RgbLed, xy()", T.ICE],
    ["Framework / HAL", "Arduino core, STM32 HAL", T.AMBER],
    ["Registers", "one chip, one reference manual", T.RED],
  ];
  layers.forEach((L, i) => {
    const y = 1.3 + i * 0.85;
    box(pres, s, MARGIN, y, 4.0, 0.7, T.CARD_BG, L[2]);
    label(s, L[0], MARGIN + 0.15, y + 0.05, 3.7, 0.32, { fontSize: 15, bold: true, color: T.WHITE, fontFace: T.HEADING, align: "left" });
    label(s, L[1], MARGIN + 0.15, y + 0.36, 3.7, 0.28, { fontSize: 11, align: "left" });
  });
  label(s, "digitalWrite(pin, HIGH) at the bottom:", 4.9, 1.3, 4.6, 0.35, { fontSize: 14, bold: true, color: T.WHITE, align: "left" });
  T.addTable(s,
    ["Chip", "Register written"],
    [
      ["ESP32-C3", "GPIO_OUT_W1TS_REG"],
      ["RP2040 / RP2350", "SIO GPIO_OUT_SET"],
      ["STM32", "GPIOx->BSRR"],
    ],
    4.9, 1.75, 4.6, [1.9, 2.7], 12);
  T.addBullets(s, [
    "Different names, same idea: write a 1 to set a pin",
    "No read-modify-write, so an ISR cannot corrupt it",
  ], 4.9, 3.35, 4.6, 1.2, 12);
  notes(s, "Portable code keeps the chip in the bottom two layers. The top two should compile unchanged. " +
    "W1TS = write one to set. Same pattern on all three families - nobody copied anybody, it is just the right design.");

  // 8 ------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 2", title: "One Picture, Three Chips", note: "WS2812B 8x8 panels on ESP32-C3, Pico and Pico 2" });
  n++;

  // 9 ------------------------------------------------------------------
  s = T.contentSlide(pres, "WS2812B: One Wire, 64 Pixels", ++n);
  // chain diagram
  const cx0 = MARGIN, cy = 1.35;
  box(pres, s, cx0, cy, 0.8, 0.55, null, T.ICE);
  label(s, "MCU", cx0, cy, 0.8, 0.55, { bold: true, color: T.WHITE });
  const px = ["px 0", "px 1", "px 2", "...", "px 63"];
  px.forEach((p, i) => {
    const x = 1.75 + i * 1.5;
    if (p !== "...") {
      box(pres, s, x, cy, 0.9, 0.55, T.CARD_BG, T.AMBER);
    }
    label(s, p, x, cy, 0.9, 0.55, { bold: true, color: T.WHITE });
    hline(pres, s, x - (i === 0 ? 0.45 : 0.6), cy + 0.27, i === 0 ? 0.45 : 0.6, T.ICE);
  });
  label(s, "DIN -> each pixel keeps the first 24 bits, sends the rest on through DOUT", MARGIN, 2.0, CONTENT_W, 0.3, { fontSize: 12, color: T.GRAY });
  T.addBullets(s, [
    "24 bits per pixel, in the order G, R, B, most significant bit first",
    "One bit = 1.25 us (800 kHz). No clock wire: the pulse width is the data",
    "A pause in the signal ends the frame: every pixel shows its new colour at once",
    "One frame of 64 pixels: 64 x 24 x 1.25 us = 1.92 ms",
  ], MARGIN, 2.45, CONTENT_W, 2.0, 14);
  notes(s, "Compare with session 12-14: UART has no clock wire either, but uses a fixed bit rate and start / stop bits. " +
    "Here the timing of each pulse carries the bit. That is why it needs hardware help.");

  // 10 -----------------------------------------------------------------
  s = T.contentSlide(pres, "A Bit Is a Pulse Width", ++n);
  const yH = 1.75, yL = 2.75, cw = 2.6;
  bitCell(pres, s, 1.0, yH, yL, cw, 0.32, T.ICE);
  bitCell(pres, s, 1.0 + cw, yH, yL, cw, 0.64, T.AMBER);
  bitCell(pres, s, 1.0 + 2 * cw, yH, yL, cw, 0.32, T.ICE);
  label(s, "0", 1.0, 1.3, cw, 0.35, { fontSize: 18, bold: true, color: T.ICE });
  label(s, "1", 1.0 + cw, 1.3, cw, 0.35, { fontSize: 18, bold: true, color: T.AMBER });
  label(s, "0", 1.0 + 2 * cw, 1.3, cw, 0.35, { fontSize: 18, bold: true, color: T.ICE });
  label(s, "high 0.4 us", 1.0, 2.9, cw * 0.6, 0.3, { fontSize: 11 });
  label(s, "high 0.8 us", 1.0 + cw, 2.9, cw * 0.8, 0.3, { fontSize: 11 });
  label(s, "<----------- 1.25 us ----------->", 1.0 + 2 * cw, 2.9, cw, 0.3, { fontSize: 11, color: T.GRAY });
  T.addBullets(s, [
    "Tolerance is about +-150 ns. Too slow for loop() and digitalWrite()",
    "So every chip uses a hardware block to generate it, and the CPU only fills a buffer",
    "You will see these pulses in PulseView at the analyzer station",
  ], MARGIN, 3.4, CONTENT_W, 1.5, 14);
  notes(s, "digitalWrite on the C3 takes on the order of a hundred ns and an interrupt in the middle of a frame would ruin the timing. " +
    "Old AVR libraries did it in hand-tuned assembly with interrupts off. Our chips have better options - next slide.");

  // 11 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Who Makes the Waveform", ++n);
  T.addCards(pres, s, [
    { title: "ESP32-C3: RMT", accent: T.GREEN,
      body: "Remote Control peripheral, made for IR remotes.\nThe library turns every bit into a (level, duration) item, RMT plays them out." },
    { title: "RP2040 / RP2350: PIO", accent: T.AMBER,
      body: "Programmable I/O: tiny state machines with their own 4-instruction program.\nThe CPU pushes 32-bit words into a FIFO." },
    { title: "STM32: timer + DMA", accent: T.ICE,
      body: "A PWM timer at 800 kHz. DMA writes a new compare value for every bit.\nOn the slide only - next part." },
  ], 1.3, 2.75);
  T.addCode(s, "strip.setPixelColor(xy(x, y), colour);\nstrip.show();   // same two lines on every board",
    MARGIN, 4.1, CONTENT_W, 0.9, 11);
  notes(s, "Same library, Adafruit NeoPixel. Inside it there is an #if per architecture - open the library source on screen if time allows (esp.c, rp2040 part).");

  // 12 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Wiring the Panel", ++n);
  T.addTable(s,
    ["Panel wire", "Goes to"],
    [
      ["red / black pair (5V, GND)", "4.25 V supply + / -"],
      ["WHITE (input connector)", "GND: supply - AND board GND"],
      ["green (input connector, DIN)", "C3 GPIO 10 / Pico GP22 / Pico 2 GP0"],
      ["DOUT group (other end)", "leave free"],
    ],
    MARGIN, BODY_TOP + 0.1, 5.3, [2.4, 2.9], 11);
  T.addBullets(s, [
    "DIN threshold: 0.7 x VDD",
    "At 5 V: 3.5 V - 3.3 V logic is not enough",
    "At 4.25 V: 3.0 V - 3.3 V works",
    "Brightness 20 of 255: about 0.3 A. Full white: almost 4 A",
  ], 6.05, BODY_TOP + 0.1, 3.45, 2.6, 12);
  T.addCallout(pres, s, "error", "WHITE is ground on this panel",
    "Not data. Power off while wiring. Supply first, then the data wire. Never the board 5V pin for the panel.",
    MARGIN, 3.95, CONTENT_W, 0.95);
  notes(s, "Same 5 V vs 3.3 V question as the TJA1050 RXD in session 21, other direction: there the 5 V side drove our pin, here our 3.3 V drives a 5 V input. " +
    "Lowering the supply lowers the threshold. 75 ohm series resistor is already on the panel.");

  // 13 -----------------------------------------------------------------
  s = T.contentSlide(pres, "One Source, Three Boards", ++n);
  T.addCode(s, [
    "[env:c3]",
    "platform = platformio/espressif32",
    "board = lolin_c3_mini",
    "build_flags = -std=gnu++17 -D LED_DATA_PIN=10",
    "",
    "[env:pico]",
    "board = pico",
    "build_flags = -D LED_DATA_PIN=22",
    "",
    "[env:pico2]",
    "board = rpipico2",
    "build_flags = -D LED_DATA_PIN=0",
  ].join("\n"), MARGIN, BODY_TOP + 0.05, 4.7, 3.6, 10.5, "platformio.ini (shortened)");
  T.addCode(s, [
    "#ifndef LED_DATA_PIN",
    "#error \"LED_DATA_PIN not set\"",
    "#endif",
    "",
    "Adafruit_NeoPixel strip(",
    "    kNumPixels, LED_DATA_PIN,",
    "    NEO_GRB + NEO_KHZ800);",
  ].join("\n"), 5.4, BODY_TOP + 0.05, 4.1, 2.35, 11, "main.cpp - no board name anywhere");
  T.addBullets(s, [
    "pio run -e c3 -t upload",
    "pio run -e pico2 -t upload",
  ], 5.4, 3.75, 4.1, 0.9, 13);
  notes(s, "The board is chosen by the env, not by editing the code. #error turns a forgotten flag into a build error instead of a silent wrong pin.");

  // 14 -----------------------------------------------------------------
  s = T.contentSlide(pres, "xy(): The Panel Wiring in One Function", ++n);
  T.addCode(s, [
    "// (0, 0) = top-left as the viewer sees it",
    "static uint16_t xy(uint8_t x, uint8_t y) {",
    "    const bool oddRow = (y & 1U) != 0U;",
    "    bool reversed = kRow0RightToLeft;",
    "    if (kSerpentine && oddRow) {",
    "        reversed = !reversed;",
    "    }",
    "    const uint8_t col = reversed",
    "        ? static_cast<uint8_t>(kWidth - 1U - x) : x;",
    "    return static_cast<uint16_t>(y * kWidth + col);",
    "}",
  ].join("\n"), MARGIN, BODY_TOP + 0.05, 5.3, 3.2, 10);
  // serpentine mini grid, 3 rows x 8
  const gx = 6.2, gy = 1.35, cs = 0.4;
  for (let r = 0; r < 3; r++) {
    for (let c = 0; c < 8; c++) {
      const reversed = (r % 2 === 0);
      const idx = r * 8 + (reversed ? 7 - c : c);
      const first = idx === 0;
      box(pres, s, gx + c * cs, gy + r * (cs + 0.08), cs - 0.05, cs - 0.05, first ? T.AMBER : T.CARD_BG, T.MID);
      label(s, String(idx), gx + c * cs, gy + r * (cs + 0.08), cs - 0.05, cs - 0.05, { fontSize: 9, color: first ? T.DARK_BG : T.ICE, bold: first });
    }
  }
  label(s, "DIN enters at the top right", gx, gy + 1.5, 3.2, 0.3, { fontSize: 11, color: T.GRAY });
  label(s, "row 0 <-, row 1 ->, row 2 <- ...", gx, gy + 1.8, 3.2, 0.3, { fontSize: 11, color: T.GRAY });
  T.addCallout(pres, s, "warn", "Wrong xy(): rainbow mush",
    "Same colours, wrong places. The rest of the code never knows how the panel is soldered.",
    6.0, 3.6, 3.5, 1.1);
  notes(s, "True story from the bench test: with a wrong xy() the diagonal rainbow turned into colour mush. " +
    "This is the same idea as LED_DATA_PIN: everything that depends on the hardware sits in one place.");

  // 15 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Lab: Stations", ++n);
  T.addTable(s,
    ["Station", "Board", "Data pin", "env"],
    [
      ["A", "ESP32-C3 Super Mini", "GPIO 10", "c3"],
      ["B", "Raspberry Pi Pico (RP2040)", "GP22", "pico"],
      ["C", "Raspberry Pi Pico 2 (RP2350)", "GP0", "pico2"],
      ["D", "Logic analyzer on DIN", "D0", "PulseView"],
    ],
    MARGIN, BODY_TOP + 0.05, 4.6, [0.75, 2.05, 0.9, 0.9], 10);
  T.addBullets(s, [
    "1. Wire with power off, supply first",
    "2. Build and upload the demo, watch ORDER and WALK",
    "3. Draw your own 8x8 picture with xy(): initials, a face, an arrow",
    "4. Move your code to the next station: only the env changes",
    "5. Write down show() from Serial",
  ], 5.35, BODY_TOP + 0.05, 4.15, 2.9, 12);
  T.addCallout(pres, s, "info", "Station D",
    "24 MHz sampling, D0 on DIN, decoder rgb_led_ws281x. Capture C3 and Pico 2: measure the 0 and 1 pulses, look for gaps between pixels.",
    MARGIN, 3.95, CONTENT_W, 0.95);
  notes(s, "30 minutes. Four students, three panels: one student at the analyzer, rotate after 15 minutes. " +
    "Step 4 is the point of the session - same main.cpp, different board, same picture.");

  // 16 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Results: Time Inside show()", ++n);
  T.addTable(s,
    ["Board", "Hardware", "show() measured", "vs 1920 us on the wire"],
    [
      ["ESP32-C3", "RMT", "2072 us", "+152 us, waits for the end"],
      ["RP2040 (Pico)", "PIO", "1834 us", "-86 us, returns early"],
      ["RP2350 (Pico 2)", "PIO", "1834 us", "-86 us, returns early"],
    ],
    MARGIN, BODY_TOP + 0.1, CONTENT_W, [2.0, 1.4, 2.2, 3.4], 12);
  T.addBullets(s, [
    "C3: the CPU waits until the last bit is out, plus time to convert the buffer into RMT items",
    "RP2040 and RP2350: show() returns while the last pixels are still in the PIO FIFO",
    "M0+ at 133 MHz and M33 at 150 MHz give the same 1834 us: PIO sets the time, not the CPU",
    "Same API, same picture, different behaviour. Portable code is not the same as identical timing",
  ], MARGIN, 2.75, CONTENT_W, 2.2, 13);
  notes(s, "Bench numbers from the instructor setup, 2026-11-16, brightness 20, 50 frames per second. " +
    "All three rows measured on the instructor bench. 86 us is about 3 pixels of 30 us each - roughly the depth of the PIO TX FIFO.");

  // 17 -----------------------------------------------------------------
  s = T.contentSlide(pres, "The Black Pill Way: Timer + DMA with HAL", ++n);
  T.addCode(s, [
    "// TIM2 at 84 MHz: one bit = 1.25 us = 105 ticks",
    "#define WS_ARR   104U   // period - 1",
    "#define WS_ZERO   34U   // 0.4 us high",
    "#define WS_ONE    67U   // 0.8 us high",
    "",
    "static uint16_t buf[64U * 24U + 50U];  // + reset",
    "",
    "for (uint16_t i = 0U; i < 64U * 24U; i++) {",
    "    buf[i] = bit(i) ? WS_ONE : WS_ZERO;",
    "}",
    "HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1,",
    "                      (uint32_t *)buf, sizeof(buf) / 2U);",
  ].join("\n"), MARGIN, BODY_TOP + 0.05, 5.6, 3.65, 10.5, "Sketch, not tested in class");
  T.addBullets(s, [
    "One 16-bit compare value per bit: 3 KB of RAM for 64 pixels",
    "The DMA reloads the timer after every period, the CPU is free",
    "This is what the Arduino library does for you on the other chips",
  ], 6.35, BODY_TOP + 0.1, 3.15, 3.5, 12);
  notes(s, "Shown, not run: the STM32 block ended with session 17. The numbers come from the clock: 84 MHz / 800 kHz = 105. " +
    "bit(i) stands for 'bit i of the GRB stream'. Compare the memory cost with RMT items on the C3.");

  // 18 -----------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 3", title: "Final Projects", note: "Approve, check the hardware, plan sessions 23-25" });
  n++;

  // 19 -----------------------------------------------------------------
  s = T.contentSlide(pres, "What We Check in Each Proposal", ++n);
  T.addBullets(s, [
    "Goal in one sentence: what does the device do for whom",
    "Hardware: what you need, and do we already have it",
    "Pin plan: ESP32-C3 has 13 usable GPIO, 2 / 8 / 9 are strapping pins",
    "Platform: which board, and one reason from today",
    "Minimum version: what must work on demo day",
    "Risk: the one part you have never done before, and a fallback",
  ], MARGIN, BODY_TOP, 5.6, 3.6, 14);
  T.addTable(s,
    ["Session", "Date", "Goal"],
    [
      ["23", "Fri 2026-11-20", "skeleton builds, hardware wired"],
      ["24", "Mon 2026-11-23", "minimum version works"],
      ["25", "Fri 2026-11-27", "demo day"],
    ],
    6.25, BODY_TOP + 0.1, 3.25, [0.75, 1.15, 1.35], 10);
  notes(s, "About 20 minutes per student. Proposal files: students/<username>/final-project.md. " +
    "Settle here: individual or paired (open decision 3 in course-program.md).");

  // 20 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Choosing a Platform for Your Project", ++n);
  T.addTable(s,
    ["You need", "Look at", "Why"],
    [
      ["WiFi, a web page, MQTT, ESP-NOW", "ESP32-C3", "radio on chip, sessions 18-20"],
      ["many precise signals, LEDs, odd protocols", "Pico / Pico 2", "PIO, two cores, more pins"],
      ["float math in a fast loop", "Pico 2 / STM32", "hardware FPU"],
      ["battery, sleeping most of the time", "ESP32-C3", "deep sleep 5 uA, session 19"],
      ["not sure", "ESP32-C3", "reference platform, most code to reuse"],
    ],
    MARGIN, BODY_TOP + 0.1, CONTENT_W, [3.6, 1.8, 3.6], 12);
  T.addCallout(pres, s, "ok", "Rule of thumb",
    "Pick the board you already know unless the project needs something it does not have.",
    MARGIN, 3.95, CONTENT_W, 0.85);
  notes(s, "Two boards in one project (for example C3 for WiFi and Pico for LEDs over UART) is allowed but doubles the risk - only with a fallback.");

  // 21 -----------------------------------------------------------------
  T.summarySlide(pres, ++n, [
    "Arduino and HAL hide the chip; pins, peripherals and timing are where it shows through",
    "Keep the hardware in one place: LED_DATA_PIN in platformio.ini, the panel layout in xy()",
    "The same API can behave differently: measure on every platform",
    "Final projects approved - sessions 23 and 24 are for building",
  ], "Start your project: students/<username>/final-project/ as a PlatformIO project that builds for your board, plus a README with the pin table. Commit before session 23 (Fri 2026-11-20).");

  await T.save(pres, "lesson22.pptx");
  console.log("OK: lesson22.pptx, slides: " + n);
})().catch((e) => { console.error("BUILD FAILED:", e.message); process.exit(1); });
