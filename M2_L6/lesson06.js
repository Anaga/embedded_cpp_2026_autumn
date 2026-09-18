/*
 * lesson06.js - Lesson 06: Functions, Scope and Bits
 *
 * Build: node lesson06.js
 * QA:    soffice --headless --convert-to pdf lesson06.pptx --outdir /tmp/qa
 *        pdftoppm -png -r 70 /tmp/qa/lesson06.pdf /tmp/qa/slide
 */

const T = require("./slides-theme");

(async () => {
  const { FaCode } = require("react-icons/fa");

  const pres = T.newDeck(
    "Lesson 06 - Functions, Scope and Bits",
    "Lesson 06  |  Functions, Scope and Bits"
  );

  const iconCode = await T.icon(FaCode, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 06",
    title: "Functions, Scope and Bits",
    subtitle: "Naming your own work, and reading a number bit by bit",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Session 6 of 25",
    iconData: iconCode,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addBullets(s, [
    "Writing your own functions instead of only calling ours",
    "Where a variable is visible, and how long it stays alive",
    "The stack: what happens on every call and every return",
    "Bits, masks and shifts",
    "Last hour: an RGB LED that shows the temperature as a colour",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.8);
  T.addCallout(pres, s, "info", "Goal",
    "By the end of the lesson you can write a function, explain why a local variable " +
    "forgets, and turn a number into a pattern of lights.");

  // -------------------------------------------------------------------------
  // 3. Section 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "Functions",
    note: "A name for a piece of work",
  });

  // -------------------------------------------------------------------------
  // 4. You have already used them
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "You have already used functions", 4);
  T.addCode(s, [
    "uint16_t counts = readPotentiometer();",
    "int16_t f = mapToFahrenheit(counts);",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.2, 1.1, 12);
  T.addBullets(s, [
    "Someone wrote them, you called them",
    "A name, some input, one answer back",
    "You did not need to know what was inside",
    "Today you write your own",
  ], T.MARGIN, 2.6, T.CONTENT_W, 2.0);

  // -------------------------------------------------------------------------
  // 5. Anatomy
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The four parts of a function", 5);
  T.addCode(s, [
    "int16_t doubleIt(int16_t value) {",
    "    return value * 2;",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.6, 1.4, 13);
  T.addBullets(s, [
    "int16_t - what comes back",
    "doubleIt - the name you call",
    "(int16_t value) - what goes in",
    "return - the answer leaving",
  ], 6.3, T.BODY_TOP, 3.2, 2.2, 14);
  T.addCallout(pres, s, "info", "One job each",
    "A function that does one thing has a name that says what it does. " +
    "If the name needs the word and in it, it is two functions.",
    T.MARGIN, 3.2, T.CONTENT_W, 0.9);

  // -------------------------------------------------------------------------
  // 6. Prototypes
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The compiler reads top to bottom", 6);
  T.addCode(s, [
    "void printBinary(uint8_t value);   // prototype",
    "",
    "void loop(void) {",
    "    printBinary(42U);              // call",
    "}",
    "",
    "void printBinary(uint8_t value) {  // the real thing",
    "    // ...",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.4, 3.15, 12);
  T.addBullets(s, [
    "Call before the body?",
    "Then announce it first",
    "That line is the prototype",
  ], 7.0, T.BODY_TOP + 0.3, 2.5, 2.2, 13);

  // -------------------------------------------------------------------------
  // 7. Section 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "Scope and the Stack",
    note: "Where a variable lives and when it disappears",
  });

  // -------------------------------------------------------------------------
  // 8. Where a variable lives
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Where a variable is visible", 8);
  T.addCode(s, [
    "static uint8_t g_total = 0U;    // the whole file",
    "",
    "void demo(void) {",
    "    uint8_t local = 0U;         // this function only",
    "    local = local + 1U;",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.6, 2.3, 12);
  T.addBullets(s, [
    "The braces are the boundary",
    "Outside them the name does not exist",
    "Two functions can each have a local called value; they are different variables",
  ], T.MARGIN, 3.7, T.CONTENT_W, 1.3, 14);

  // -------------------------------------------------------------------------
  // 9. The stack
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "What a call actually does", 9);
  T.addCards(pres, s, [
    { title: "Call", body: "A frame is put on the stack: the parameters and the locals." },
    { title: "Work", body: "The function uses its own frame and nobody else's." },
    { title: "Return", body: "The frame is dropped. The locals go with it." },
  ], 1.4, 2.3);
  T.addCallout(pres, s, "warn", "The stack is small",
    "A few kilobytes, not megabytes. Large local arrays and deep chains of calls " +
    "are how a board runs out of it.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.9);

  // -------------------------------------------------------------------------
  // 10. static
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The variable that remembers", 10);
  T.addCode(s, [
    "void countCalls(void) {",
    "    static uint8_t calls = 0U;",
    "    calls = calls + 1U;",
    "    Serial.println(calls);",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.6, 2.0, 12);
  T.addBullets(s, [
    "Created once, not on every call",
    "Prints 1, 2, 3, 4 ...",
    "Without static: 1, 1, 1, 1",
  ], 6.3, T.BODY_TOP + 0.1, 3.2, 2.0, 14);
  T.addCallout(pres, s, "ok", "Keep this one in mind",
    "Tonight's star task needs a value that survives from one call to the next.",
    T.MARGIN, 3.6, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 11. Section 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "Bits",
    note: "Looking inside a byte",
  });

  // -------------------------------------------------------------------------
  // 12. A number is eight bits
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "One byte, eight bits", 12);
  T.addTable(s,
    ["Value", "Bits", "Which bits are set"],
    [
      ["0", "00000000", "none"],
      ["1", "00000001", "bit 0"],
      ["5", "00000101", "bit 0 and bit 2"],
      ["128", "10000000", "bit 7"],
      ["255", "11111111", "all of them"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.0, 3.0, 4.0]);
  T.addCallout(pres, s, "info", "Counting from the right",
    "Bit 0 is worth 1, bit 1 is worth 2, bit 2 is worth 4, and so on up to 128.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 13. printBinary
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Your first function from scratch", 13);
  T.addCode(s, [
    "void printBinary(uint8_t value) {",
    "    for (int8_t i = 7; i >= 0; i--) {",
    "        Serial.print((value >> i) & 1U);",
    "    }",
    "    Serial.println();",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.0, 2.3, 12);
  T.addBullets(s, [
    "Shift the bit you want down to the bottom",
    "Mask away everything else",
    "Bit 7 first, bit 0 last",
  ], T.MARGIN, 3.7, T.CONTENT_W, 1.3, 14);

  // -------------------------------------------------------------------------
  // 14. Masks and shifts
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Four operations, that is all", 14);
  T.addCode(s, [
    "value & 1U     // is bit 0 set?",
    "value | 4U     // switch bit 2 on",
    "value >> 2     // move the bits right",
    "value << 2     // move the bits left",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.8, 1.8, 12);
  T.addBullets(s, [
    "& asks a question",
    "| sets something",
    "Shifts move, they do not add",
  ], 6.5, T.BODY_TOP, 3.0, 1.8, 14);
  T.addCallout(pres, s, "error", "One character apart",
    "& and && are different operators. So are | and ||. The compiler accepts both " +
    "and only one of them does what you meant.",
    T.MARGIN, 3.3, T.CONTENT_W, 0.9);

  // -------------------------------------------------------------------------
  // 15. Section 4
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 4",
    title: "The Colour Thermometer",
    note: "Datasheet, wiring, and tonight's homework",
  });

  // -------------------------------------------------------------------------
  // 16. Datasheet
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "What the LED datasheet says", 16);
  T.addTable(s,
    ["Parameter", "Red", "Green", "Blue"],
    [
      ["Forward voltage, typical", "1.95 V", "3.3 V", "3.3 V"],
      ["Continuous current, max", "30 mA", "30 mA", "30 mA"],
      ["Brightness at 20 mA", "1500 mcd", "5000 mcd", "1500 mcd"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [3.6, 1.8, 1.8, 1.8]);
  T.addCallout(pres, s, "info", "Measured on the bench",
    "With 100 ohm all three look right. With 200 ohm the red one goes dim first, " +
    "even though it has the most voltage to spare. The last row explains why.",
    T.MARGIN, 3.3, T.CONTENT_W, 1.0);

  // -------------------------------------------------------------------------
  // 17. Wiring
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Wiring the RGB LED", 17);
  T.addTable(s,
    ["LED pin", "Connects to", "Through"],
    [
      ["Longest pin, common anode", "3V3", "direct"],
      ["Red", "GPIO 5", "100 ohm"],
      ["Green", "GPIO 6", "100 ohm"],
      ["Blue", "GPIO 7", "100 ohm"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [3.6, 2.7, 2.7]);
  T.addCallout(pres, s, "warn", "Common anode means backwards",
    "LOW switches a colour ON. HIGH switches it OFF. The same logic as the LED " +
    "on the board itself.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.9);

  // -------------------------------------------------------------------------
  // 18. Colour as three bits
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A colour is three bits", 18);
  T.addTable(s,
    ["Blue", "Green", "Red", "Colour", "Temperature band"],
    [
      ["1", "0", "0", "blue", "cold"],
      ["1", "1", "0", "cyan", "cool"],
      ["0", "1", "0", "green", "normal"],
      ["0", "1", "1", "yellow", "warm"],
      ["0", "0", "1", "red", "hot"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [1.2, 1.2, 1.2, 2.4, 3.0]);
  T.addCallout(pres, s, "info", "Three switches, eight colours",
    "Bit 0 is red, bit 1 is green, bit 2 is blue. Five of the eight combinations " +
    "are used here.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 19. Summary
  // -------------------------------------------------------------------------
  T.summarySlide(pres, 19,
    [
      "A function has a name, its input, and one result",
      "A local variable dies at the closing brace; static survives the return",
      "Every call pushes a frame on the stack and every return drops it",
      "Bits are read with a shift and a mask",
    ],
    "Build the colour thermometer: the knob sets the temperature, the LED shows the band. " +
    "Star task: stop the colour flickering at the edge of a band.");

  await T.save(pres, "lesson06.pptx");
  console.log("OK: lesson06.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
