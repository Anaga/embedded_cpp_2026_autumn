/*
 * lesson09.js - Lesson 09: References, Overloading and a Button Class
 *
 * Build: node lesson09.js
 * QA:    soffice --headless --convert-to pdf lesson09.pptx --outdir /tmp/qa
 *        pdftoppm -png -r 70 /tmp/qa/lesson09.pdf /tmp/qa/slide
 */

const T = require("./slides-theme");

(async () => {
  const { FaGamepad } = require("react-icons/fa");

  const pres = T.newDeck(
    "Lesson 09 - References, Overloading and a Button Class",
    "Lesson 09  |  References, Overloading and a Button Class"
  );

  const iconPad = await T.icon(FaGamepad, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 09",
    title: "References and Overloading",
    subtitle: "And why two buttons need two objects",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Session 9 of 25",
    iconData: iconPad,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addBullets(s, [
    "References: a second name for a variable that already exists",
    "Overloading: one name, several versions, chosen by the arguments",
    "A second button, and the moment static stops being enough",
    "A Button class: every object remembers for itself",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.4);
  T.addCallout(pres, s, "info", "Goal",
    "By the end of the lesson two buttons work side by side without disturbing each " +
    "other, and you have the parts for a two-player game.");

  // -------------------------------------------------------------------------
  // 3. Section 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "References",
    note: "Another name for the same box",
  });

  // -------------------------------------------------------------------------
  // 4. A reference
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A second name for the same box", 4);
  T.addCode(s, [
    "uint8_t count = 5U;",
    "uint8_t &alias = count;    // alias is another name for count",
    "",
    "alias = 7U;                // count is now 7",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.2, 1.6, 12);
  T.addBullets(s, [
    "No new box is made: alias and count are the same memory",
    "It must be tied to something when it is created",
    "Once tied, it can never point anywhere else",
  ], T.MARGIN, 3.0, T.CONTENT_W, 1.6, 15);

  // -------------------------------------------------------------------------
  // 5. Three ways to pass
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Three ways to hand over a value", 5);
  T.addTable(s,
    ["", "Copy", "Pointer", "Reference"],
    [
      ["Declared as", "Colour c", "Colour *c", "Colour &c"],
      ["Called as", "f(x)", "f(&x)", "f(x)"],
      ["Can change x", "no", "yes", "yes"],
      ["Can be empty", "no", "yes, nullptr", "no"],
      ["Fields reached with", "c.red", "c->red", "c.red"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.6, 2.1, 2.2, 2.1]);
  T.addCallout(pres, s, "info", "A reference is a pointer that behaves",
    "It does the same job as a pointer, but it cannot be empty, cannot be moved, " +
    "and needs no & or * at the call.",
    T.MARGIN, 3.8, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 6. const reference
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The one you will use most", 6);
  T.addCode(s, [
    "void setColour(const Colour &c);           // no copy, read only",
    "",
    "led.setColour(warm);                       // called like a copy",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 1.4, 12);
  T.addBullets(s, [
    "& : the function works on the original, nothing is copied",
    "const : and it promises not to change it",
    "At the call it looks exactly like passing a copy",
  ], T.MARGIN, 2.8, T.CONTENT_W, 1.4, 15);
  T.addCallout(pres, s, "ok", "The default for anything bigger than a number",
    "Fast like a pointer, safe like a copy. For a single uint8_t, a plain copy is still fine.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 7. Section 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "Overloading",
    note: "One name, several versions",
  });

  // -------------------------------------------------------------------------
  // 8. Two setColour
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Same name, different inputs", 8);
  T.addCode(s, [
    "void setColour(const Colour &c);",
    "void setColour(uint8_t red, uint8_t green, uint8_t blue);",
    "",
    "led.setColour(warm);               // the first one",
    "led.setColour(255U, 120U, 0U);     // the second one",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.0, 12);
  T.addBullets(s, [
    "The compiler picks by the number and the types of the arguments",
    "The caller uses whichever form is handier at that moment",
    "In C this would need two names: setColour and setColourRgb",
  ], T.MARGIN, 3.4, T.CONTENT_W, 1.5, 15);

  // -------------------------------------------------------------------------
  // 9. Write the work once
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Write the work once", 9);
  T.addCode(s, [
    "void RgbLed::setColour(uint8_t red, uint8_t green, uint8_t blue) {",
    "    const Colour c = { red, green, blue };",
    "    setColour(c);                    // hand over to the other one",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 1.6, 12);
  T.addBullets(s, [
    "One version does the real work; the others only translate",
    "A bug fixed there is fixed for every form",
  ], T.MARGIN, 3.0, T.CONTENT_W, 1.0, 15);
  T.addCallout(pres, s, "error", "Return type alone is not enough",
    "uint8_t read() and int16_t read() cannot live together: the call read() looks the same " +
    "for both. The parameters have to differ.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 10. Section 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "Two Buttons",
    note: "Where static stops being enough",
  });

  // -------------------------------------------------------------------------
  // 11. Wiring
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Wiring the second button", 11);
  T.addTable(s,
    ["Part", "Connects to"],
    [
      ["Button A, player 1", "GPIO 0 and GND"],
      ["Button B, player 2", "GPIO 1 and GND"],
      ["RGB LED", "GPIO 5, 6, 7 as before, common anode to 3V3"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [3.2, 5.8]);
  T.addCallout(pres, s, "info", "Both buttons use INPUT_PULLUP",
    "No resistors. Not pressed reads HIGH, pressed reads LOW, exactly as in lesson 07.",
    T.MARGIN, 3.2, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 12. The broken version
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The obvious way, and why it breaks", 12);
  T.addCode(s, [
    "bool isButtonPressed(uint8_t pin) {",
    "    static uint8_t previous = HIGH;",
    "    static uint32_t last_change_ms = 0U;",
    "    // ... the same as in lesson 07 ...",
    "}",
    "",
    "isButtonPressed(PIN_A);",
    "isButtonPressed(PIN_B);    // the same previous, the same timer",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.85, 12);
  T.addCallout(pres, s, "error", "The two buttons now share one memory",
    "Button A writes previous, button B overwrites it, and each one restarts the other's " +
    "timer. Hold A and press B: B is never counted.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.9);

  // -------------------------------------------------------------------------
  // 13. Why
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "static belongs to the function", 13);
  T.addCards(pres, s, [
    { title: "One function", body: "One set of static variables, made once, shared by every single call." },
    { title: "Two buttons", body: "Need two memories: two previous states and two timers." },
    { title: "The fix", body: "Move the memory into an object. One object per button." },
  ], 1.4, 1.8);
  T.addCallout(pres, s, "ok", "This is what objects are for",
    "A class describes what one button needs to remember. Every object gets its own copy.",
    T.MARGIN, 3.5, T.CONTENT_W, 0.8);

  // -------------------------------------------------------------------------
  // 14. class Button
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The Button class", 14);
  T.addCode(s, [
    "class Button {",
    "public:",
    "    Button(uint8_t pin);",
    "    void begin(void);",
    "    bool wasPressed(void);",
    "",
    "private:",
    "    uint8_t m_pin;",
    "    uint8_t m_previous;",
    "    uint32_t m_last_change_ms;",
    "};",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.8, 3.45, 11);
  T.addBullets(s, [
    "The two statics became fields",
    "Every object has its own pair",
    "wasPressed() is true once per press",
    "begin() sets INPUT_PULLUP",
  ], 6.5, T.BODY_TOP + 0.3, 3.0, 2.8, 14);

  // -------------------------------------------------------------------------
  // 15. Two objects
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Two objects, two memories", 15);
  T.addCode(s, [
    "Button player1(PIN_A);",
    "Button player2(PIN_B);",
    "",
    "void loop(void) {",
    "    if (player1.wasPressed()) { /* ... */ }",
    "    if (player2.wasPressed()) { /* ... */ }",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.2, 2.55, 12);
  T.addCallout(pres, s, "ok", "A third button is one more line",
    "The debounce logic is written once, inside the class. Each object runs it on its own " +
    "pin with its own memory.",
    T.MARGIN, 3.9, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 16. Section 4
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 4",
    title: "The Reaction Game",
    note: "Two players, two buttons, one LED",
  });

  // -------------------------------------------------------------------------
  // 17. Rules
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The rules", 17);
  T.addBullets(s, [
    "The LED is off. Both players wait with a finger on their button.",
    "After a random pause of 2 to 5 seconds, the LED turns green.",
    "The first one to press wins; the LED shows that player's colour.",
    "Press before green and it is a false start: the other player wins.",
    "The serial monitor shows the reaction time and the score.",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.8);
  T.addCallout(pres, s, "info", "You already have every part",
    "Two Button objects, the RgbLed class, millis() for time, and one new function: " +
    "random(), for the pause nobody can guess.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 18. Summary
  // -------------------------------------------------------------------------
  T.summarySlide(pres, 18,
    [
      "A reference is a second name; const & passes without a copy and without a pointer",
      "Overloads share a name and differ in their parameters",
      "static belongs to the function, so it cannot tell two buttons apart",
      "An object carries its own state: one Button per button",
    ],
    "Build the two-player reaction game. Details are in homework/README.md.");

  await T.save(pres, "lesson09.pptx");
  console.log("OK: lesson09.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
