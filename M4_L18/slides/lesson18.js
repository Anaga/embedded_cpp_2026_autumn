// lesson18.js - Lesson 18: WiFi and a Web Server (ESP32-C3)
// Build: npm install pptxgenjs react react-dom react-icons sharp
//        node lesson18.js   -> lesson18.pptx
// ASCII only: slides-theme.js asserts it on every string.

const T = require("./slides-theme");
const {
  FaWifi, FaServer, FaCode, FaBroadcastTower, FaLaptop, FaNetworkWired,
  FaMicrochip, FaMobileAlt,
} = require("react-icons/fa");

const { MARGIN, CONTENT_W, BODY_TOP } = T;

// ---------------------------------------------------------------- helpers

// Numbered steps without the square bullet marker.
function addSteps(slide, items, x, y, w, h, fontSize) {
  const runs = items.map((t, i) => ({
    text: T.assertAscii((i + 1) + ".  " + t, "step"),
    options: { breakLine: i < items.length - 1 },
  }));
  slide.addText(runs, {
    x, y, w, h,
    fontSize: fontSize || 14, fontFace: T.BODY, color: T.ICE,
    lineSpacingMultiple: 1.35, valign: "top", margin: 0,
  });
}

// Small coloured label above a code block.
function addLabel(slide, text, color, x, y, w) {
  slide.addText(T.assertAscii(text, "label"), {
    x, y, w, h: 0.32,
    fontSize: 13, fontFace: T.HEADING, color, bold: true, margin: 0,
  });
}

// Plain text line.
function addText(slide, text, x, y, w, h, opts) {
  slide.addText(T.assertAscii(text, "text"), Object.assign({
    x, y, w, h, fontSize: 13, fontFace: T.BODY, color: T.ICE, margin: 0, valign: "top",
  }, opts || {}));
}

// Single card (for layouts where addCards does not fit).
function addBox(pres, slide, title, body, x, y, w, h, accent) {
  slide.addShape(pres.shapes.RECTANGLE, {
    x, y, w, h, fill: { color: T.CARD_BG },
    line: { color: accent || T.MID, width: accent ? 2 : 1 }, shadow: T.makeShadow(),
  });
  slide.addText(T.assertAscii(title, "box title"), {
    x: x + 0.15, y: y + 0.15, w: w - 0.3, h: 0.4,
    fontSize: 16, fontFace: T.HEADING, color: T.WHITE, bold: true, margin: 0,
  });
  slide.addText(T.assertAscii(body, "box body"), {
    x: x + 0.15, y: y + 0.6, w: w - 0.3, h: h - 0.75,
    fontSize: 13, fontFace: T.BODY, color: T.ICE, margin: 0, valign: "top",
  });
}

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "notes"));
}

// ---------------------------------------------------------------- deck

