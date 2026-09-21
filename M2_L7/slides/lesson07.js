/*
 * lesson07.js - Lesson 07: Pointers, Arrays and Strings
 *
 * Build: node lesson07.js
 * QA:    soffice --headless --convert-to pdf lesson07.pptx --outdir /tmp/qa
 *        pdftoppm -png -r 70 /tmp/qa/lesson07.pdf /tmp/qa/slide
 */

const T = require("./slides-theme");

(async () => {
  const { FaMemory } = require("react-icons/fa");

  const pres = T.newDeck(
    "Lesson 07 - Pointers, Arrays and Strings",
    "Lesson 07  |  Pointers, Arrays and Strings"
  );

  const iconMem = await T.icon(FaMemory, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 07",
    title: "Pointers, Arrays and Strings",
    subtitle: "Where things live, and how to point at them",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Session 7 of 25",
    iconData: iconMem,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addBullets(s, [
    "Arrays: many values under one name",
    "Addresses and pointers: where a value lives, and how to reach it",
    "Why an array and a pointer are almost the same thing",
    "Strings: text is just an array that ends in zero",
    "Last hour: a button, and why one press counts as five",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.8);
  T.addCallout(pres, s, "info", "Goal",
    "By the end of the lesson you can pass an array to a function, get two answers back " +
    "from one call, and build a line of text safely.");

  // -------------------------------------------------------------------------
  // 3. Section 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "Arrays",
    note: "A row of boxes with one name",
  });

  // -------------------------------------------------------------------------
  // 4. Many boxes, one name
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Many boxes, one name", 4);
  T.addCode(s, [
    "uint8_t readings[5] = { 10U, 20U, 30U, 40U, 50U };",
    "",
    "Serial.println(readings[0]);    // 10, the first",
    "Serial.println(readings[4]);    // 50, the last",
    "Serial.println(sizeof(readings));   // 5 bytes",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.2, 2.0, 12);
  T.addBullets(s, [
    "Counting starts at 0, so five boxes are numbered 0 to 4",
    "All boxes have the same type",
    "The size is fixed when you write the program",
  ], T.MARGIN, 3.45, T.CONTENT_W, 1.5, 15);

  // -------------------------------------------------------------------------
  // 5. Walking an array
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Walking through an array", 5);
  T.addCode(s, [
    "static const uint8_t COUNT = 5U;",
    "uint8_t readings[COUNT] = { 10U, 20U, 30U, 40U, 50U };",
    "",
    "uint16_t sum = 0U;",
    "for (uint8_t i = 0U; i < COUNT; i++) {",
    "    sum = sum + readings[i];",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.2, 2.6, 12);
  T.addCallout(pres, s, "error", "No error at the edge",
    "readings[5] compiles and runs. It reads whatever happens to sit next to the array. " +
    "C does not check, so you have to.",
    T.MARGIN, 4.0, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 6. Section 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "Addresses and Pointers",
    note: "Every box has a number on the door",
  });

  // -------------------------------------------------------------------------
  // 7. Four expressions
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Four things to read aloud", 7);
  T.addCode(s, [
    "uint8_t value = 42U;",
    "uint8_t *p = &value;    // p holds the address of value",
    "*p = 7U;                // value is now 7",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.0, 1.3, 12);
  T.addTable(s,
    ["Expression", "Read it as", "Gives"],
    [
      ["value", "the value", "42, then 7"],
      ["&value", "the address of value", "a number like 0x3FC8..."],
      ["p", "the pointer itself", "the same address"],
      ["*p", "what p points at", "the value in that box"],
    ],
    T.MARGIN, 2.7, T.CONTENT_W, [2.2, 3.6, 3.2]);

  // -------------------------------------------------------------------------
  // 8. Where things live
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Where things live", 8);
  T.addTable(s,
    ["Kind of variable", "Lives in", "Exists"],
    [
      ["Global, or static", "RAM, a fixed place", "the whole time the board runs"],
      ["Local", "the stack, also RAM", "only while its function runs"],
      ["Text in quotes", "flash, a different range", "forever, and read only"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.6, 3.0, 3.4]);
  T.addCallout(pres, s, "info", "Print the addresses and look",
    "Your numbers will differ from your neighbour's. What matters is which ones sit " +
    "close together and which one is in a completely different range.",
    T.MARGIN, 3.3, T.CONTENT_W, 1.0);

  // -------------------------------------------------------------------------
  // 9. Changing the caller's variable
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Changing someone else's variable", 9);
  T.addCode(s, [
    "void addOne(uint8_t *target) {",
    "    *target = *target + 1U;",
    "}",
    "",
    "uint8_t count = 5U;",
    "addOne(&count);    // count is now 6",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.0, 2.3, 12);
  T.addBullets(s, [
    "The function gets the address",
    "Through it, it changes the original",
    "Without the pointer it would change a copy",
  ], 6.7, T.BODY_TOP + 0.1, 2.8, 2.3, 13);
  T.addCallout(pres, s, "ok", "Why bother",
    "return hands back one value. Pointers let a function hand back as many as it needs.",
    T.MARGIN, 3.7, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 10. Two answers from one call
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Two answers from one call", 10);
  T.addCode(s, [
    "void minMax(const uint8_t *values, uint8_t count,",
    "            uint8_t *min_out, uint8_t *max_out);",
    "",
    "uint8_t lowest;",
    "uint8_t highest;",
    "minMax(readings, COUNT, &lowest, &highest);",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.4, 2.3, 12);
  T.addBullets(s, [
    "const: the function promises to only read the array",
    "count: the function cannot find out the length by itself",
    "_out: the names say which pointers are for answers",
  ], T.MARGIN, 3.7, T.CONTENT_W, 1.3, 14);

  // -------------------------------------------------------------------------
  // 11. Section 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "Arrays Meet Pointers",
    note: "And text turns out to be an array too",
  });

  // -------------------------------------------------------------------------
  // 12. Array name is an address
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "An array name is an address", 12);
  T.addCode(s, [
    "uint8_t *first = readings;         // the same as &readings[0]",
    "",
    "void show(const uint8_t *values, uint8_t count) {",
    "    Serial.println(sizeof(values));   // 4, not the array size",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.0, 12);
  T.addCallout(pres, s, "warn", "sizeof lies inside a function",
    "A passed array arrives as a pointer. sizeof then measures the pointer, " +
    "4 bytes on this chip, not the array. That is why the count travels with it.",
    T.MARGIN, 3.4, T.CONTENT_W, 1.0);

  // -------------------------------------------------------------------------
  // 13. A string is an array
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A string is an array of char", 13);
  T.addCode(s, "char greeting[] = \"Hi\";", 2.5, 1.25, 5.0, 0.7, 14);
  T.addTable(s,
    ["Index", "0", "1", "2"],
    [
      ["Content", "'H'", "'i'", "'\\0'"],
    ],
    T.MARGIN, 2.2, T.CONTENT_W, [2.4, 2.2, 2.2, 2.2]);
  T.addBullets(s, [
    "Two letters take three bytes",
    "The zero at the end is how every function knows where the text stops",
    "Forget it, and printing runs on into whatever memory follows",
  ], T.MARGIN, 3.2, T.CONTENT_W, 1.6, 15);

  // -------------------------------------------------------------------------
  // 14. snprintf
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Building text safely", 14);
  T.addCode(s, [
    "char line[32];",
    "snprintf(line, sizeof(line), \"%u x %u = %u\", a, b, a * b);",
    "Serial.println(line);",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 1.4, 12);
  T.addBullets(s, [
    "The buffer is an array you declared, with a fixed size",
    "snprintf never writes past that size, and always ends with the zero",
    "Here sizeof works: line is a real array, not a passed pointer",
  ], T.MARGIN, 2.8, T.CONTENT_W, 1.5, 15);
  T.addCallout(pres, s, "warn", "Why not String",
    "Arduino String grows on the heap at run time. On a small board that ends in fragments " +
    "and a crash hours later. A fixed buffer cannot do that.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 15. Formats
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "One number, three ways to write it", 15);
  T.addTable(s,
    ["Format", "255 prints as", "Meaning"],
    [
      ["%u", "255", "decimal"],
      ["%o", "377", "octal, base 8"],
      ["%X", "FF", "hexadecimal, base 16"],
      ["%4u", "_255", "at least 4 wide, padded with spaces"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.0, 3.0, 4.0]);
  T.addCode(s,
    "const char *FORMATS[] = { \"%4o\", \"%4u\", \"%4X\" };",
    T.MARGIN, 3.35, T.CONTENT_W, 0.7, 13);
  T.addBullets(s, [
    "An array of pointers to strings: every topic of today in one line",
  ], T.MARGIN, 4.2, T.CONTENT_W, 0.6, 14);

  // -------------------------------------------------------------------------
  // 16. Section 4
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 4",
    title: "The Button",
    note: "One press, many clicks",
  });

  // -------------------------------------------------------------------------
  // 17. Wiring
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Wiring the button", 17);
  T.addTable(s,
    ["Button leg", "Connects to"],
    [
      ["One leg", "GPIO 0"],
      ["The other leg", "GND"],
    ],
    T.MARGIN, 1.4, 6.0, [3.0, 3.0]);
  T.addCode(s, [
    "pinMode(BUTTON_PIN, INPUT_PULLUP);",
    "",
    "if (digitalRead(BUTTON_PIN) == LOW) {",
    "    // pressed",
    "}",
  ].join("\n"), T.MARGIN, 2.7, 6.0, 2.0, 12);
  T.addBullets(s, [
    "No resistor needed",
    "The chip pulls the pin up to 3V3 inside",
    "Pressing connects it to GND",
    "So pressed reads LOW",
  ], 6.8, 1.4, 2.7, 3.0, 13);

  // -------------------------------------------------------------------------
  // 18. Bounce
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "One press, five clicks", 18);
  T.addBullets(s, [
    "Metal contacts do not close cleanly; they bounce for a few milliseconds",
    "The chip is fast enough to see every bounce as a separate press",
    "Holding the button down is not the same as pressing it",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 1.3, 15);
  T.addCards(pres, s, [
    { title: "Press, not hold", body: "React when the pin changes from HIGH to LOW, not while it stays LOW." },
    { title: "Wait it out", body: "After a change, ignore the pin until the contacts have settled." },
  ], 2.55, 1.5);
  T.addCallout(pres, s, "ok", "Solved together, in class",
    "Both need a value that survives between calls. You know which keyword does that.",
    T.MARGIN, 4.3, T.CONTENT_W, 0.75);

  // -------------------------------------------------------------------------
  // 19. Summary
  // -------------------------------------------------------------------------
  T.summarySlide(pres, 19,
    [
      "An array is a row of boxes; counting starts at 0 and nothing checks the end",
      "& gives an address, * reaches through it",
      "A passed array is a pointer, so its length travels separately",
      "A string is a char array ending in zero; build it with snprintf",
    ],
    "Print a 16 x 16 multiplication table. The button switches it between octal, decimal " +
    "and hexadecimal. Details are in the README.");

  await T.save(pres, "lesson07.pptx");
  console.log("OK: lesson07.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
