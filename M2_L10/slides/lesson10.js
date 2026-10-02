/*
 * lesson10.js - Lesson 10: States, Templates and Interrupts
 *
 * Build: node lesson10.js
 * QA:    soffice --headless --convert-to pdf lesson10.pptx --outdir /tmp/qa
 *        pdftoppm -png -r 70 /tmp/qa/lesson10.pdf /tmp/qa/slide
 */

const T = require("./slides-theme");

(async () => {
  const { FaProjectDiagram } = require("react-icons/fa");

  const pres = T.newDeck(
    "Lesson 10 - States, Templates and Interrupts",
    "Lesson 10  |  States, Templates and Interrupts"
  );

  const iconFlow = await T.icon(FaProjectDiagram, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 10",
    title: "States, Templates and Interrupts",
    subtitle: "C++ that fits in 400 kB",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Session 10 of 25",
    iconData: iconFlow,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addBullets(s, [
    "Your reaction game is a state machine, and enum class gives the states names",
    "Why we stay away from new, String and std::vector on a microcontroller",
    "Templates: one class for any type, with its size fixed before the program runs",
    "Interrupts: letting the hardware tell you a button was pressed",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.4);
  T.addCallout(pres, s, "info", "Goal",
    "By the end of the lesson you can store a history of values without the heap, " +
    "and catch a button press the moment it happens.");

  // -------------------------------------------------------------------------
  // 3. Section 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "State Machines",
    note: "What your game already was",
  });

  // -------------------------------------------------------------------------
  // 4. The game as states
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Your game, drawn as states", 4);
  T.addTable(s,
    ["State", "LED", "What can happen", "Next state"],
    [
      ["Waiting", "off", "the pause ends / someone presses early", "Go / Result"],
      ["Go", "green", "a press", "Result"],
      ["Result", "winner's colour", "two seconds pass", "Waiting"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [1.6, 1.9, 3.5, 2.0]);
  T.addCallout(pres, s, "ok", "This has a name: a state machine",
    "The program is always in exactly one state. Events move it to the next one. " +
    "Most firmware you will ever read is built this way.",
    T.MARGIN, 3.1, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 5. enum class
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Names for the states: enum class", 5);
  T.addCode(s, [
    "enum class State : uint8_t {",
    "    Waiting,",
    "    Go,",
    "    Result",
    "};",
    "",
    "State g_state = State::Waiting;",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.6, 2.55, 12);
  T.addBullets(s, [
    "Names, not numbers",
    "State::Go cannot be mixed up with 1, or with another enum",
    ": uint8_t keeps it one byte",
  ], 6.3, T.BODY_TOP + 0.2, 3.2, 2.4, 13);

  // -------------------------------------------------------------------------
  // 6. switch on the state
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "loop() asks one question", 6);
  T.addCode(s, [
    "void loop(void) {",
    "    switch (g_state) {",
    "        case State::Waiting: handleWaiting(); break;",
    "        case State::Go:      handleGo();      break;",
    "        case State::Result:  handleResult();  break;",
    "    }",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.4, 2.55, 12);
  T.addCallout(pres, s, "ok", "One state, one function",
    "loop() only asks where are we. Each handler decides what happens next and " +
    "changes g_state when it is time to move on.",
    T.MARGIN, 3.9, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 7. Section 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "Memory Without Surprises",
    note: "What to avoid, and what to use instead",
  });

  // -------------------------------------------------------------------------
  // 8. What to avoid
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "What we leave out, and why", 8);
  T.addTable(s,
    ["Avoid", "Why", "Instead"],
    [
      ["new, delete, malloc", "memory carved up while running; it fragments", "arrays and objects made once"],
      ["String", "grows on the heap behind your back", "char buffers and snprintf"],
      ["std::vector", "the same heap, plus copies when it grows", "a fixed-size template: today"],
      ["exceptions", "big code, hidden paths, often switched off", "return a status"],
      ["RTTI, dynamic_cast", "flash spent on type information", "know your types by design"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.4, 3.6, 3.0], 11);
  T.addCallout(pres, s, "warn", "The failure comes hours later",
    "A board that allocates and frees for a whole night ends up with plenty of free " +
    "memory in pieces too small to use. It crashes, and nothing points at the cause.",
    T.MARGIN, 3.85, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 9. Function template
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "One function for any type", 9);
  T.addCode(s, [
    "template <typename T>",
    "T largest(T a, T b) {",
    "    return (a > b) ? a : b;",
    "}",
    "",
    "largest(3U, 7U);       // T is unsigned",
    "largest(-40, 12);      // T is int",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.0, 2.55, 12);
  T.addBullets(s, [
    "T is a placeholder for a type",
    "The compiler fills it in at each call",
  ], 6.7, T.BODY_TOP + 0.2, 2.8, 2.0, 13);
  T.addCallout(pres, s, "info", "A template costs flash, not RAM",
    "The compiler writes one copy of the function for every type you actually use, " +
    "and none for the types you do not.",
    T.MARGIN, 3.9, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 10. RingBuffer
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A class template: RingBuffer", 10);
  T.addCode(s, [
    "template <typename T, uint8_t N>",
    "class RingBuffer {",
    "public:",
    "    void push(T value);",
    "    uint8_t count(void) const;",
    "    T at(uint8_t index) const;   // 0 is the oldest",
    "",
    "private:",
    "    T m_items[N];",
    "    uint8_t m_next = 0U;",
    "    uint8_t m_count = 0U;",
    "};",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.75, 11);
  T.addBullets(s, [
    "T: what is stored",
    "N: how many, fixed when you compile",
    "const after a method: it only reads",
    "sizeof is known; nothing is allocated while running",
  ], 6.6, T.BODY_TOP + 0.3, 2.9, 3.2, 13);

  // -------------------------------------------------------------------------
  // 11. push
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Round and round", 11);
  T.addCode(s, [
    "void push(T value) {",
    "    m_items[m_next] = value;",
    "    m_next = (uint8_t)((m_next + 1U) % N);   // after the last: 0",
    "    if (m_count < N) {",
    "        m_count++;",
    "    }",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.6, 2.55, 12);
  T.addCallout(pres, s, "ok", "Full is not a problem",
    "Once all N places are used, the newest value simply overwrites the oldest. " +
    "The buffer always holds the last N values, in the same memory, forever.",
    T.MARGIN, 3.9, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 12. Section 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "Smoothing the Knob",
    note: "The averaging from lesson 05, done properly",
  });

  // -------------------------------------------------------------------------
  // 13. Moving average
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A moving average", 13);
  T.addCode(s, [
    "RingBuffer<uint16_t, 16> samples;",
    "",
    "samples.push((uint16_t)analogRead(POT_PIN));",
    "",
    "uint32_t sum = 0U;",
    "for (uint8_t i = 0U; i < samples.count(); i++) {",
    "    sum += samples.at(i);",
    "}",
    "const uint16_t smooth = (uint16_t)(sum / samples.count());",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.95, 11);
  T.addCallout(pres, s, "info", "One new reading per pass, not sixteen",
    "In lesson 05 we took 16 readings in a row and waited for all of them. Now each pass " +
    "adds one, and the average always covers the last 16.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.85);

  // -------------------------------------------------------------------------
  // 14. Section 4
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 4",
    title: "Interrupts",
    note: "Let the hardware call you",
  });

  // -------------------------------------------------------------------------
  // 15. Polling vs interrupt
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Asking all the time, or being told", 15);
  T.addTable(s,
    ["", "Polling", "Interrupt"],
    [
      ["Who checks", "loop(), over and over", "the hardware, by itself"],
      ["Reacts", "when loop() gets round to it", "within microseconds"],
      ["Can miss", "anything shorter than one pass", "nothing"],
      ["Costs", "nothing, it is simple", "rules you must follow"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.2, 3.4, 3.4]);
  T.addCallout(pres, s, "info", "Why the game wants it",
    "The press is stamped with the time the moment it happens, not when loop() notices. " +
    "Reaction times become fair to the microsecond.",
    T.MARGIN, 3.4, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 16. attachInterrupt
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "attachInterrupt", 16);
  T.addCode(s, [
    "static volatile uint32_t g_pressed_at_us = 0U;",
    "static volatile bool g_pressed = false;",
    "",
    "void IRAM_ATTR onButtonA(void) {",
    "    g_pressed_at_us = micros();",
    "    g_pressed = true;",
    "}",
    "",
    "attachInterrupt(digitalPinToInterrupt(PIN_A), onButtonA, FALLING);",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.95, 11);
  T.addBullets(s, [
    "FALLING: from HIGH to LOW, the moment of the press",
    "loop() checks g_pressed, does the real work, and clears it",
  ], T.MARGIN, 4.2, T.CONTENT_W, 0.85, 14);

  // -------------------------------------------------------------------------
  // 17. Rules
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The rules for an interrupt handler", 17);
  T.addBullets(s, [
    "Keep it short: note what happened, leave the work to loop()",
    "No Serial, no delay(), nothing that waits",
    "Variables shared with loop() are volatile",
    "IRAM_ATTR keeps the handler in RAM, ready at any moment",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.3);
  T.addCallout(pres, s, "error", "Bounce does not go away",
    "One press can fire the interrupt five times. The handler sees every bounce, just " +
    "faster than loop() ever could. Debouncing is still your job.",
    T.MARGIN, 3.7, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 18. Summary
  // -------------------------------------------------------------------------
  T.summarySlide(pres, 18,
    [
      "A state machine: one state at a time, events move it on; enum class names the states",
      "No heap at run time: fixed arrays, char buffers, objects made once",
      "A template is written once and works for any type; N fixes the size in advance",
      "An interrupt reacts at once, and its handler must stay short and simple",
    ],
    "Extend the reaction game with a history of the last 10 reaction times per player " +
    "in a RingBuffer. Details are in homework/README.md.");

  await T.save(pres, "lesson10.pptx");
  console.log("OK: lesson10.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
