/*
 * lesson05.js - Lesson 05: Variables, Types and Arithmetic
 *
 * Build: node lesson05.js
 * QA:    soffice --headless --convert-to pdf lesson05.pptx --outdir /tmp/qa
 *        pdftoppm -png -r 70 /tmp/qa/lesson05.pdf /tmp/qa/slide
 */

const T = require("./slides-theme");

(async () => {
  const { FaMicrochip } = require("react-icons/fa");

  const pres = T.newDeck(
    "Lesson 05 - Variables, Types and Arithmetic",
    "Lesson 05  |  Variables, Types and Arithmetic"
  );

  const iconChip = await T.icon(FaMicrochip, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 05",
    title: "Variables, Types and Arithmetic",
    subtitle: "From bits in memory to a temperature on the screen",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Session 5 of 25",
    iconData: iconChip,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addBullets(s, [
    "Every value a program uses lives somewhere in memory",
    "A type decides how many bytes that is, and which values fit",
    "Integer arithmetic on a microcontroller follows rules that surprise you",
    "Last hour: a potentiometer on your board, printing a temperature",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.6);
  T.addCallout(pres, s, "info", "Goal",
    "By the end of the lesson your board prints a temperature between -40 and +140 F, " +
    "and you can explain every line of the program that does it.");

  // -------------------------------------------------------------------------
  // 3. Section 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "Memory and Variables",
    note: "What the board actually stores",
  });

  // -------------------------------------------------------------------------
  // 4. Memory is numbered boxes
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Memory is a row of numbered boxes", 4);
  T.addTable(s,
    ["Address", "Bits in that box", "Could mean"],
    [
      ["0x3FC80000", "00011001", "the number 25"],
      ["0x3FC80001", "01000001", "the number 65, or the letter A"],
      ["0x3FC80002", "11111111", "the number 255, or the number -1"],
    ],
    T.MARGIN, 1.45, T.CONTENT_W, [2.6, 3.0, 3.4]);
  T.addBullets(s, [
    "The board stores bits. That is all it stores.",
    "The type is the instruction for how to read them.",
    "Same bits, different type, different value.",
  ], T.MARGIN, 3.15, T.CONTENT_W, 1.6, 15);

  // -------------------------------------------------------------------------
  // 5. Three properties of a variable
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A variable has three properties", 5);
  T.addCode(s, "uint8_t red = 25U;", 2.5, 1.25, 5.0, 0.7, 14);
  T.addCards(pres, s, [
    { title: "Name", body: "What you call it in the source. The chip never sees it." },
    { title: "Address", body: "Which box in memory holds the value." },
    { title: "Size", body: "How many boxes it takes. The type decides." },
  ], 2.15, 1.8);
  T.addCallout(pres, s, "info", "Why size matters here",
    "A laptop has gigabytes of memory. This board has 400 kB in total. Size is not a detail.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 6. sizeof
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Ask the compiler: sizeof", 6);
  T.addCode(s, [
    "#include <stdint.h>",
    "",
    "uint8_t  small  = 7U;",
    "uint16_t medium = 7U;",
    "uint32_t large  = 7U;",
    "",
    "Serial.println(sizeof(small));",
    "Serial.println(sizeof(medium));",
    "Serial.println(sizeof(large));",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.3, 3.15, 12);
  T.addBullets(s, [
    "The same value 7 in all three",
    "Different boxes, different counts",
    "Prints 1, then 2, then 4",
    "Predict the output first, then run it",
  ], 6.1, T.BODY_TOP + 0.3, 3.4, 2.4, 14);

  // -------------------------------------------------------------------------
  // 7. Section 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "Types and Their Limits",
    note: "Every type has an edge, and values fall off it",
  });

  // -------------------------------------------------------------------------
  // 8. The types we use
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The types we use in this course", 8);
  T.addTable(s,
    ["Type", "Bytes", "Lowest", "Highest"],
    [
      ["uint8_t", "1", "0", "255"],
      ["int8_t", "1", "-128", "127"],
      ["uint16_t", "2", "0", "65535"],
      ["int16_t", "2", "-32768", "32767"],
      ["int32_t", "4", "-2147483648", "2147483647"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.4, 1.4, 2.6, 2.6]);
  T.addCallout(pres, s, "info", "Reading the names",
    "u means unsigned: zero and upwards, no negative values. The number is how many bits.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 9. Why not int
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Why not just write int?", 9);
  T.addBullets(s, [
    "int is not the same size everywhere: 4 bytes here, 2 bytes on other chips",
    "uint8_t means exactly one byte, on every machine, forever",
    "Sensor values and hardware registers have fixed widths; your variables should match",
    "Course rule: fixed-width types for anything that touches hardware",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.6);
  T.addCallout(pres, s, "warn", "Style rule",
    "Unsigned constants carry an explicit suffix: 500U, 3U. It tells both the compiler " +
    "and the next reader what you meant.");

  // -------------------------------------------------------------------------
  // 10. The edge of a type
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "What happens at the edge", 10);
  T.addCode(s, [
    "uint8_t counter = 250U;",
    "",
    "for (uint8_t i = 0U; i < 8U; i++) {",
    "    Serial.println(counter);",
    "    counter = counter + 1U;",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.3, 2.3, 12);
  T.addBullets(s, [
    "Prints 250 251 252 253 254 255 0 1",
    "No error. No warning. It simply wraps.",
    "int8_t does the same: 120 + 10 gives -126",
  ], 6.1, T.BODY_TOP + 0.1, 3.4, 2.2, 14);
  T.addCallout(pres, s, "error", "This is not a bug in C",
    "The type has 256 possible values. After the last one it starts again from the first. " +
    "Choosing a type means choosing where that edge sits.",
    T.MARGIN, 3.9, T.CONTENT_W, 1.0);

  // -------------------------------------------------------------------------
  // 11. Section 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "Arithmetic That Surprises You",
    note: "Integer division does not do what school taught",
  });

  // -------------------------------------------------------------------------
  // 12. Integer division
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Integer division cuts", 12);
  T.addCode(s, [
    "Serial.println(5 / 9);      // 0",
    "Serial.println(9 / 5);      // 1",
    "Serial.println(-5 / 2);     // -2, not -3",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.3, 1.4, 12);
  T.addBullets(s, [
    "There is no fraction to store",
    "The part after the point is dropped",
    "Dropped, not rounded",
    "Negatives cut toward zero too",
  ], 6.1, T.BODY_TOP, 3.4, 2.4, 14);
  T.addCallout(pres, s, "warn", "Where this bites you tonight",
    "The conversion formula contains 5 / 9. Written on its own, that is zero, " +
    "and the whole result becomes zero with it.",
    T.MARGIN, 3.0, 5.3, 1.1);

  // -------------------------------------------------------------------------
  // 13. Order of operations
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The order changes the answer", 13);
  T.addCode(s, [
    "int16_t f = 100;",
    "",
    "int16_t a = (f - 32) * 5 / 9;   // 37, correct",
    "int16_t b = (f - 32) / 9 * 5;   // 35, wrong",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.2, 1.8, 12);
  T.addBullets(s, [
    "Multiply first, divide last",
    "Both compile",
    "Both look reasonable",
    "Nothing warns you",
  ], 6.7, T.BODY_TOP, 2.8, 2.0, 14);
  T.addCallout(pres, s, "ok", "Two points you can check",
    "-40 F is exactly -40 C. 32 F is exactly 0 C. If your program agrees on both, " +
    "the formula is right.",
    T.MARGIN, 3.2, T.CONTENT_W, 0.9);

  // -------------------------------------------------------------------------
  // 14. Conditions and loops
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Conditions and loops, the short version", 14);
  T.addCode(s, [
    "if (celsius < 0) {",
    "    Serial.println(\"below freezing\");",
    "} else {",
    "    Serial.println(\"above freezing\");",
    "}",
    "",
    "// loop() is already a forever loop.",
    "// Everything you write there repeats.",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 2.85, 12);
  T.addBullets(s, [
    "== compares, = assigns",
    "The braces hold what runs",
    "You need exactly this much to read tonight's homework",
  ], 6.6, T.BODY_TOP + 0.2, 2.9, 2.4, 13);

  // -------------------------------------------------------------------------
  // 15. Section 4
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 4",
    title: "On the Board",
    note: "Potentiometer, serial monitor, and your homework",
  });

  // -------------------------------------------------------------------------
  // 16. Wiring
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Wiring the potentiometer", 16);
  T.addTable(s,
    ["Potentiometer pin", "Connects to", "Note"],
    [
      ["Middle pin (wiper)", "GPIO 4", "Analog input"],
      ["One outer pin", "3V3", "See the README"],
      ["Other outer pin", "GND", "Directly"],
    ],
    T.MARGIN, 1.45, T.CONTENT_W, [3.0, 2.4, 3.6]);
  T.addBullets(s, [
    "Three legs go straight into the breadboard. No soldering.",
    "Turning the knob changes the voltage on the middle pin.",
  ], T.MARGIN, 3.1, T.CONTENT_W, 1.0, 15);
  T.addCallout(pres, s, "info", "If the knob works backwards",
    "Swap the two outer pins. Nothing is damaged, the direction simply reverses.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 17. The scaffold
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The program you are given", 17);
  T.addCode(s, [
    "// Already written and working:",
    "uint16_t counts = readPotentiometer();",
    "int16_t fahrenheit = mapToFahrenheit(counts);",
    "",
    "Serial.print(\"F = \");",
    "Serial.println(fahrenheit);",
    "",
    "// TODO: print the same temperature in Celsius",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 2.85, 12);
  T.addBullets(s, [
    "Reading the knob: done for you",
    "Mapping to Fahrenheit: done for you",
    "The arithmetic: yours",
    "Whole numbers only. No float.",
  ], 6.6, T.BODY_TOP + 0.2, 2.9, 2.4, 13);

  // -------------------------------------------------------------------------
  // 18. Summary
  // -------------------------------------------------------------------------
  T.summarySlide(pres, 18,
    [
      "Memory holds bits; the type decides how they are read",
      "Fixed-width types make the size and the range visible",
      "Values wrap at the edge of their type, silently",
      "Integer division cuts, and the order of operations changes the result",
    ],
    "Convert the Fahrenheit reading to Celsius using whole-number arithmetic. " +
    "Check your result at -40 F and at 32 F. Commit and push before the next session.");

  await T.save(pres, "lesson05.pptx");
  console.log("OK: lesson05.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
