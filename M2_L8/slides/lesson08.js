/*
 * lesson08.js - Lesson 08: Structs, Classes and PWM
 *
 * Build: node lesson08.js
 * QA:    soffice --headless --convert-to pdf lesson08.pptx --outdir /tmp/qa
 *        pdftoppm -png -r 70 /tmp/qa/lesson08.pdf /tmp/qa/slide
 */

const T = require("./slides-theme");

(async () => {
  const { FaLightbulb } = require("react-icons/fa");

  const pres = T.newDeck(
    "Lesson 08 - Structs, Classes and PWM",
    "Lesson 08  |  Structs, Classes and PWM"
  );

  const iconBulb = await T.icon(FaLightbulb, "#CADCFC");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 08",
    title: "Structs, Classes and PWM",
    subtitle: "From eight colours to sixteen million",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Session 8 of 25",
    iconData: iconBulb,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addBullets(s, [
    "Structs: values that belong together travel together",
    "The first class: an object that owns three pins",
    "PWM: how a digital pin produces something between on and off",
    "Then PWM moves inside the class, and nothing outside it changes",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.4);
  T.addCallout(pres, s, "info", "Goal",
    "By the end of the lesson your LED can show any colour, and the code that asks for " +
    "a colour does not know how the LED works.");

  // -------------------------------------------------------------------------
  // 3. Section 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "Structs",
    note: "One name for several values",
  });

  // -------------------------------------------------------------------------
  // 4. A struct
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Three numbers that belong together", 4);
  T.addCode(s, [
    "struct Colour {",
    "    uint8_t red;",
    "    uint8_t green;",
    "    uint8_t blue;",
    "};",
    "",
    "Colour warm = { 255U, 120U, 0U };",
    "Serial.println(warm.red);      // 255",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.2, 2.85, 12);
  T.addBullets(s, [
    "An array holds many of one thing",
    "A struct holds a few different things",
    "Each field has its own name and type",
    "The dot reaches a field",
  ], 6.9, T.BODY_TOP + 0.2, 2.6, 2.6, 13);

  // -------------------------------------------------------------------------
  // 5. Copies
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A struct is a value, not a pointer", 5);
  T.addCode(s, [
    "Colour a = { 10U, 20U, 30U };",
    "Colour b = a;          // b is a separate copy",
    "b.red = 99U;           // a.red is still 10",
    "",
    "void show(const Colour *c);   // no copy, and read only",
    "Colour dim(Colour c);         // takes a copy, returns a new one",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.6, 2.3, 12);
  T.addCallout(pres, s, "info", "Three bytes are cheap to copy",
    "For a small struct, passing a copy is fine and safer. For a big one, pass a pointer. " +
    "The rule is the same as for any other value.",
    T.MARGIN, 3.7, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 6. Section 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "The First Class",
    note: "A struct that can also do things",
  });

  // -------------------------------------------------------------------------
  // 7. The class
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A class: data plus behaviour", 7);
  T.addCode(s, [
    "class RgbLed {",
    "public:",
    "    RgbLed(uint8_t red, uint8_t green, uint8_t blue);",
    "    void begin(void);",
    "    void setColour(Colour c);",
    "",
    "private:",
    "    uint8_t m_red;",
    "    uint8_t m_green;",
    "    uint8_t m_blue;",
    "};",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.4, 3.45, 11);
  T.addBullets(s, [
    "public: what others may use",
    "private: what only the class touches",
    "m_ marks a field, so a local never hides one",
  ], 7.1, T.BODY_TOP + 0.3, 2.4, 2.6, 13);

  // -------------------------------------------------------------------------
  // 8. The constructor
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "The constructor runs at birth", 8);
  T.addCode(s, [
    "RgbLed::RgbLed(uint8_t red, uint8_t green, uint8_t blue)",
    "    : m_red(red), m_green(green), m_blue(blue) {",
    "}",
    "",
    "void RgbLed::begin(void) {",
    "    pinMode(m_red, OUTPUT);",
    "    pinMode(m_green, OUTPUT);",
    "    pinMode(m_blue, OUTPUT);",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.6, 2.9, 11);
  T.addCallout(pres, s, "warn", "Why begin() and not the constructor",
    "An object made before setup() runs exists before the chip is ready. Remember the pins " +
    "in the constructor, touch the hardware in begin().",
    T.MARGIN, 4.2, T.CONTENT_W, 0.85);

  // -------------------------------------------------------------------------
  // 9. Using it
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Using the object", 9);
  T.addCode(s, [
    "static const Colour RED = { 255U, 0U, 0U };",
    "",
    "RgbLed led(PIN_RED, PIN_GREEN, PIN_BLUE);",
    "",
    "void setup(void) {",
    "    led.begin();",
    "    led.setColour(RED);",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 6.2, 2.85, 12);
  T.addBullets(s, [
    "One object, one LED",
    "A second LED is a second object, same class",
    "led.m_red is not allowed: it is private",
  ], 6.9, T.BODY_TOP + 0.2, 2.6, 2.4, 13);

  // -------------------------------------------------------------------------
  // 10. Section 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "PWM",
    note: "Between on and off, very fast",
  });

  // -------------------------------------------------------------------------
  // 11. What PWM is
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "A pin has two states, and no middle", 11);
  T.addBullets(s, [
    "digitalWrite can only give 0 V or 3.3 V",
    "So the pin is switched on and off thousands of times a second",
    "The eye cannot follow that; it averages what it sees",
    "The share of time spent on is called the duty cycle",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.0);
  T.addTable(s,
    ["Duty", "The pin", "The eye"],
    [
      ["0", "always off", "dark"],
      ["64", "on a quarter of the time", "dim"],
      ["128", "on half the time", "medium"],
      ["255", "always on", "full"],
    ],
    T.MARGIN, 3.0, T.CONTENT_W, [1.8, 4.0, 3.2]);

  // -------------------------------------------------------------------------
  // 12. Frequency and resolution
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Two settings to choose", 12);
  T.addTable(s,
    ["Setting", "We use", "Why"],
    [
      ["Frequency", "5000 Hz", "slower starts to flicker, especially at the edge of vision"],
      ["Resolution", "8 bits", "256 steps, and a step fits in a uint8_t"],
      ["Largest duty", "255", "(1 << 8) - 1, the same edge as always"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [2.2, 1.8, 5.0]);
  T.addCallout(pres, s, "info", "More bits are not always better",
    "Frequency and resolution share the same timer. Ask for both too high and the chip " +
    "cannot deliver. For an LED, 8 bits at 5 kHz is comfortable.",
    T.MARGIN, 3.4, T.CONTENT_W, 1.0);

  // -------------------------------------------------------------------------
  // 13. PWM on this chip
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "PWM on the ESP32-C3", 13);
  T.addCode(s, [
    "static const uint32_t PWM_FREQ_HZ = 5000U;",
    "static const uint8_t PWM_BITS = 8U;",
    "static const uint8_t CHANNEL_RED = 0U;",
    "",
    "ledcSetup(CHANNEL_RED, PWM_FREQ_HZ, PWM_BITS);   // once, in begin()",
    "ledcAttachPin(PIN_RED, CHANNEL_RED);             // once, in begin()",
    "ledcWrite(CHANNEL_RED, 128U);                    // any time after that",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.55, 12);
  T.addBullets(s, [
    "PWM is made by a channel, not by a pin; the chip has six of them",
    "ledcAttachPin connects a pin to a channel; ledcWrite talks to the channel",
  ], T.MARGIN, 3.9, T.CONTENT_W, 1.0, 14);

  // -------------------------------------------------------------------------
  // 14. Common anode
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Our LED counts backwards", 14);
  T.addCode(s, [
    "static const uint8_t PWM_MAX = 255U;",
    "",
    "// common anode: the pin sinks the current",
    "// 0 is full brightness, 255 is off",
    "ledcWrite(CHANNEL_RED, PWM_MAX - c.red);",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.0, 2.0, 12);
  T.addCallout(pres, s, "error", "The first thing that will go wrong",
    "Ask for red and get cyan, ask for black and get white. One subtraction, in one place " +
    "inside the class, and every caller is spared it.",
    T.MARGIN, 3.5, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 15. Section 4
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 4",
    title: "Sixteen Million Colours",
    note: "The class changes inside, the calls do not",
  });

  // -------------------------------------------------------------------------
  // 16. New insides
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Same class, new insides", 16);
  T.addCode(s, [
    "void RgbLed::setColour(Colour c) {",
    "    ledcWrite(CHANNEL_RED,   PWM_MAX - c.red);",
    "    ledcWrite(CHANNEL_GREEN, PWM_MAX - c.green);",
    "    ledcWrite(CHANNEL_BLUE,  PWM_MAX - c.blue);",
    "}",
    "",
    "led.setColour(warm);    // this line never changed",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 7.4, 2.6, 12);
  T.addCallout(pres, s, "ok", "That is what the class bought you",
    "digitalWrite became ledcWrite, eight colours became millions, and not one line " +
    "outside the class had to be touched.",
    T.MARGIN, 3.9, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 17. Mixing
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Mixing by numbers", 17);
  T.addTable(s,
    ["Colour", "red", "green", "blue"],
    [
      ["Red", "255", "0", "0"],
      ["Orange", "255", "120", "0"],
      ["Yellow", "255", "255", "0"],
      ["White", "255", "255", "255"],
      ["Dim white", "40", "40", "40"],
    ],
    T.MARGIN, 1.4, T.CONTENT_W, [3.0, 2.0, 2.0, 2.0]);
  T.addBullets(s, [
    "256 x 256 x 256 combinations, and no new wiring",
    "Green looks brighter than red at the same number: the eye, not a bug",
  ], T.MARGIN, 3.5, T.CONTENT_W, 1.2, 14);

  // -------------------------------------------------------------------------
  // 18. The wheel
  // -------------------------------------------------------------------------
  s = T.contentSlide(pres, "Tonight: the colour wheel", 18);
  T.addBullets(s, [
    "Walk the colours in a circle: red -> yellow -> green -> cyan -> blue -> magenta -> red",
    "One number decides where you are on that circle: the hue, 0 to 359",
    "The potentiometer sets that number, so the knob picks the colour",
    "Turning the knob a full turn walks the whole wheel",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.4);
  T.addCallout(pres, s, "info", "Everything you need, you now have",
    "A struct to carry a colour, a class to drive the LED, PWM to make the shades, " +
    "and the knob you already know how to read.",
    T.MARGIN, 3.8, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // 19. Summary
  // -------------------------------------------------------------------------
  T.summarySlide(pres, 19,
    [
      "A struct groups values; a class adds what can be done with them",
      "private keeps the inside of an object the owner's business",
      "The constructor remembers, begin() touches the hardware",
      "PWM turns a two-state pin into 256 levels of brightness",
    ],
    "Build the colour wheel: the knob picks the hue, the LED shows it. " +
    "Details are in homework/README.md.");

  await T.save(pres, "lesson08.pptx");
  console.log("OK: lesson08.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
