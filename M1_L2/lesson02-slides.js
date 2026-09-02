/*
 * lesson02-slides.js
 * Lesson 02 - First Firmware
 * PlatformIO and project anatomy | blink, button, serial | git workflow
 *
 * Build: node lesson02-slides.js  -> lesson02-first-firmware.pptx
 */

const T = require("../slides-theme");
const {
  FaMicrochip, FaTools, FaLightbulb, FaCodeBranch, FaFolderOpen,
  FaPlay, FaUpload, FaTerminal, FaGithub,
} = require("react-icons/fa");

(async () => {
  const pres = T.newDeck(
    "Lesson 02 - First Firmware",
    "Lesson 02  |  First Firmware"
  );

  const icChip = await T.icon(FaMicrochip, "#CADCFC");
  const icTools = await T.icon(FaTools, "#CADCFC");
  const icBulb = await T.icon(FaLightbulb, "#FFB300");
  const icBranch = await T.icon(FaCodeBranch, "#4CAF50");
  const icPlay = await T.icon(FaPlay, "#4CAF50");
  const icUpload = await T.icon(FaUpload, "#CADCFC");
  const icTerm = await T.icon(FaTerminal, "#FFB300");

  // -------------------------------------------------------------------------
  // 1. Title
  // -------------------------------------------------------------------------
  T.titleSlide(pres, {
    tag: "LESSON 02",
    title: "First Firmware",
    subtitle: "PlatformIO  |  Blink and button  |  git workflow",
    course: "Embedded Software Development in C/C++",
    meta: "Gamma Intelligence  |  ESP32-C3 Super Mini",
    iconData: icChip,
  });

  // -------------------------------------------------------------------------
  // 2. Today
  // -------------------------------------------------------------------------
  let s = T.contentSlide(pres, "Today", 2);
  T.addCards(pres, s, [
    { title: "Part 1", body: "VS Code + PlatformIO\nInstall the toolchain\nProject anatomy\nplatformio.ini", iconData: icTools },
    { title: "Part 2", body: "Build, upload, monitor\nBlink on GPIO 8\nSerial output\nRead the BOOT button", accent: T.AMBER, iconData: icBulb },
    { title: "Part 3", body: "git add, commit, push\nCommit messages\nWhat to do when\ngit refuses", accent: T.GREEN, iconData: icBranch },
  ], 1.35, 2.55);
  T.addCallout(pres, s, "info", "By the end of today",
    "Firmware you wrote is running on your board, and the source code is on GitHub.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.85);

  // -------------------------------------------------------------------------
  // PART 1
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 1",
    title: "VS Code and PlatformIO",
    note: "The tools you will use every single session",
  });

  // 4. What is what
  s = T.contentSlide(pres, "Who Does What", 4);
  T.addCards(pres, s, [
    { title: "VS Code", body: "The editor.\nWhere you read and write code.\nKnows nothing about microcontrollers by itself." },
    { title: "PlatformIO", body: "The build system.\nDownloads the compiler, builds the firmware, uploads it, opens the serial monitor.", accent: T.AMBER },
    { title: "Toolchain", body: "Compiler + linker + upload tool for a specific chip.\nInstalled per platform, automatically, on first build.", accent: T.GREEN },
  ], 1.35, 2.55);
  T.addCallout(pres, s, "info", "One environment for the whole course",
    "ESP32-C3, STM32 and RP2350 are all driven from the same PlatformIO setup.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.85);

  // 5. Installation
  s = T.contentSlide(pres, "Installing PlatformIO", 5);
  T.addBullets(s, [
    "Install VS Code from code.visualstudio.com",
    "Extensions (Ctrl+Shift+X) -> search \"PlatformIO IDE\" -> Install",
    "Wait for \"PlatformIO Core installed\" - the alien head icon appears in the sidebar",
    "Restart VS Code once if the icon does not appear",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.4);
  T.addCallout(pres, s, "warn", "The first build is slow - that is normal",
    "PlatformIO downloads the whole ESP32 toolchain (~1 GB) on the first build. On classroom WiFi this takes minutes. Start it, let it run, do not cancel.",
    T.MARGIN, 3.85, T.CONTENT_W, 1.1);

  // 6. Project anatomy
  s = T.contentSlide(pres, "Project Anatomy", 6);
  T.addCode(s, [
    "lesson-02/",
    "  platformio.ini    <- the project definition",
    "  src/",
    "    main.cpp        <- your code lives here",
    "  include/          <- your headers (.h)",
    "  lib/              <- project-local libraries",
    "  .pio/             <- build output. Generated.",
    "                       Never edit, never commit.",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.4, 2.4, 10);
  T.addBullets(s, [
    "One folder = one project = one platformio.ini",
    "PlatformIO: Home -> New Project, or clone one that already exists",
  ], 6.1, T.BODY_TOP + 0.1, 3.4, 2.0, 12);
  T.addCallout(pres, s, "info", "A project is text files",
    "Everything that matters is main.cpp plus platformio.ini. That is why the whole project goes into git and builds identically on any machine.",
    T.MARGIN, 3.85, T.CONTENT_W, 1.1);

  // 7. platformio.ini
  s = T.contentSlide(pres, "platformio.ini", 7);
  T.addCode(s, [
    "[env:lolin_c3_mini]",
    "platform = espressif32",
    "board = lolin_c3_mini",
    "framework = arduino",
    "monitor_speed = 115200",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.0, 1.75, 11);
  T.addBullets(s, [
    "platform: which chip family and toolchain",
    "board: our ESP32-C3 Super Mini",
    "framework: the API our code is written against",
    "monitor_speed: must match Serial.begin()",
  ], 5.7, T.BODY_TOP + 0.05, 3.8, 2.2, 12);
  T.addCallout(pres, s, "warn", "Five lines, three sources of silent failure",
    "Wrong board id -> firmware for the wrong chip. Wrong monitor_speed -> garbage in the serial monitor. Wrong framework -> nothing compiles.",
    T.MARGIN, 3.85, T.CONTENT_W, 1.1);

  // 8. Build, upload, monitor
  s = T.contentSlide(pres, "Build, Upload, Monitor", 8);
  T.addCards(pres, s, [
    { title: "Build", body: "Check mark in the status bar.\nCompiles and links.\nErrors appear here first.", iconData: icPlay },
    { title: "Upload", body: "Right arrow in the status bar.\nBuilds, then flashes over USB.", iconData: icUpload },
    { title: "Monitor", body: "Plug icon in the status bar.\nShows Serial output.\nExit: Ctrl+C in the terminal.", accent: T.AMBER, iconData: icTerm },
  ], 1.35, 2.55);
  T.addCallout(pres, s, "error", "Upload fails or the port is missing?",
    "1) Close the serial monitor - it holds the port. 2) Check the USB cable - charge-only cables have no data lines. 3) Try the other USB connector on the board.",
    T.MARGIN, 4.05, T.CONTENT_W, 0.95);

  // -------------------------------------------------------------------------
  // PART 2
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 2",
    title: "Blink, Serial, Button",
    note: "The smallest programs that prove the whole chain works",
  });

  // 10. Blink
  s = T.contentSlide(pres, "Blink", 10);
  T.addCode(s, [
    "#include <Arduino.h>",
    "",
    "static const uint8_t LED_PIN = 8U;",
    "static const uint32_t BLINK_MS = 500U;",
    "",
    "void setup() {",
    "    pinMode(LED_PIN, OUTPUT);",
    "}",
    "",
    "void loop() {",
    "    digitalWrite(LED_PIN, LOW);   // ON",
    "    delay(BLINK_MS);",
    "    digitalWrite(LED_PIN, HIGH);  // OFF",
    "    delay(BLINK_MS);",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.2, 3.85, 9);
  T.addBullets(s, [
    "setup() runs once at boot",
    "loop() runs forever after it",
    "LED_PIN and BLINK_MS: named constants at the top, no magic numbers below",
    "uint8_t / uint32_t: fixed-width types, always",
  ], 6.0, T.BODY_TOP + 0.15, 3.4, 3.4, 12);

  // 11. Active low
  s = T.contentSlide(pres, "Why LOW Means ON", 11);
  T.addBullets(s, [
    "The blue status LED on this board sits between 3.3V and GPIO 8",
    "The GPIO pin is on the cathode side and sinks current",
    "Pin LOW -> current flows -> LED on. Pin HIGH -> no current -> LED off",
    "This is called active-low, and it is everywhere in embedded: buttons, resets, chip-select lines",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.6);
  T.addCallout(pres, s, "warn", "GPIO 8 is also a strapping pin",
    "It must be HIGH at reset for normal boot - the board's own pull-up takes care of that. Later, when we hang external hardware on pins, GPIO 2, 8 and 9 are the ones to treat with care.",
    T.MARGIN, 3.95, T.CONTENT_W, 1.0);

  // 12. Serial
  s = T.contentSlide(pres, "Serial: Your First Debugging Tool", 12);
  T.addCode(s, [
    "void setup() {",
    "    Serial.begin(115200U);",
    "    pinMode(LED_PIN, OUTPUT);",
    "}",
    "",
    "void loop() {",
    "    digitalWrite(LED_PIN, LOW);",
    "    Serial.println(\"LED on\");",
    "    delay(BLINK_MS);",
    "    digitalWrite(LED_PIN, HIGH);",
    "    Serial.println(\"LED off\");",
    "    delay(BLINK_MS);",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.2, 3.6, 9);
  T.addBullets(s, [
    "115200 here = monitor_speed there",
    "println is how firmware talks back",
    "When something misbehaves, the first question is always: what does Serial say?",
  ], 6.0, T.BODY_TOP + 0.2, 3.4, 3.0, 12);

  // 13. Button
  s = T.contentSlide(pres, "Reading a Button", 13);
  T.addCode(s, [
    "static const uint8_t BUTTON_PIN = 9U;  // BOOT",
    "",
    "void setup() {",
    "    pinMode(BUTTON_PIN, INPUT_PULLUP);",
    "    pinMode(LED_PIN, OUTPUT);",
    "}",
    "",
    "void loop() {",
    "    if (digitalRead(BUTTON_PIN) == LOW) {",
    "        digitalWrite(LED_PIN, LOW);   // pressed",
    "    } else {",
    "        digitalWrite(LED_PIN, HIGH);  // released",
    "    }",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.2, 3.6, 9);
  T.addBullets(s, [
    "The board has two buttons: RESET restarts the chip, BOOT sits on GPIO 9",
    "After boot, BOOT is just a button: pressed connects the pin to ground, so pressed reads LOW - active-low again",
    "But GPIO 9 is a strapping pin: held LOW during reset it enters the serial bootloader. Do not hold it while resetting",
    "No delay() here: loop() polls the pin thousands of times per second",
  ], 6.0, T.BODY_TOP, 3.4, 3.7, 11);

  // -------------------------------------------------------------------------
  // PART 3
  // -------------------------------------------------------------------------
  T.sectionSlide(pres, {
    kicker: "PART 3",
    title: "The git Workflow",
    note: "How every assignment in this course is handed in",
  });

  // 15. add commit push
  s = T.contentSlide(pres, "add, commit, push", 15);
  T.addCode(s, [
    "cd embedded-cpp-2026-autumn",
    "git pull                      # start current",
    "",
    "# ... edit code ...",
    "",
    "git status                    # what changed?",
    "git add lesson-02/",
    "git commit -m \"lesson-02: blink with button\"",
    "git push",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.6, 2.7, 10);
  T.addBullets(s, [
    "pull first, push last",
    "Commit messages: English, ASCII, say what the change is",
    "Commit when something works, not once at midnight",
  ], 6.25, T.BODY_TOP + 0.15, 3.3, 2.5, 12);
  T.addCallout(pres, s, "warn", "Never commit .pio/",
    "Build output is generated, huge, and machine-specific. The .gitignore in the course repository already excludes it - do not force-add it.",
    T.MARGIN, 4.05, T.CONTENT_W, 0.9);

  // 16. When git refuses
  s = T.contentSlide(pres, "When git Refuses", 16);
  T.addTable(s,
    ["It says", "It means", "You do"],
    [
      ["rejected: fetch first", "GitHub has commits you do not", "git pull, then push again"],
      ["merge conflict", "Same lines changed twice", "Edit the marked file, add, commit"],
      ["Permission denied (publickey)", "SSH key problem from lesson 01", "ssh -T git@github.com to test"],
      ["not a git repository", "You are in the wrong folder", "cd into the cloned repository"],
    ],
    T.MARGIN, T.BODY_TOP + 0.1, T.CONTENT_W, [2.7, 3.15, 3.15], 11);
  T.addCallout(pres, s, "info", "Read the message before asking",
    "git error messages usually contain the fix. Reading them is a course skill, same as reading a datasheet.",
    T.MARGIN, 4.05, T.CONTENT_W, 0.9);

  // 17. Summary
  T.summarySlide(pres, 17, [
    "PlatformIO turns VS Code into the whole embedded toolchain: build, upload, monitor",
    "A project is platformio.ini plus src/ - everything else is generated",
    "Blink proves the chain: compiler -> USB -> flash -> running code",
    "The blue LED (GPIO 8) and the BOOT button (GPIO 9) are both active-low",
    "Work flows through git: pull, edit, add, commit, push",
  ],
  "Make the LED blink a morse-style pattern and control it with the button - scaffold in lesson-02/homework. Commit and push before the next session.");

  await T.save(pres, "lesson02-first-firmware.pptx");
  console.log("OK: lesson02-first-firmware.pptx written");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
