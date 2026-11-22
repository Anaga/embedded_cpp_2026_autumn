// Lesson 24 - From Parts to a Device, and getting ready for demo day
// Build: node lesson24-slides.js  ->  lesson24.pptx

const T = require("./slides-theme");
const { FaPuzzlePiece } = require("react-icons/fa");

const { MARGIN, CONTENT_W, BODY_TOP } = T;

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "speaker notes"));
}

(async () => {
  const pres = T.newDeck(
    "Lesson 24 - From Parts to a Device",
    "Lesson 24  |  Z2: integration and demo day"
  );
  const puzzle = await T.icon(FaPuzzlePiece, "#CADCFC");
  let n = 1;
  let s;

  // 1 ------------------------------------------------------------------
  s = T.titleSlide(pres, {
    tag: "LESSON 24",
    title: "From Parts to a Device",
    subtitle: "One loop, many parts - and getting ready for demo day",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3  |  millis()  |  FreeRTOS task  |  NVS  |  Demo day",
    iconData: puzzle,
  });
  notes(s, "Session 24, Mon 2026-11-23. 180 min.\n" +
    "0-18 integration: the glued demo, measuring loop(), parts as classes, the network task. " +
    "18-22 several WiFi networks in NVS. 22-32 demo day format and checklist. " +
    "32-175 project work, instructor goes from student to student. 175-180 commit.\n" +
    "Before class: flash env 'after' once and type 'wifi add' for the class router, so env 'before' " +
    "finds the network too (both read the same NVS list).");

  // 2 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Today", ++n);
  T.addCards(pres, s, [
    { title: "Part 1  (30 min)", body: "Parts -> one device.\nMany WiFi networks.\nDemo day: format and checklist.", accent: T.ICE },
    { title: "Part 2  (145 min)", body: "Your final project.\nI come to each of you.\nGoal: the minimum version works.", accent: T.GREEN },
  ], 1.35, 1.9);
  T.addTable(s,
    ["Session", "Date", "Goal"],
    [
      ["23", "Fri 2026-11-20", "skeleton builds, hardware wired, first commit"],
      ["24", "Mon 2026-11-23", "minimum version works - end to end"],
      ["25", "Fri 2026-11-27", "demo day: 10 min demo + 5 min questions each"],
    ],
    MARGIN, 3.6, CONTENT_W, [1.2, 2.2, 5.6], 12);
  notes(s, "Ask round the table first, 1 sentence each: which parts of your project already work alone? " +
    "Remember the answers - the integration part is about exactly that.");

  // 3 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Every Part Worked Alone. Together...", ++n);
  T.addCards(pres, s, [
    { title: "Button", body: "Press it - nothing. Press again - now it works. Some presses are lost.", accent: T.RED },
    { title: "LED", body: "The blink stutters and stops for a second or two.", accent: T.AMBER },
    { title: "Bot", body: "Answers - but only after the sensor, the delay and the blink.", accent: T.AMBER },
    { title: "WiFi gone", body: "Router off: the whole board freezes, the button too.", accent: T.RED },
  ], BODY_TOP + 0.1, 2.35);
  T.addCallout(pres, s, "warn", "One cause",
    "Every part WAITS for something. While one part waits, all the others wait with it - there is only one loop().",
    MARGIN, 3.9, CONTENT_W, 0.9);
  notes(s, "Live: flash env 'before' (pio run -e before -t upload). Blue LED should blink 4 times a second - " +
    "it stutters. Press button A several times quickly: green toggles only sometimes. Send /status: slow. " +
    "Then switch the class router off for 20 s: everything stops. Do not explain yet - ask why.");

  // 4 ------------------------------------------------------------------
  s = T.contentSlide(pres, "The Glued Version - Spot the Waits", ++n);
  T.addCode(s, [
    "void loop() {",
    "  while (wifiMulti.run() != WL_CONNECTED) delay(500);",
    "  aht.getEvent(&h, &t);",
    "  int n = bot.getUpdates(bot.last_message_received + 1);",
    "  for (int i = 0; i < n; i++) handle(bot.messages[i]);",
    "  if (digitalRead(PIN_BUTTON_A) == LOW) {",
    "    toggleLed();",
    "    delay(200);                       // \"debounce\"",
    "  }",
    "  digitalWrite(PIN_LED_B, LOW);  delay(125);",
    "  digitalWrite(PIN_LED_B, HIGH); delay(125);",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 6.0, 3.9, 10.5);
  T.addBullets(s, [
    "How many waits can you find?",
    "Which one is the longest?",
    "Which ones can we remove - and which one can we NOT?",
  ], 6.75, BODY_TOP + 0.1, 2.75, 3.0, 13);
  notes(s, "This is demo/src/before/main.cpp, shortened. Answers: six waits.\n" +
    "1 while + delay(500): forever if the router is off. 2 getEvent: about 80 ms (the AHT10 conversion, session 15). " +
    "3 getUpdates: 0.3-2 s, HTTPS round trip. 4 delay(200) after the button. 5-6 two delay(125) for the blink.\n" +
    "Removable: 1, 4, 5, 6 with millis() and state - we know how since K3. getEvent: acceptable at 80 ms, or by hand " +
    "(session 13 lab). The bot CANNOT be made non-blocking from outside the library - that needs a task.");

  // 5 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Measure Before You Fix", ++n);
  T.addCode(s, [
    "void LoopStats::update(uint32_t now) {",
    "  uint32_t us   = micros();",
    "  uint32_t pass = us - lastUs_;   // one loop() pass",
    "  lastUs_ = us;",
    "  if (pass > maxUs_) maxUs_ = pass;",
    "  passes_++;",
    "  if (now - lastReport_ >= STATS_PERIOD_MS) {",
    "    report(now);                  // print, then reset",
    "  }",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 5.5, 3.05, 10.5);
  T.addTable(s,
    ["Serial", "before", "after"],
    [
      ["passes / s", "0.4", "80000"],
      ["longest pass", "2.3-2.8 s", "4-8 ms"],
    ],
    6.2, BODY_TOP + 0.05, 3.3, [1.3, 1.0, 1.0], 11);
  T.addBullets(s, [
    "Call it last in loop()",
    "The longest pass decides: a button press shorter than that can be lost",
  ], 6.2, BODY_TOP + 1.3, 3.3, 1.8, 12);
  T.addCallout(pres, s, "info", "Same idea as session 11",
    "micros() timestamps, min / max printed once in a while. Measure first - then you know which part to fix.",
    MARGIN, 4.3, CONTENT_W, 0.75);
  notes(s, "Show the real numbers from the before demo on Serial. Write them on the board, compare after the fix. " +
    "'after' numbers measured 2026-11-22 on a bare C3 with WiFi and the bot running (115 000 passes/s without WiFi). " +
    "One pass of 343 ms showed up while WiFi connected: the WiFi driver tasks run at a higher priority than loop(). " +
    "Good question for the group: why does a task not protect us from that? 'before' measured the same day: 0.4 passes/s, longest pass 2.3-2.8 s. " +
    "'before' has no serial console - typing 'help' there gets no answer, which is itself a symptom.");

  // 6 ------------------------------------------------------------------
  s = T.contentSlide(pres, "The Rule: A Part Never Waits", ++n);
  T.addTable(s,
    ["Instead of waiting ...", "... check and return"],
    [
      ["delay(125) for a blink", "millis(): is it time to toggle? (K3)"],
      ["delay(200) after a button", "debounce state: 30 ms of quiet (K3 Button)"],
      ["while (!connected) delay(500)", "connected? no -> try again later, return"],
      ["read the sensor every pass", "every 2 s, on its own millis() timer"],
      ["bot.getUpdates(): 0.3-2 s", "cannot be split -> its own task (slide 10)"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [4.0, 5.0], 13);
  T.addCallout(pres, s, "ok", "Every part, every pass",
    "Look at the clock, do a tiny piece of work if it is time, return. loop() runs thousands of times a second.",
    MARGIN, 3.65, CONTENT_W, 0.9);
  notes(s, "Nothing new here - enum class state machines and millis() timers are from K3 and session 12. " +
    "The new thing is doing it for EVERY part at the same time.");

  // 7 ------------------------------------------------------------------
  s = T.contentSlide(pres, "One Part = One Class", ++n);
  T.addCode(s, [
    "class Heartbeat {",
    " public:",
    "  explicit Heartbeat(RgbLed &led) : led_(led) {}",
    "  void begin() { last_ = millis(); }",
    "  void update(uint32_t now) {",
    "    if (now - last_ < HEARTBEAT_HALF_MS) return;",
    "    last_ = now;  on_ = !on_;",
    "    led_.set(RgbLed::Channel::BLUE, on_);",
    "  }",
    " private:",
    "  RgbLed  &led_;",
    "  uint32_t last_ = 0U;",
    "  bool     on_   = false;",
    "};",
  ].join("\n"), MARGIN, BODY_TOP, 5.6, 3.85, 9.5);
  T.addBullets(s, [
    "begin(): once, from setup()",
    "update(now): every pass, from loop(), returns at once",
    "The part keeps its own state and timer - no globals",
    "One .h file per part, as in K3 (RgbLed, Button)",
    "Every part of your project gets the same two methods",
  ], 6.35, BODY_TOP, 3.15, 3.9, 12);
  notes(s, "The blue heartbeat is our 'is the loop alive' indicator: if it stutters, some part waits. " +
    "If a project in the room already has one class per part, show it as the example.");

  // 8 ------------------------------------------------------------------
  s = T.contentSlide(pres, "loop() Becomes a List", ++n);
  T.addCode(s, [
    "void setup() {",
    "  rgb.begin();  heartbeat.begin();  button.begin();",
    "  sensor.begin();  console.begin();",
    "  net::begin();         // WiFi + bot: own task",
    "}",
    "void loop() {",
    "  uint32_t now = millis();",
    "  button.update(now);",
    "  if (button.pressed()) ledToggle();",
    "  ledCommands();        // from the bot task",
    "  sensor.update(now);",
    "  heartbeat.update(now);  console.update();",
    "  publishSnapshot();    // for the bot task",
    "  stats.update(now);",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 5.3, 3.85, 9);
  T.addBullets(s, [
    "Reads like a table of contents",
    "Adding a part = one object, one begin(), one update()",
    "Order does not matter much: every call is short",
    "Glue (button -> LED) stays visible in loop()",
  ], 6.1, BODY_TOP, 3.4, 3.9, 13);
  notes(s, "This is demo/src/after/main.cpp, shortened: Serial.begin, shared::begin(), stats.begin() and the chip temperature part left out. " +
    "Live: flash env 'after'. Heartbeat is smooth, every press toggles green, /status answers within a second. " +
    "Switch the router off: the heartbeat and the button keep going.");

  // 9 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Every Part Has Its Own Rhythm", ++n);
  T.addTable(s,
    ["Part", "How often", "Who runs it"],
    [
      ["Button A (GPIO 0)", "every pass, 30 ms debounce", "loop()"],
      ["Heartbeat, blue LED", "toggle every 125 ms", "loop()"],
      ["AHT10", "every 2 s, retry every 5 s if missing", "loop()"],
      ["Chip temperature", "every 2 s, temperatureRead()", "loop()"],
      ["Serial console", "every pass, one line at a time", "loop()"],
      ["Loop statistics", "print every 5 s", "loop()"],
      ["WiFi (re)connect", "when disconnected: seconds", "net task"],
      ["Telegram poll", "every 1 s: 0.3-2 s each", "net task"],
    ],
    MARGIN, BODY_TOP, CONTENT_W, [3.0, 4.0, 2.0], 11);
  T.addCallout(pres, s, "info", "Write this table for your own project",
    "Part, period, who runs it. Anything that takes more than a few ms and cannot be cut into pieces goes into the last rows.",
    MARGIN, 4.38, CONTENT_W, 0.67);
  notes(s, "Ask each student to make this table for their project during the project part - it is 5 minutes " +
    "and it is also a good slide for Friday.");

  // 10 -----------------------------------------------------------------
  s = T.contentSlide(pres, "The Part That Must Wait: Its Own Task", ++n);
  T.addCode(s, [
    "static void netTask(void *) {",
    "  for (;;) {",
    "    if (WiFi.status() != WL_CONNECTED) {",
    "      wifiMulti.run(WIFI_CONNECT_MS);  // seconds",
    "    } else if (millis() - lastPoll >= BOT_POLL_MS) {",
    "      pollBot();                       // 0.3-2 s",
    "      lastPoll = millis();",
    "    }",
    "    vTaskDelay(pdMS_TO_TICKS(NET_IDLE_MS));",
    "  }",
    "}",
    "// net::begin(), called from setup():",
    "xTaskCreate(netTask, \"net\", NET_STACK_BYTES, nullptr, 1, nullptr);",
  ].join("\n"), MARGIN, BODY_TOP, 5.9, 3.95, 10);
  T.addBullets(s, [
    "loop() is already a FreeRTOS task (\"loopTask\") - we add a second one",
    "The C3 has one core: the scheduler switches between tasks every 1 ms",
    "While the net task waits for the network, loop() runs",
    "Inside a task delay() is fine: it waits only for THIS task",
  ], 6.6, BODY_TOP, 2.9, 3.95, 12);
  notes(s, "Keep it short - this is not an RTOS lesson. The point: a library that blocks (bot, WiFiMulti, " +
    "HTTPClient) goes into a task; everything else stays in loop(). Stack 10 KB because of TLS. " +
    "Priority 1 = same as loopTask, so they share the CPU.");

  // 11 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Two Tasks, One Board: Who Owns What", ++n);
  T.addCode(s, [
    "// net task -> loop(): a queue of commands",
    "LedCmd cmd = LedCmd::ON;",
    "xQueueSend(ledQueue, &cmd, 0);              // bot",
    "while (xQueueReceive(ledQueue, &cmd, 0) == pdTRUE) {",
    "  ledApply(cmd);                            // loop()",
    "}",
    "",
    "// loop() -> net task: a copy under a lock",
    "portENTER_CRITICAL(&mux);",
    "snapshot = current;        // a few bytes",
    "portEXIT_CRITICAL(&mux);",
  ].join("\n"), MARGIN, BODY_TOP, 5.9, 3.2, 10);
  T.addBullets(s, [
    "One owner per thing: loop() owns the pins, the net task owns WiFi and the bot",
    "Never call ledSet() from the bot - send a command",
    "The critical section from session 11, now between two tasks",
  ], 6.6, BODY_TOP, 2.9, 3.3, 12);
  T.addCallout(pres, s, "error", "Two tasks writing the same variable",
    "Works for days, then fails once on demo day. Queue or lock - nothing in between.",
    MARGIN, 4.45, CONTENT_W, 0.6);
  notes(s, "The snapshot holds temperature, humidity, sensor ok and LED state - what /status needs. " +
    "Same discipline as the ISR in session 11: the other side only gets a copy, taken under a lock.");

  // 12 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Integrate One Part at a Time", ++n);
  T.addBullets(s, [
    "1. Empty loop() with LoopStats and the heartbeat - builds, runs",
    "2. Add ONE part: object, begin(), update()",
    "3. Build, flash, test that part AND the heartbeat",
    "4. Check the longest pass on Serial - did it jump?",
    "5. Commit: \"add AHT10 to loop\"",
    "6. Next part. Broken? git diff against the last commit",
  ], MARGIN, BODY_TOP, 5.6, 2.9, 14);
  T.addCallout(pres, s, "ok", "Why one at a time",
    "Five parts added at once and nothing works: five suspects. One at a time: one suspect - the last one.",
    MARGIN, 4.15, CONTENT_W, 0.8);
  T.addCallout(pres, s, "warn", "Small commits",
    "git is your undo button. A commit after every part that works.",
    6.4, BODY_TOP + 0.05, 3.1, 1.2);
  notes(s, "This is the plan for the next two hours: everybody starts from step 1 with their own parts.");

  // 13 -----------------------------------------------------------------
  s = T.contentSlide(pres, "One Board, Many Networks", ++n);
  T.addCode(s, [
    "// Serial: wifi add \"Home Net\" secret123",
    "Preferences prefs;",
    "prefs.begin(\"wifi\", false);    // NVS namespace",
    "prefs.putString(\"s1\", ssid);",
    "prefs.putString(\"p1\", pass);",
    "prefs.putUChar(\"n\", 2);",
    "prefs.end();",
    "",
    "// at boot: every saved network -> WiFiMulti",
    "for (uint8_t i = 0; i < count; i++) {",
    "  wifiMulti.addAP(nets[i].ssid, nets[i].pass);",
    "}",
    "wifiMulti.run();   // scan, take the STRONGEST known",
  ].join("\n"), MARGIN, BODY_TOP, 5.6, 3.7, 10);
  T.addBullets(s, [
    "NVS: key-value store in flash, survives power off AND re-flashing",
    "At home: wifi add once. In class: wifi add once. Demo day: just works",
    "wifi list, wifi del 2, wifi clear - no password in the code, none in git",
    "Not encrypted: whoever has the board can read it out",
  ], 6.3, BODY_TOP, 3.2, 3.9, 12);
  notes(s, "Live on the instructor board: 'wifi list', then 'wifi add' with the phone hotspot, 'reboot', " +
    "switch the class router off - the board moves to the hotspot. " +
    "Up to 5 networks, SSID up to 31 characters (WiFiMulti limit), quotes for names with spaces. " +
    "Copy src/common/wifi_store.* and the console into your project - it is 2 files.");

  // 14 -----------------------------------------------------------------
  s = T.sectionSlide(pres, { kicker: "FRIDAY 2026-11-27", title: "Demo Day", note: "10 minutes demo + 5 minutes questions, each of you" });
  n++;
  notes(s, "Order of presentations: drawn by lot on Friday, or volunteers first.");

  // 15 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Your 15 Minutes", ++n);
  T.addTable(s,
    ["Minute", "What", "Tip"],
    [
      ["0 - 1", "The idea: what problem, for whom", "one sentence, no slides needed"],
      ["1 - 3", "Hardware: board, parts, wiring", "show the pin table from your README"],
      ["3 - 7", "Live demo", "the main use case first, extras after"],
      ["7 - 9", "Code: one part you are proud of", "one class, not the whole main.cpp"],
      ["9 - 10", "What went wrong, what is next", "a real bug and how you found it"],
      ["10 - 15", "Questions", "\"I do not know yet\" is a fine answer"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [1.2, 3.9, 3.9], 12);
  T.addCallout(pres, s, "info", "Questions you can expect",
    "What happens if the sensor is unplugged? How long is your longest loop pass? Which part took the longest to get working?",
    MARGIN, 3.9, CONTENT_W, 0.9);
  notes(s, "Keep the timing: show a 1-minute warning at 9 minutes. " +
    "The 'what went wrong' part counts as much as the demo - the debugging thread of the course.");

  // 16 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Make the Demo Survive", ++n);
  T.addBullets(s, [
    "Cold start: unplug, plug into a power bank - works without the laptop?",
    "Class network saved in NVS (wifi add) and tested in this room",
    "Unplug the sensor during the demo: a message, no freeze",
    "Record a 30 s video when it works: plan B if the hardware dies",
    "Code freeze Thursday evening - no new features on Friday",
    "Bring: board, USB cable, power bank, spare jumper wires",
  ], MARGIN, BODY_TOP, 5.8, 2.95, 13);
  T.addCallout(pres, s, "warn", "Demo effect",
    "It worked yesterday. Today: new room, other WiFi, cold start, nervous hands.",
    MARGIN, 4.15, CONTENT_W, 0.8);
  T.addCallout(pres, s, "ok", "Rehearse",
    "Run the whole 10 minutes once at home, with a timer, from power-off.",
    6.6, BODY_TOP + 0.05, 2.9, 1.3);
  notes(s, "The cold start is the one people forget: Serial.begin plus 'while (!Serial)' blocks forever on a power bank. " +
    "Check that nobody has 'while (!Serial)' in setup().");

  // 17 -----------------------------------------------------------------
  s = T.contentSlide(pres, "The Project README", ++n);
  T.addCode(s, [
    "# Room Climate Monitor",
    "One sentence: what it does and for whom.",
    "",
    "## Hardware",
    "| Part    | Pin         | Notes                   |",
    "|---------|-------------|-------------------------|",
    "| AHT10   | GPIO 8 / 9  | I2C, strapping pins     |",
    "## How to build and run",
    "secrets.h, wifi add, pio run -t upload",
    "## How it works",
    "Parts, periods, who runs what (slide 9).",
    "## Known problems and next steps",
    "Honest list.",
  ].join("\n"), MARGIN, BODY_TOP, 5.6, 3.7, 10);
  T.addBullets(s, [
    "In English, in projects/<name>/README.md",
    "A photo of the device, if you can",
    "C3: say which strapping pins (2, 8, 9) you use and why it is safe",
    "No passwords, no tokens - secrets.h stays in .gitignore",
  ], 6.3, BODY_TOP, 3.2, 3.9, 12);
  notes(s, "The README is part of the grade and it is what remains on GitHub after the course. " +
    "Check during the project part that every README exists and is in English.");

  // 18 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Today in Your Project", ++n);
  T.addBullets(s, [
    "Write the rhythm table: part, period, who runs it",
    "LoopStats in your loop(): what is your longest pass?",
    "Add parts one at a time, commit after each",
    "wifi add for the class network, test a cold start",
    "Push before you leave: the minimum version works",
  ], MARGIN, BODY_TOP, 5.6, 3.3, 14);
  T.addCallout(pres, s, "ok", "Stuck?",
    "Symptom -> hypothesis -> test -> conclusion. Then call me and tell me what you already tried.",
    6.3, BODY_TOP + 0.05, 3.2, 1.35);
  T.addCallout(pres, s, "info", "Feature ideas?",
    "Write them under \"next steps\" in the README. Today and Friday: make what you have reliable.",
    6.3, BODY_TOP + 1.6, 3.2, 1.35);
  notes(s, "Order of visits: whoever has a blocker first, then everyone in turn. " +
    "Check the longest loop pass with each student.");

  // 19 -----------------------------------------------------------------
  T.summarySlide(pres, ++n, [
    "A part never waits: check the clock, do a little, return",
    "One part = one class with begin() and update(now); loop() is the list",
    "Measure the longest loop pass - it tells you which part waits",
    "What cannot stop waiting goes into its own task; share by queue or lock",
    "Preferences (NVS) + WiFiMulti: several networks, the strongest wins",
  ], "Before Friday: minimum version works from a cold start, README done, code frozen and pushed by Thursday evening.");

  await T.save(pres, "lesson24.pptx");
  console.log("OK: lesson24.pptx, " + n + " slides");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