(async () => {
  const pres = T.newDeck("Lesson 18 - WiFi and a Web Server",
                         "Lesson 18  |  WiFi and a Web Server");

  const icWifi = await T.icon(FaWifi, "#CADCFC");
  const icServer = await T.icon(FaServer, "#CADCFC");
  const icCode = await T.icon(FaCode, "#CADCFC");
  const icMqtt = await T.icon(FaBroadcastTower, "#FFB300");
  const icLaptop = await T.icon(FaLaptop, "#CADCFC");
  const icRouter = await T.icon(FaNetworkWired, "#CADCFC");
  const icChip = await T.icon(FaMicrochip, "#CADCFC");
  const icPhone = await T.icon(FaMobileAlt, "#CADCFC");

  let n = 1;
  let s;

  // 1 ------------------------------------------------------------ title
  s = T.titleSlide(pres, {
    tag: "LESSON 18",
    title: "WiFi and a Web Server",
    subtitle: "ESP32-C3: WiFi, HTTP and JSON - and a first look at MQTT",
    course: "Embedded Software Development in C/C++",
    meta: "Session 18  |  2026-11-02  |  Back on the ESP32-C3",
    iconData: icWifi,
  });
  notes(s, "Start of the connectivity block: sessions 18-21 on the ESP32-C3. " +
           "Before class: class router or hotspot on the table, 2.4 GHz, AP isolation off, " +
           "SSID and password on the whiteboard.");

  // 2 ------------------------------------------------------------ today
  s = T.contentSlide(pres, "Today", ++n);
  T.addCards(pres, s, [
    { title: "On the network", body: "Your C3 joins WiFi. Scan, signal strength, events and reasons.", iconData: icWifi },
    { title: "A web server", body: "The board serves a page. You open it from your laptop.", iconData: icServer },
    { title: "JSON API", body: "/status returns data, POST /led switches the LED. Tested with curl.", iconData: icCode },
    { title: "MQTT preview", body: "The board publishes, you subscribe from the laptop or phone.", iconData: icMqtt, accent: T.AMBER },
  ], 1.25, 2.75);
  T.addCallout(pres, s, "info", "Two projects, one step apart",
    "M4_L18/lab1-web: AHT10 + WiFi + web server. M4_L18/lab2-mqtt: the same plus MQTT. You read them, run them and extend them.",
    MARGIN, 4.1, CONTENT_W, 0.95);
  notes(s, "2 min. The demo is complete; the labs are about running, reading and extending it.");

  // 3 ------------------------------------------------------------ homework check
  s = T.contentSlide(pres, "Homework check: lesson 17", ++n);
  T.addBullets(s, [
    "Night light: the two limits are named constants; the LED goes through led_set(bool on), so active-low lives in one place",
    "Orientation: one function MpuSample -> enum, one function enum -> text",
    "Extra: print only on a change - the static variable, and the first sample is not a change",
    "What did not work? DFU upload, the macOS accessory prompt, a missing sensor",
  ], MARGIN, BODY_TOP, CONTENT_W, 2.8, 15);
  T.addCallout(pres, s, "ok", "The same pattern today",
    "Decide in one function, act in another. The web page, curl and MQTT all end in the same ledSet().",
    MARGIN, 4.1, CONTENT_W, 0.85);
  notes(s, "15 min. Look at one or two solutions on the projector. Ask who pushed.");

  // 4 ------------------------------------------------------------ section
  s = T.sectionSlide(pres, { kicker: "PART 1", title: "Back on the ESP32-C3", note: "Wiring, project setup, secrets" });
  n++;

  // 5 ------------------------------------------------------------ pin map
  s = T.contentSlide(pres, "C3 pin map from today", ++n);
  T.addTable(s, ["Function", "Pin", "Notes"], [
    ["Button A / B", "GPIO 0 / 1", "INPUT_PULLUP, active low (K3)"],
    ["Potentiometer", "GPIO 4", "3.3k in series to 3V3 (K3)"],
    ["RGB LED R / G / B", "GPIO 5 / 6 / 7", "common anode to 3V3, LOW = on (K3)"],
    ["AHT10 SDA", "GPIO 8", "strapping pin, on-board blue LED"],
    ["AHT10 SCL", "GPIO 9", "strapping pin, BOOT button"],
    ["AHT10 VIN / GND", "3V3 / GND", "never 5V"],
  ], MARGIN, BODY_TOP, CONTENT_W, [2.6, 2.0, 4.4], 13);
  T.addCallout(pres, s, "warn", "GPIO 8 and 9 are strapping pins",
    "The AHT10 pull-ups keep both HIGH at reset - that is the normal boot. The blue LED on GPIO 8 now " +
    "flickers with every I2C transfer: a free I2C activity light. Pressing BOOT (GPIO 9 = SCL) spoils one reading.",
    MARGIN, 3.85, CONTENT_W, 1.1);
  notes(s, "5 min. SDA 8 / SCL 9 are the Wire defaults of lolin_c3_mini, so most examples work " +
           "without pin setup. Anything that holds SDA or SCL low at reset can stop the boot. " +
           "Students rewire the AHT10 from the Pico to the C3 now.");

  // 6 ------------------------------------------------------------ project setup
  s = T.contentSlide(pres, "Project setup: platformio.ini and secrets", ++n);
  T.addCode(s, [
    "[env:c3]",
    "platform = platformio/espressif32",
    "board = lolin_c3_mini",
    "framework = arduino",
    "monitor_speed = 115200",
    "build_unflags = -std=gnu++11",
    "build_flags = -std=gnu++17",
    "lib_deps =",
    "    adafruit/Adafruit AHTX0",
    "    bblanchon/ArduinoJson@^7",
    "    knolleary/PubSubClient@^2.8",
  ].join("\n"), MARGIN, BODY_TOP, 4.5, 3.2, 10, "lab1-web/platformio.ini (env s2 for the Lolin S2 Mini)");
  T.addBullets(s, [
    "Copy include/secrets.example.h -> include/secrets.h",
    "Fill in the class WiFi name and password",
    "secrets.h is in .gitignore: it never reaches GitHub",
    "No secrets.h -> the build stops and says what to do",
  ], 5.25, BODY_TOP, 4.25, 2.3, 13);
  T.addCallout(pres, s, "info", "Why platformio/espressif32",
    "With the owner in front PlatformIO always takes the official platform (Arduino core 2.0.17), " +
    "even if a core 3.x platform is installed as well.",
    5.25, 3.5, 4.25, 1.45);
  notes(s, "3 min. Core 3.x (pioarduino) installs under the same name espressif32 and then takes over " +
           "every project that says only 'espressif32' - tested on the instructor Mac. Online examples " +
           "are mostly 3.x: translate, do not paste.");

  // 7 ------------------------------------------------------------ section
  s = T.sectionSlide(pres, { kicker: "PART 2", title: "WiFi", note: "Radio, signal strength, connecting without blocking" });
  n++;

  // 8 ------------------------------------------------------------ basics
  s = T.contentSlide(pres, "WiFi basics", ++n);
  T.addCards(pres, s, [
    { title: "2.4 GHz only", body: "The C3 has no 5 GHz radio. Channels 1 to 13; 1, 6 and 11 do not overlap. A 5 GHz-only network is invisible to it." },
    { title: "Station (STA)", body: "The board joins an existing network, like your laptop. The router gives it an IP address. Today's default.", accent: T.GREEN },
    { title: "Access point (AP)", body: "The board IS the network. The laptop joins it and opens 192.168.4.1. No router needed - plan B." },
  ], 1.25, 2.1);
  T.addCallout(pres, s, "info", "Every board has a MAC address",
    "Six bytes, unique per chip. The demo prints it and builds a short id from the last two bytes: esp-9494. Write yours on the board.",
    MARGIN, 3.65, CONTENT_W, 0.95);
  notes(s, "5 min. Hand out stickers or a marker for the ids now - they are needed again in L19 (ESP-NOW peers by MAC).");

  // 9 ------------------------------------------------------------ RSSI
  s = T.contentSlide(pres, "Signal strength: RSSI in dBm", ++n);
  T.addTable(s, ["RSSI", "Meaning"], [
    ["-30 .. -50 dBm", "excellent"],
    ["-50 .. -67 dBm", "good"],
    ["-67 .. -75 dBm", "fine for sensors"],
    ["-75 .. -85 dBm", "weak, reconnects"],
    ["below -85 dBm", "unusable"],
  ], MARGIN, BODY_TOP, 4.2, [2.0, 2.2], 13);
  addText(s, "Same spot, same router (instructor bench):", 5.0, BODY_TOP, 4.5, 0.32, { fontSize: 13, color: T.WHITE, bold: true });
  T.addTable(s, ["Board", "RSSI"], [
    ["Lolin S2 Mini", "-49 dBm"],
    ["C3 Super Mini, board 1", "-67 dBm"],
    ["C3 Super Mini, board 2", "-81 dBm"],
  ], 5.0, 1.55, 4.5, [2.9, 1.6], 13);
  T.addCallout(pres, s, "warn", "The Super Mini antenna is weak",
    "dBm is logarithmic: every 10 dB is 10x less power, so the C3 hears 18-32 dB worse than the S2. Sit close to the " +
    "class router and hold the board by the cable, not by the antenna end. Some boards connect only with " +
    "WiFi.setTxPower(WIFI_POWER_8_5dBm) - the demo sets it.",
    MARGIN, 3.5, CONTENT_W, 1.45);
  notes(s, "5 min. Numbers from the instructor bench on 2026-11-01, 2 m from the router. One C3 saw no network " +
           "at all from the next room; at the router it worked. The ceramic antenna sits next to the crystal and LDO.");

  // 10 ----------------------------------------------------------- lab 1
  s = T.contentSlide(pres, "Lab 1a: who is on the air?", ++n);
  T.addCode(s, [
    "int n = WiFi.scanNetworks();",
    "for (int i = 0; i < n; i++) {",
    "  Serial.printf(\"%2d  %-32s %4d dBm  ch %2d\\n\", i,",
    "                WiFi.SSID(i).c_str(), WiFi.RSSI(i),",
    "                WiFi.channel(i));",
    "}",
    "WiFi.scanDelete();",
  ].join("\n"), MARGIN, BODY_TOP, 5.3, 2.15, 10, "Runs once in setup(): blocks for 2-3 s");
  addSteps(s, [
    "Wire the AHT10 to 8 / 9",
    "secrets.h: class network",
    "pio run -e c3 -t upload",
    "pio device monitor -e c3",
    "Note your id and RSSI",
    "Compare with your neighbour",
  ], 6.1, BODY_TOP, 3.4, 2.5, 14);
  T.addCode(s, [
    "Board ESP32-C3 Super Mini  MAC 28:37:2F:75:94:94  id esp-9494",
    " 0  class-router                      -55 dBm  ch  6",
    " 1  HUAWEICF10                        -81 dBm  ch  5",
  ].join("\n"), MARGIN, 3.8, CONTENT_W, 1.1, 10);
  notes(s, "15 min. Expect at least one board that sees little or nothing: move it next to the router first, " +
           "then try without setTxPower. A board that sees nothing even at the router is a spare-board case.");

  // 11 ----------------------------------------------------------- do not block
  s = T.contentSlide(pres, "Connecting: do not wait in a loop", ++n);
  addLabel(s, "Blocks forever if the password is wrong", T.RED, MARGIN, BODY_TOP, 4.3);
  T.addCode(s, [
    "WiFi.begin(WIFI_SSID, WIFI_PASS);",
    "while (WiFi.status() != WL_CONNECTED) {",
    "  delay(100);",
    "}",
    "Serial.println(WiFi.localIP());",
  ].join("\n"), MARGIN, 1.55, 4.3, 1.75, 11);
  addLabel(s, "The demo: start it, then carry on", T.GREEN, 5.2, BODY_TOP, 4.3);
  T.addCode(s, [
    "WiFi.onEvent(onWifiEvent);",
    "WiFi.begin(WIFI_SSID, WIFI_PASS);",
    "// setup() ends, loop() runs.",
    "// The driver retries by itself,",
    "// events tell us what happens.",
  ].join("\n"), 5.2, 1.55, 4.3, 1.75, 11);
  T.addBullets(s, [
    "loop() keeps serving the page and reading the sensor while WiFi connects or reconnects",
    "Same rule as since K3: no waiting loops in the main path",
  ], MARGIN, 3.6, CONTENT_W, 1.3, 14);
  notes(s, "3 min. Ask first: what happens with this loop if the router is switched off? Then show the right side.");

  // 12 ----------------------------------------------------------- events
  s = T.contentSlide(pres, "WiFi events", ++n);
  T.addCode(s, [
    "static void onWifiEvent(WiFiEvent_t event,",
    "                        WiFiEventInfo_t info) {",
    "  switch (event) {",
    "    case ARDUINO_EVENT_WIFI_STA_GOT_IP:",
    "      Serial.printf(\"WiFi: got IP %s\\n\",",
    "                    WiFi.localIP().toString().c_str());",
    "      break;",
    "    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:",
    "      Serial.printf(\"WiFi: disconnected, reason %u\\n\",",
    "                    info.wifi_sta_disconnected.reason);",
    "      break;",
    "    default: break;",
    "  }",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 5.6, 3.95, 10);
  T.addBullets(s, [
    "Called by the WiFi driver, not by loop()",
    "Runs in another task - like the ISR in L11, but here Serial is allowed",
    "Keep it short: print, set a flag",
    "The demo prints a reason only when it changes",
  ], 6.35, BODY_TOP, 3.15, 3.8, 13);
  notes(s, "4 min. Link to session 11: ISR discipline. This is not an ISR, but it is still a different " +
           "context than loop(): shared variables need care.");

  // 13 ----------------------------------------------------------- reasons
  s = T.contentSlide(pres, "Why did it disconnect?", ++n);
  T.addTable(s, ["Reason", "Name", "Usually means"], [
    ["2", "AUTH_EXPIRE", "weak signal - move closer"],
    ["15", "4WAY_HANDSHAKE_TIMEOUT", "wrong password"],
    ["201", "NO_AP_FOUND", "SSID typo, 5 GHz only, out of range"],
    ["202", "AUTH_FAIL", "wrong password"],
    ["204", "HANDSHAKE_TIMEOUT", "wrong password"],
  ], MARGIN, BODY_TOP, CONTENT_W, [1.0, 3.4, 4.6], 13);
  T.addCallout(pres, s, "ok", "Read it, do not guess",
    "Symptom -> hypothesis -> evidence, as in session 15. The scan shows whether the network is there at all; " +
    "the reason shows why the connection failed.",
    MARGIN, 3.55, CONTENT_W, 1.0);
  notes(s, "3 min. Live: put a wrong password in secrets.h on the instructor board and show reason 15 or 202.");

  // 14 ----------------------------------------------------------- network
  s = T.contentSlide(pres, "Finding the board on the network", ++n);
  T.addCards(pres, s, [
    { title: "Laptop", body: "192.168.3.20\nopens http://esp-9494.local/", iconData: icLaptop },
    { title: "Router", body: "192.168.3.1\nDHCP hands out the addresses", iconData: icRouter },
    { title: "Board", body: "192.168.3.43, port 80\nmDNS name esp-9494.local", iconData: icChip, accent: T.GREEN },
  ], 1.2, 2.2);
  T.addBullets(s, [
    "Laptop and board must be on the same network",
    "The IP can change after a restart - the .local name stays",
  ], MARGIN, 3.45, CONTENT_W, 0.75, 13);
  T.addCallout(pres, s, "warn", "The page does not open?",
    "Same WiFi on the laptop? macOS: Privacy & Security -> Local Network -> allow the browser and the terminal. " +
    "Router: AP isolation off.",
    MARGIN, 4.2, CONTENT_W, 0.85);
  notes(s, "3 min. Example addresses from the instructor bench. mDNS works out of the box on macOS and usually on Windows 10/11.");

  // 15 ----------------------------------------------------------- section
  s = T.sectionSlide(pres, { kicker: "PART 3", title: "HTTP and JSON", note: "The language of the web, spoken by a microcontroller" });
  n++;

  // 16 ----------------------------------------------------------- HTTP
  s = T.contentSlide(pres, "One HTTP exchange", ++n);
  addLabel(s, "Request: from the laptop", T.ICE, MARGIN, BODY_TOP, 4.3);
  T.addCode(s, [
    "GET /status HTTP/1.1",
    "Host: esp-9494.local",
    "User-Agent: curl/8.7.1",
    "Accept: */*",
  ].join("\n"), MARGIN, 1.55, 4.3, 1.45, 11);
  addLabel(s, "Response: from the board", T.ICE, 5.2, BODY_TOP, 4.3);
  T.addCode(s, [
    "HTTP/1.1 200 OK",
    "Content-Type: application/json",
    "Content-Length: 104",
    "",
    "{\"id\":\"esp-9494\",\"led\":false,",
    " \"t\":23.5,\"rh\":53.6, ...}",
  ].join("\n"), 5.2, 1.55, 4.3, 2.05, 11);
  T.addBullets(s, [
    "Plain text: a request line, headers, an empty line, then the body",
    "Status codes in the demo: 200 OK, 204 No Content (/favicon.ico), 400 Bad Request (broken JSON), 404 Not Found",
    "See it yourself: curl -i http://esp-xxxx.local/status",
  ], MARGIN, 3.8, CONTENT_W, 1.25, 13);
  notes(s, "5 min. The browser asks for /favicon.ico by itself - without the 204 handler the board log shows " +
           "'request handler not found'. Good moment to show that the client decides what is asked.");

  // 17 ----------------------------------------------------------- GET / POST
  s = T.contentSlide(pres, "GET or POST", ++n);
  T.addTable(s, ["", "GET", "POST"], [
    ["Purpose", "read something", "change something"],
    ["Body", "none", "form fields or JSON"],
    ["Repeat", "safe", "changes state again"],
    ["From the browser", "address bar, link", "form, fetch()"],
    ["In the demo", "/  and  /status", "/led"],
  ], MARGIN, BODY_TOP, CONTENT_W, [2.4, 3.3, 3.3], 13);
  T.addCode(s, [
    "curl -i http://esp-9494.local/status",
    "curl -X POST -d 'on=1' http://esp-9494.local/led",
    "curl -X POST -H 'Content-Type: application/json' -d '{\"on\":false}' http://esp-9494.local/led",
  ].join("\n"), MARGIN, 3.45, CONTENT_W, 1.1, 10, "Form field and JSON body: the demo accepts both");
  notes(s, "4 min. Windows: curl ships with Windows 10/11; in PowerShell use curl.exe and double quotes " +
           "(escape the inner quotes of the JSON).");

  // 18 ----------------------------------------------------------- WebServer
  s = T.contentSlide(pres, "WebServer on the board", ++n);
  T.addCode(s, [
    "static WebServer server(80);",
    "",
    "static void handleStatus() {",
    "  char out[256];",
    "  buildStatusJson(out, sizeof(out));",
    "  server.send(200, \"application/json\", out);",
    "}",
    "",
    "void setup() {",
    "  // ... WiFi ...",
    "  server.on(\"/status\", HTTP_GET, handleStatus);",
    "  server.on(\"/led\", HTTP_POST, handleLed);",
    "  server.begin();",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 5.3, 3.95, 10);
  T.addCode(s, [
    "void loop() {",
    "  server.handleClient();",
    "  // sensor, MQTT, print",
    "}",
  ].join("\n"), 6.05, BODY_TOP, 3.45, 1.4, 10);
  T.addBullets(s, [
    "server.on(path, method, handler): one function per URL",
    "handleClient() polls - like the UART in L12",
    "A delay() in loop() = a page that hangs",
    "Unknown path -> onNotFound() -> 404",
  ], 6.05, 2.75, 3.45, 2.35, 12);
  notes(s, "5 min. WebServer is synchronous: one request at a time, handled inside handleClient(). " +
           "That is enough for a class demo and keeps the loop() model they know.");

  // 19 ----------------------------------------------------------- JSON
  s = T.contentSlide(pres, "JSON with ArduinoJson 7", ++n);
  T.addCode(s, [
    "{",
    "  \"id\": \"esp-9494\",",
    "  \"led\": false,",
    "  \"t\": 23.5,",
    "  \"rh\": 53.6,",
    "  \"rssi\": -62",
    "}",
  ].join("\n"), MARGIN, BODY_TOP, 3.4, 2.15, 10, "Object, keys in quotes, numbers, true / false, null");
  T.addCode(s, [
    "// build: GET /status",
    "JsonDocument doc;",
    "doc[\"id\"]  = deviceId;",
    "doc[\"led\"] = ledOn;",
    "serializeJson(doc, out, size);",
    "",
    "// parse: POST /led",
    "JsonDocument doc;",
    "if (deserializeJson(doc, server.arg(\"plain\")) ||",
    "    !doc[\"on\"].is<bool>()) {",
    "  server.send(400, \"text/plain\", \"bad JSON\\n\");",
    "  return;",
    "}",
  ].join("\n"), 4.15, BODY_TOP, 5.35, 3.7, 10);
  T.addCallout(pres, s, "warn", "v6 or v7?",
    "Online examples often use StaticJsonDocument<200> (v6). We use v7: just JsonDocument. Same trap as core 3.x.",
    MARGIN, 3.75, 3.4, 1.25);
  notes(s, "5 min. Never trust input: the parse checks both the syntax and the type of 'on'. " +
           "Live: send {\"on\":\"yes\"} with curl and show the 400.");

  // 20 ----------------------------------------------------------- page
  s = T.contentSlide(pres, "The page: HTML inside the firmware", ++n);
  T.addCode(s, [
    "static const char INDEX_HTML[] = R\"html(",
    "<p>Temperature: <b id=\"t\">-</b> C</p>",
    "<button onclick=\"setLed(true)\">LED on</button>",
    "<script>",
    "function poll() {",
    "  fetch('/status').then(r => r.json()).then(show);",
    "}",
    "setInterval(poll, 2000);",
    "</script>",
    ")html\";",
  ].join("\n"), MARGIN, BODY_TOP, 5.6, 2.9, 10, "Shortened - the full page is in lab1-web/src/main.cpp");
  T.addBullets(s, [
    "R\"html( ... )html\" is a raw string: no escaping of quotes",
    "The browser runs the JavaScript; the board only answers /status",
    "fetch() every 2 s: the values update without a reload",
    "A button sends POST /led with JSON; the answer is the new status",
  ], 6.35, BODY_TOP, 3.15, 3.8, 13);
  notes(s, "4 min. Bigger pages go into the flash file system (LittleFS, data/ folder, uploadfs) - " +
           "a final-project topic, not today.");

  // 21 ----------------------------------------------------------- lab 2
  s = T.contentSlide(pres, "Lab 1b: your board, your page", ++n);
  addSteps(s, [
    "Open http://esp-xxxx.local/ (or the IP from the monitor)",
    "Breathe on the AHT10 and watch the page",
    "curl -i .../status and read the headers",
    "Switch the LED from the page, then with curl and JSON",
    "Send broken JSON: which status code comes back?",
    "Press BOOT while it runs: what does the monitor say?",
  ], MARGIN, BODY_TOP, 5.4, 3.8, 14);
  addBox(pres, s, "Extra",
    "Add uptime_s and the state of button A to /status, and show both on the page.\n\n" +
    "Where: buildStatusJson() and INDEX_HTML.",
    6.2, BODY_TOP, 3.3, 2.6, T.AMBER);
  notes(s, "35 min with the extra. The 'LED' is the green channel of the RGB LED. " +
           "BOOT = SCL: expect 'AHT10: missing', then 'found' again.");

  // 22 ----------------------------------------------------------- plan B
  s = T.contentSlide(pres, "Plan B: the board as an access point", ++n);
  T.addCode(s, [
    "// include/secrets.h",
    "constexpr bool USE_SOFTAP = true;",
    "",
    "// the demo then does:",
    "WiFi.mode(WIFI_AP);",
    "WiFi.softAP(deviceId, AP_PASS);   // network \"esp-9494\"",
    "// laptop joins esp-9494, opens http://192.168.4.1/",
  ].join("\n"), MARGIN, BODY_TOP, 5.6, 2.2, 10);
  T.addBullets(s, [
    "No router, no internet - works anywhere",
    "The laptop is 20 cm away: the weak antenna does not matter",
    "While joined, the laptop has no internet",
    "MQTT is off in this mode",
  ], 6.35, BODY_TOP, 3.15, 3.0, 13);
  T.addCallout(pres, s, "info", "AP_PASS",
    "At least 8 characters, or softAP() fails. The demo prints a clear message if it does.",
    MARGIN, 3.65, 5.6, 0.85);
  notes(s, "2 min. Use this for any board that cannot hold a station connection in class.");

  // 23 ----------------------------------------------------------- section
  s = T.sectionSlide(pres, { kicker: "PART 4", title: "MQTT - a first look", note: "Details and diagnosis in session 20" });
  n++;

  // 24 ----------------------------------------------------------- pub/sub
  s = T.contentSlide(pres, "Publish and subscribe", ++n);
  T.addCards(pres, s, [
    { title: "Publisher", body: "Your board publishes\n.../esp-9494/status\nevery 5 s", iconData: icChip, accent: T.GREEN },
    { title: "Broker", body: "broker.hivemq.com:1883\nforwards each message to everyone subscribed to the topic", iconData: icMqtt, accent: T.AMBER },
    { title: "Subscriber", body: "Laptop or phone\nsubscribes to\ngamma-ed/2026/#", iconData: icPhone },
  ], 1.2, 2.4);
  T.addCallout(pres, s, "info", "HTTP vs MQTT",
    "HTTP: you ask, the board answers. MQTT: you subscribe once and messages are pushed to you. " +
    "The board subscribes too: .../esp-9494/led switches the LED.",
    MARGIN, 3.85, CONTENT_W, 1.1);
  notes(s, "4 min. Topics are paths; # matches everything below. Our prefix: gamma-ed/2026/<id>/...");

  // 25 ----------------------------------------------------------- try it
  s = T.contentSlide(pres, "Lab 2: MQTT (lab2-mqtt)", ++n);
  T.addCode(s, [
    "# watch the whole class",
    "mosquitto_sub -h broker.hivemq.com -t 'gamma-ed/2026/#' -v",
    "",
    "# switch your LED",
    "mosquitto_pub -h broker.hivemq.com -t 'gamma-ed/2026/esp-9494/led' -m 1",
  ].join("\n"), MARGIN, BODY_TOP, CONTENT_W, 1.75, 11);
  addText(s, "Upload lab2-mqtt, then: no mosquitto? MQTT Explorer (an app) or the HiveMQ WebSocket client in the browser.",
          MARGIN, 3.05, CONTENT_W, 0.4);
  T.addCallout(pres, s, "error", "A public broker is public",
    "Anyone in the world can read these topics - and switch your LED. No passwords, no personal data. " +
    "The client id must be unique worldwide: the demo uses gamma-ed-esp-xxxx.",
    MARGIN, 3.6, CONTENT_W, 1.1);
  notes(s, "6 min. Install: macOS 'brew install mosquitto', Windows installer from mosquitto.org. " +
           "Switch one student LED from the instructor laptop as the demo. Security and diagnosis: session 20.");

  // 26 ----------------------------------------------------------- summary
  T.summarySlide(pres, ++n, [
    "The C3 joins WiFi as a station; plan B is its own access point",
    "RSSI says whether the radio can work; the reason code says why it did not",
    "Never wait for WiFi in a loop: start it and react to events",
    "HTTP is text: request line, headers, body; status codes say what happened",
    "ArduinoJson 7: build for /status, parse with an error check for /led",
    "MQTT: the board publishes, everyone subscribed gets the message",
  ], "RGB colour from the web: POST /rgb with {\"r\",\"g\",\"b\"} 0..255, PWM through your RgbLed class, " +
     "three sliders on the page, the colour in /status. Spec: M4_L18/homework/README.md");

  await T.save(pres, "lesson18.pptx");
  console.log("OK: lesson18.pptx, " + n + " slides");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
