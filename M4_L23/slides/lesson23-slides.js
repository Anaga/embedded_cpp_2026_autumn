// Lesson 23 - Telegram Bots on the ESP32-C3, then final project work
// Build: node lesson23-slides.js  ->  lesson23.pptx

const T = require("./slides-theme");
const { FaTelegramPlane, FaMobileAlt, FaServer, FaMicrochip } = require("react-icons/fa");

const { MARGIN, CONTENT_W, BODY_TOP } = T;

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "speaker notes"));
}

function label(slide, text, x, y, w, h, o) {
  const opt = Object.assign({
    x, y, w, h, fontSize: 12, fontFace: T.BODY, color: T.ICE, margin: 0,
    align: "center", valign: "middle",
  }, o || {});
  slide.addText(T.assertAscii(text, "label"), opt);
}

function box(pres, slide, x, y, w, h, line) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h, fill: { color: T.CARD_BG }, line: { color: line || T.MID, width: 1.5 },
  });
}

(async () => {
  const pres = T.newDeck(
    "Lesson 23 - Telegram Bots on the ESP32-C3",
    "Lesson 23  |  Z2: Telegram bots and final project work"
  );
  const tg = await T.icon(FaTelegramPlane, "#CADCFC");
  const phone = await T.icon(FaMobileAlt, "#CADCFC");
  const server = await T.icon(FaServer, "#FFB300");
  const chip = await T.icon(FaMicrochip, "#4CAF50");
  let n = 1;
  let s;

  // 1 ------------------------------------------------------------------
  s = T.titleSlide(pres, {
    tag: "LESSON 23",
    title: "Your Board in Your Pocket",
    subtitle: "Telegram bots on the ESP32-C3, then final project work",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3  |  HTTPS  |  Telegram Bot API  |  Final project",
    iconData: tg,
  });
  notes(s, "Session 23, Fri 2026-11-20. 180 min.\n" +
    "0-30 Telegram bot: theory and live demo on the instructor C3 (bot already created, secrets.h filled in BEFORE class). " +
    "30-175 final project work, instructor goes from student to student. 175-180 commit and plan for Monday.\n" +
    "Students who want a bot in their project create it in BotFather during the project part, not during the theory.");

  // 2 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Today", ++n);
  T.addCards(pres, s, [
    { title: "Part 1  (30 min)", body: "Telegram bot on the C3.\nBotFather, token, two API calls.\nLive demo: /status, /led, alert.", accent: T.ICE },
    { title: "Part 2  (145 min)", body: "Your final project.\nI come to each of you.\nQuestions, wiring, code, bugs.", accent: T.GREEN },
  ], 1.35, 1.9);
  T.addTable(s,
    ["Session", "Date", "Goal"],
    [
      ["23", "Fri 2026-11-20", "skeleton builds, hardware wired, first commit"],
      ["24", "Mon 2026-11-23", "minimum version works"],
      ["25", "Fri 2026-11-27", "demo day"],
    ],
    MARGIN, 3.6, CONTENT_W, [1.2, 2.2, 5.6], 12);
  notes(s, "The bot is an option, not a requirement. It fits projects that should report something " +
    "or be controlled from far away: watering done or failed, room too hot, a remote switch.");

  // 3 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Three Ways to Reach Your Board", ++n);
  T.addTable(s,
    ["", "Web server (18)", "MQTT (20)", "Telegram bot (23)"],
    [
      ["Works from", "same WiFi only", "anywhere", "anywhere"],
      ["Phone side", "browser + IP", "MQTT app", "Telegram app"],
      ["Needs", "nothing", "a broker", "a bot token"],
      ["Who connects", "phone -> board", "both -> broker", "board -> Telegram"],
      ["Board can talk first", "no", "yes", "yes"],
      ["Speed", "instant", "instant", "about 1 s"],
    ],
    MARGIN, BODY_TOP + 0.05, CONTENT_W, [2.1, 2.3, 2.3, 2.3], 12);
  T.addCallout(pres, s, "info", "Why it works behind any router",
    "The board only opens OUTGOING HTTPS connections, like a browser. No open port, no port forwarding, no public IP.",
    MARGIN, 3.85, CONTENT_W, 0.85);
  notes(s, "Ask first: you are at the airport and want to know if the plants were watered. " +
    "Which of our three can do that? Web server: no (the phone is not on the home WiFi). " +
    "MQTT: yes, with a public broker and an app. Telegram: yes, with the app everybody already has.");

  // 4 ------------------------------------------------------------------
  s = T.contentSlide(pres, "How a Bot Works", ++n);
  const by = 1.35, bh = 1.55, bw = 2.1;
  const xs = [0.6, 3.95, 7.3];
  const titles = ["Your phone", "api.telegram.org", "ESP32-C3"];
  const subs = ["Telegram app,\nyou type /status", "keeps messages\nfor your bot", "asks every 1 s:\n\"anything new?\""];
  const icons = [phone, server, chip];
  const lines = [T.ICE, T.AMBER, T.GREEN];
  for (let i = 0; i < 3; i++) {
    box(pres, s, xs[i], by, bw, bh, lines[i]);
    s.addImage({ data: icons[i], x: xs[i] + bw / 2 - 0.22, y: by + 0.12, w: 0.44, h: 0.44 });
    label(s, titles[i], xs[i], by + 0.6, bw, 0.3, { fontSize: 14, bold: true, color: T.WHITE, fontFace: T.HEADING });
    label(s, subs[i], xs[i], by + 0.92, bw, 0.55, { fontSize: 11 });
  }
  label(s, "message ->", 2.7, by + 0.35, 1.25, 0.3, { fontSize: 11, color: T.AMBER });
  label(s, "<- reply", 2.7, by + 0.95, 1.25, 0.3, { fontSize: 11, color: T.AMBER });
  label(s, "<- getUpdates", 6.05, by + 0.35, 1.25, 0.3, { fontSize: 11, color: T.GREEN });
  label(s, "sendMessage ->", 6.05, by + 0.95, 1.25, 0.3, { fontSize: 11, color: T.GREEN });
  T.addBullets(s, [
    "Long polling: the board asks, Telegram answers with the new messages (or nothing)",
    "Every request is HTTPS to api.telegram.org: GET or POST, JSON back",
    "Webhook (Telegram calls YOU) needs a public HTTPS server - not for a microcontroller",
  ], MARGIN, 3.15, CONTENT_W, 1.6, 13);
  notes(s, "The board is a client here, not a server - the opposite of session 18. " +
    "That is why it works from a hotspot, a home router or the classroom router without any setup.");

  // 5 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Step 1: Create the Bot with @BotFather", ++n);
  T.addBullets(s, [
    "In Telegram open @BotFather (blue check mark), send /newbot",
    "Name: anything, e.g. \"Anton C3\"",
    "Username: unique, must end in \"bot\", e.g. anton_c3_2026_bot",
    "BotFather answers with the token: 123456789:AAH...",
    "Check it in any browser: api.telegram.org/bot<TOKEN>/getMe",
  ], MARGIN, BODY_TOP, 5.6, 2.7, 14);
  T.addCode(s, [
    "// include/secrets.h",
    "// (in .gitignore, never committed)",
    "",
    "WIFI_SSID     = \"...\";",
    "WIFI_PASS     = \"...\";",
    "BOT_TOKEN     = \"123456789:AAH...\";",
    "ALERT_CHAT_ID = \"\";",
  ].join("\n"), 6.2, BODY_TOP + 0.05, 3.3, 2.25, 10.5);
  T.addCallout(pres, s, "error", "The token IS your bot",
    "Whoever has it can read and send as your bot. Leaked into git? Send /revoke to @BotFather - the old token dies at once.",
    MARGIN, 3.65, CONTENT_W, 0.95);
  notes(s, "Demo the getMe URL on the projector with the instructor bot, then cover the token. " +
    "Remind: session 18 router password ended up in git history (open decision 14). Same rule for the token: " +
    "secrets.h is listed in the demo .gitignore.");

  // 6 ------------------------------------------------------------------
  s = T.contentSlide(pres, "The Bot API in Two Calls", ++n);
  T.addCode(s, [
    "GET /bot<TOKEN>/getUpdates?offset=715",
    "",
    "{\"ok\":true,\"result\":[{",
    "  \"update_id\":715,",
    "  \"message\":{",
    "    \"chat\":{\"id\":123456789},",
    "    \"from\":{\"first_name\":\"Anton\"},",
    "    \"text\":\"/status\"}}]}",
    "",
    "GET /bot<TOKEN>/sendMessage",
    "    ?chat_id=123456789&text=22.4 C",
  ].join("\n"), MARGIN, BODY_TOP, 5.3, 3.5, 11);
  T.addBullets(s, [
    "chat.id: who to answer - and where to send alerts later",
    "update_id: a counter for every update",
    "offset = last update_id + 1 means \"I have seen everything up to here\"",
    "Without the offset the same message comes back on every poll",
  ], 6.0, BODY_TOP, 3.5, 3.5, 13);
  notes(s, "JSON shortened - the real answer has date, message_id, username and more. " +
    "Same JSON parsing idea as /status in session 18. The library does exactly these two calls for us.");

  // 7 ------------------------------------------------------------------
  s = T.contentSlide(pres, "UniversalTelegramBot on the C3", ++n);
  T.addCode(s, [
    "WiFiClientSecure     tls;",
    "UniversalTelegramBot bot(BOT_TOKEN, tls);",
    "",
    "// setup()",
    "tls.setInsecure();   // encrypted, server not checked",
    "",
    "// loop(), once per second",
    "int n = bot.getUpdates(bot.last_message_received + 1);",
    "while (n > 0) {",
    "  for (int i = 0; i < n; i++) handleMessage(bot.messages[i]);",
    "  n = bot.getUpdates(bot.last_message_received + 1);",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 6.0, 3.45, 10);
  T.addBullets(s, [
    "#include <WiFiClientSecure.h> and <UniversalTelegramBot.h>",
    "lib_deps: UniversalTelegramBot, ArduinoJson 6",
    "Core 2.x, as since session 18",
    "messages[i].chat_id, .text, .from_name",
    "Polling every 1 s: millis(), no delay()",
    "/led = green RGB channel, GPIO 6 (GPIO 8 is I2C SDA)",
  ], 6.7, BODY_TOP, 2.8, 3.6, 12);
  notes(s, "ArduinoJson 6 on purpose: the library uses DynamicJsonDocument and containsKey, which are " +
    "deprecated in 7. It builds with 7 too, but with a page of warnings.");

  // 8 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Handling a Command", ++n);
  T.addCode(s, [
    "void handleMessage(const telegramMessage &m) {",
    "  String cmd, arg;",
    "  splitCommand(m.text, cmd, arg);  // \"/led on\"",
    "",
    "  if (cmd == \"/status\") {",
    "    bot.sendMessage(m.chat_id, statusText(), \"\");",
    "  } else if (cmd == \"/led\" && arg == \"on\") {",
    "    ledSet(true);",
    "    bot.sendMessage(m.chat_id, \"LED on\", \"\");",
    "  } else {",
    "    bot.sendMessage(m.chat_id, \"Try /help\", \"\");",
    "  }",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 6.0, 3.7, 10);
  T.addBullets(s, [
    "A command is just text that starts with /",
    "Always answer - even \"unknown command\"",
    "/start: help text plus a keyboard with buttons",
    "Serial prints the chat id of every message",
  ], 6.7, BODY_TOP, 2.8, 3.4, 12);
  notes(s, "Live: send /status, /led on, /led off and something random. /led is the GREEN RGB channel on GPIO 6 - the blue on-board LED is GPIO 8 = SDA and only flickers with I2C. Show the Serial log with the chat id. " +
    "In a group chat the command arrives as /status@my_bot - splitCommand() cuts the @ part.");

  // 9 ------------------------------------------------------------------
  s = T.contentSlide(pres, "Push Alert: Once per Crossing", ++n);
  T.addCode(s, [
    "switch (tempState) {",
    "  case TempState::NORMAL:",
    "    if (t >= alertC) {",
    "      if (send(chat, \"ALERT ...\")) tempState = HOT;",
    "    }",
    "    break;",
    "  case TempState::HOT:",
    "    if (t < alertC - 0.5F) {",
    "      if (send(chat, \"Back to normal\")) tempState = NORMAL;",
    "    }",
    "    break;",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 6.0, 3.45, 10);
  T.addBullets(s, [
    "No state = a message every 2 s",
    "0.5 C band: the thermostat from session 15",
    "State changes only if the send worked",
    "/alert 26.5 sets the threshold from the phone",
    "Telegram allows about 1 message per second per chat: send on an event, never in a loop",
  ], 6.7, BODY_TOP, 2.8, 3.6, 12);
  notes(s, "Live: /alert set about 1 C above the current reading, breathe on the AHT10, the phone buzzes. " +
    "Wait until it drops 0.5 C below: back to normal. The board talks first - the web server could not do that.");

  // 10 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Four Things That Bite", ++n);
  T.addCards(pres, s, [
    { title: "Poll blocks", body: "getUpdates() holds loop() for 0.6-2 s, sometimes 6 s (HTTPS round trip). Fine for a sensor. Not for a button or a motor.", accent: T.AMBER },
    { title: "setInsecure()", body: "Encrypted, but the board does not check who answers. A real device loads the root certificate: setCACert().", accent: T.AMBER },
    { title: "Anyone can write", body: "Your bot is public by its username. Check the chat id of every message and ignore strangers - one line, see the demo README.", accent: T.RED },
    { title: "One poller", body: "Two boards with one token steal each other's messages. One token per board.", accent: T.ICE },
  ], BODY_TOP + 0.15, 2.5);
  T.addCallout(pres, s, "info", "Measure it",
    "Measured on a C3: poll 590 ms or 2050 ms (a new HTTPS connection), max 6399 ms. The demo prints it every 10 s.",
    MARGIN, 4.0, CONTENT_W, 0.8);
  notes(s, "Show the status line: poll N ms (max M). That is the debugging thread from session 11 - " +
    "measure instead of guessing. If a project has a fast part, the bot goes to a slow timer or to its own FreeRTOS task.");

  // 11 -----------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 2", title: "Project Time", note: "Questions, wiring, code - I come to each of you" });
  n++;

  // 12 -----------------------------------------------------------------
  s = T.contentSlide(pres, "Today in Your Project", ++n);
  T.addBullets(s, [
    "The PlatformIO project builds for YOUR board",
    "Hardware wired and checked with the multimeter first",
    "Each part tested alone: sensor, actuator, display, radio",
    "README: what it does and a pin table (C3: GPIO 2 / 8 / 9 strapping)",
    "Commit and push before you leave",
  ], MARGIN, BODY_TOP, 5.4, 3.0, 14);
  T.addCallout(pres, s, "ok", "Stuck?",
    "Symptom -> hypothesis -> test -> conclusion. Then call me and tell me what you already tried.",
    6.2, BODY_TOP + 0.05, 3.3, 1.35);
  T.addCallout(pres, s, "info", "Bot in your project?",
    "Only if it is in your proposal, or your minimum version already works.",
    6.2, BODY_TOP + 1.6, 3.3, 1.15);
  notes(s, "Order of visits: whoever has a blocker first, then everyone in turn. " +
    "Check against the proposal: minimum version and the fallback for the risky part. " +
    "A bot only if it is in the proposal or the minimum already works.");

  // 13 -----------------------------------------------------------------
  T.summarySlide(pres, ++n, [
    "A Telegram bot: the board polls api.telegram.org over HTTPS, no open port",
    "Token from @BotFather is a password: secrets.h, .gitignore, /revoke if leaked",
    "getUpdates with offset + 1, sendMessage to a chat id",
    "Alerts: send on a state change, with a band - never in a loop",
    "Measure what the poll costs your loop() before you add it to a project",
  ], "Session 24, Mon 2026-11-23: the minimum version of your project works. Commit before the session.");

  await T.save(pres, "lesson23.pptx");
  console.log("OK: lesson23.pptx, " + n + " slides");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
