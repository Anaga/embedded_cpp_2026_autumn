// Lesson 19 - Sleep, Time and ESP-NOW (ESP32-C3 Super Mini)
//
// Build:
//   cp ../../../tools/slides-theme.js .      (or keep a copy next to this file)
//   npm install pptxgenjs react react-dom react-icons sharp
//   node lesson19.js            -> lesson19.pptx
//
// QA: render with LibreOffice and look at every slide.

const T = require("./slides-theme");

const NUM = { n: 0 };
const next = () => ++NUM.n;

function notes(slide, text) {
  slide.addNotes(T.assertAscii(text, "notes"));
}

// A row of small labelled boxes joined by arrows, computed from index.
function addFlow(pres, slide, steps, y, h, accentIdx) {
  const n = steps.length;
  const arrowW = 0.35;
  const w = (T.CONTENT_W - arrowW * (n - 1)) / n;
  steps.forEach((s, i) => {
    const x = T.MARGIN + i * (w + arrowW);
    const hot = accentIdx !== undefined && accentIdx.indexOf(i) >= 0;
    slide.addShape(pres.shapes.RECTANGLE, {
      x: x, y: y, w: w, h: h,
      fill: { color: T.CARD_BG }, line: { color: hot ? T.AMBER : T.MID, width: hot ? 2 : 1 },
    });
    slide.addText(T.assertAscii(s.title, "flow title"), {
      x: x, y: y + 0.08, w: w, h: 0.35,
      fontSize: 13, fontFace: T.HEADING, color: T.WHITE, bold: true, align: "center", margin: 0,
    });
    if (s.body) {
      slide.addText(T.assertAscii(s.body, "flow body"), {
        x: x + 0.08, y: y + 0.45, w: w - 0.16, h: h - 0.5,
        fontSize: 11, fontFace: T.BODY, color: T.ICE, align: "center", valign: "top", margin: 0,
      });
    }
    if (i < n - 1) {
      slide.addText("->", {
        x: x + w, y: y + h / 2 - 0.2, w: arrowW, h: 0.4,
        fontSize: 16, fontFace: T.CODE, color: T.ICE, bold: true, align: "center", margin: 0,
      });
    }
  });
}

(async () => {
  const { FaBatteryQuarter, FaMoon, FaClock, FaBroadcastTower } = require("react-icons/fa");
  const pres = T.newDeck(
    "Lesson 19 - Sleep, Time and ESP-NOW",
    "Lesson 19  |  Sleep, Time and ESP-NOW"
  );

  const icBattery = await T.icon(FaBatteryQuarter, "#FFB300");
  const icMoon = await T.icon(FaMoon, "#CADCFC");
  const icClock = await T.icon(FaClock, "#CADCFC");
  const icRadio = await T.icon(FaBroadcastTower, "#4CAF50");

  // ------------------------------------------------------------------ 1 title
  let s = T.titleSlide(pres, {
    tag: "LESSON 19",
    title: "Sleep, Time and ESP-NOW",
    subtitle: "Power modes, NTP and radio without a router on the ESP32-C3",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Arduino-ESP32 core 2.x  |  PlatformIO",
    iconData: icBattery,
  });
  next();
  notes(s, "Session 19, Friday evening. Today's story: a device that runs on a battery " +
    "has to sleep. Sleep breaks two things we took for granted: the clock and the network " +
    "connection. NTP fixes the clock, ESP-NOW fixes the network cost.");

  // ------------------------------------------------------------------ 2 plan
  s = T.contentSlide(pres, "Today", next());
  T.addTable(s,
    ["Part", "What", "Lab", "Min"],
    [
      ["0", "Homework 18 check: RGB colour from the web", "-", "15"],
      ["1", "Power modes of the ESP32-C3, deep sleep, wake sources", "Lab 1", "45"],
      ["2", "Exact time: NTP, time zone, time across deep sleep", "Lab 2", "30"],
      ["3", "ESP-NOW: why, how, the channel rule", "demo", "40"],
      ["4", "A sleeping sensor node that reports over ESP-NOW", "Lab 3", "40"],
      ["5", "Summary and homework", "-", "10"],
    ],
    T.MARGIN, T.BODY_TOP + 0.1, T.CONTENT_W, [0.6, 6.0, 1.4, 1.0], 13);
  T.addCallout(pres, s, "info", "One question runs through the whole session",
    "If the board sleeps most of the time, what does it cost to wake up and say something?",
    T.MARGIN, 3.95, T.CONTENT_W, 0.85);
  notes(s, "Timing: 15 / 45 / 30 / 40 / 40 / 10 = 180 min. Break after part 2. " +
    "The wiring from session 18 stays: AHT10 on GPIO 8 / 9, RGB on 5 / 6 / 7, " +
    "button A on GPIO 0 (K3 wiring).");

  // ------------------------------------------------------------------ 3 homework check
  s = T.contentSlide(pres, "Homework 18 Check", next());
  T.addBullets(s, [
    "POST /rgb with validation: what did your three bad curl calls return?",
    "Range check: is 256 rejected? Is \"r\":\"red\" rejected? Is a missing field rejected?",
    "Sliders: onchange, not oninput - why?",
    "One function decides the colour: rgbSet(const Colour&). Who calls it?",
    "MQTT extra: who got a colour from mosquitto_pub?",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.7);
  T.addCallout(pres, s, "ok", "Keep the board wired",
    "Today uses the same board: AHT10 on GPIO 8 / 9, RGB on 5 / 6 / 7, button A on GPIO 0.",
    T.MARGIN, 4.0, T.CONTENT_W, 0.85);
  notes(s, "Ask one student to show the three bad curl calls on the projector. " +
    "oninput sends a request for every pixel of slider movement; onchange sends one.");

  // ------------------------------------------------------------------ PART 1
  s = T.sectionSlide(pres, { kicker: "PART 1", title: "Power Modes", note: "What the chip can switch off, and what it costs to switch back on" });
  next();

  // ------------------------------------------------------------------ battery math
  s = T.contentSlide(pres, "Why Sleep: a 1000 mAh Battery", next());
  T.addTable(s,
    ["Design", "Average current", "Battery life"],
    [
      ["Always awake, WiFi connected", "~30 mA", "~1.4 days"],
      ["Deep sleep, wake every 10 min, WiFi 3 s", "~0.5 mA", "~80 days"],
      ["Deep sleep, wake every 10 min, ESP-NOW 0.1 s", "~0.02 mA", "years (*)"],
    ],
    T.MARGIN, T.BODY_TOP + 0.1, T.CONTENT_W, [4.6, 2.2, 2.2], 13);
  T.addBullets(s, [
    "Assumed: 100 mA average while awake with the radio on, 5 uA in deep sleep (chip only)",
    "Average = awake share x awake current + sleep share x sleep current",
    "(*) Then the battery's own self-discharge and the board, not the chip, decide",
  ], T.MARGIN, 2.85, T.CONTENT_W, 1.3, 13);
  T.addCallout(pres, s, "warn", "The awake time dominates",
    "3 s awake every 10 min costs 100 times more than 10 min of deep sleep. Shorten the wake.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.85);
  notes(s, "Work one row on the board. WiFi row: 3 s x 100 mA = 300 mAs per wake, " +
    "6 wakes per hour = 1800 mAs = 0.5 mAh per hour. Sleep: 0.005 mAh per hour. " +
    "1000 / 0.505 = about 1980 h = about 80 days. ESP-NOW row: 0.1 s x 100 mA x 6 = 60 mAs " +
    "= 0.017 mAh per hour, plus 0.005 -> about 0.022 mAh per hour -> 45000 h. " +
    "Numbers are rough on purpose; the ratio is the point. This slide sets up part 3.");

  // ------------------------------------------------------------------ modes table
  s = T.contentSlide(pres, "ESP32-C3 Power Modes", next());
  T.addTable(s,
    ["Mode", "CPU", "RAM", "WiFi", "Current (typ)"],
    [
      ["Active, radio TX", "runs", "kept", "on", "up to 335 mA peak"],
      ["Active, radio RX", "runs", "kept", "on", "84 mA"],
      ["Modem sleep", "runs", "kept", "off between beacons", "13 .. 28 mA"],
      ["Light sleep", "paused", "kept", "off", "130 uA"],
      ["Deep sleep", "off", "lost", "off", "5 uA"],
      ["Power off (EN low)", "off", "lost", "off", "1 uA"],
    ],
    T.MARGIN, T.BODY_TOP + 0.1, T.CONTENT_W, [2.2, 1.0, 1.0, 2.4, 2.4], 12);
  s.addText("ESP32-C3 datasheet v1.7, tables 4-7, 4-8, 4-9. 3.3 V, 25 C. Modem sleep range: 80 .. 160 MHz, idle .. running.", {
    x: T.MARGIN, y: 3.75, w: T.CONTENT_W, h: 0.3, fontSize: 10, fontFace: T.BODY, color: T.GRAY, italic: true, margin: 0,
  });
  T.addCallout(pres, s, "info", "Arduino already uses modem sleep",
    "With WiFi connected the core turns the radio off between router beacons. WiFi.setSleep(false) keeps it on.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.85);
  notes(s, "Light sleep: RAM and the program state survive, wake takes about a millisecond, " +
    "but the WiFi connection is not kept and the USB console stops answering. " +
    "We only use deep sleep in code today. Modem sleep comes back in part 3: a gateway that " +
    "must hear ESP-NOW packets cannot afford it.");

  // ------------------------------------------------------------------ chip vs board
  s = T.contentSlide(pres, "Chip Numbers Are Not Board Numbers", next());
  T.addCards(pres, s, [
    { title: "The chip", body: "5 uA in deep sleep. This is what the datasheet promises, measured on the bare chip.", accent: T.GREEN },
    { title: "The Super Mini", body: "Power LED, voltage regulator and USB parts stay powered. They draw far more than the chip.", accent: T.AMBER },
    { title: "The AHT10", body: "Still on 3V3 while the chip sleeps. Every module on the rail counts.", accent: T.AMBER },
  ], T.BODY_TOP + 0.15, 2.0);
  T.addCallout(pres, s, "warn", "Measure before you believe a battery estimate",
    "A battery product uses a board designed for it: no power LED, a low-quiescent regulator, switchable sensor power.",
    T.MARGIN, 3.65, T.CONTENT_W, 0.95);
  notes(s, "If a USB power meter or a multimeter in series is available, show the board current " +
    "awake vs in deep sleep. The drop is real but nowhere near 5 uA on this board. " +
    "Today we measure what we can in firmware: how long the board is awake.");

  // ------------------------------------------------------------------ deep sleep = reboot
  s = T.contentSlide(pres, "Deep Sleep Is a Reboot", next());
  T.addCode(s, [
    "RTC_DATA_ATTR uint32_t bootCount = 0; // kept",
    "uint32_t lostCount = 0;   // 0 again after a wake",
    "",
    "void setup() {",
    "  bootCount++;",
    "  // ... do the work ...",
    "  esp_sleep_enable_timer_wakeup(SLEEP_US);",
    "  esp_deep_sleep_start();  // does not return",
    "}",
    "",
    "void loop() {}             // never runs",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.3, 3.5, 11);
  T.addBullets(s, [
    "Wake = reset: setup() runs again from the top",
    "Survives: 8 KB RTC memory (RTC_DATA_ATTR), the RTC timer",
    "Lost: RAM variables, WiFi, Serial, pin states",
    "A sleeping program lives in setup()",
  ], 6.0, T.BODY_TOP, 3.5, 3.3, 13);
  notes(s, "RTC_DATA_ATTR places the variable in RTC memory. It survives deep sleep, " +
    "but a power cycle clears it. Ask: what happens to a millis() timestamp across sleep? " +
    "Answer: millis() restarts at 0 on every wake.");

  // ------------------------------------------------------------------ wake sources
  s = T.contentSlide(pres, "Wake Sources on the ESP32-C3", next());
  T.addCode(s, [
    "constexpr uint64_t SLEEP_US = 20ULL * 1000000ULL;",
    "constexpr uint8_t  PIN_BUTTON = 0;    // button A",
    "",
    "esp_sleep_enable_timer_wakeup(SLEEP_US);",
    "esp_deep_sleep_enable_gpio_wakeup(",
    "    1ULL << PIN_BUTTON,",
    "    ESP_GPIO_WAKEUP_GPIO_LOW);",
    "esp_deep_sleep_start();",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.3, 2.6, 11);
  T.addBullets(s, [
    "Timer: microseconds, 64-bit",
    "GPIO: only GPIO 0 .. 5 can wake from deep sleep on the C3",
    "Button A on GPIO 0 fits, button B on GPIO 1 too",
    "Reset button (EN): a full reset, not a wake",
  ], 6.0, T.BODY_TOP, 3.5, 2.6, 13);
  T.addCallout(pres, s, "warn", "Two traps with the button",
    "Still pressed when sleep starts -> the board wakes at once. Wait for the release first. " +
    "Wakes by itself -> the input floats: add an external 10k pull-up to 3V3.",
    T.MARGIN, 3.9, T.CONTENT_W, 1.05);
  notes(s, "esp_sleep_start sets the pulls for the wake pins itself, but the IDF documentation " +
    "recommends an external pull-up when waking on a low level. GPIO 0 .. 5 are the pins " +
    "with RTC function on the C3. GPIO 8 and 9 (I2C) are strapping pins and cannot wake.");

  // ------------------------------------------------------------------ wake cause
  s = T.contentSlide(pres, "Why Did I Wake Up?", next());
  T.addCode(s, [
    "switch (esp_sleep_get_wakeup_cause()) {",
    "  case ESP_SLEEP_WAKEUP_TIMER:",
    "    Serial.println(\"wake: timer\");",
    "    break;",
    "  case ESP_SLEEP_WAKEUP_GPIO:",
    "    Serial.println(\"wake: button\");",
    "    break;",
    "  default:                       // UNDEFINED",
    "    Serial.println(\"cold start: power-on or reset\");",
    "    break;",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.6, 3.5, 11);
  T.addBullets(s, [
    "One setup(), three different starts",
    "Cold start: do the expensive things once (WiFi, NTP)",
    "Timer wake: the routine job, as short as possible",
    "Button wake: a person is there - stay awake a little",
  ], 6.3, T.BODY_TOP, 3.2, 3.3, 13);
  notes(s, "This switch is the skeleton of every sleeping program today, including the homework.");

  // ------------------------------------------------------------------ USB
  s = T.contentSlide(pres, "Deep Sleep and the USB Port", next());
  T.addBullets(s, [
    "The C3 has no USB-serial chip: USB is inside the chip, and it powers off in deep sleep",
    "The COM port disappears, then comes back after each wake",
    "The PlatformIO monitor reconnects by itself, but may miss the first lines",
    "-> the labs wait SERIAL_WAIT_MS after a wake before printing (only for us, not for a battery)",
    "Upload fails while the board sleeps: there is no port to upload to",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.8, 14);
  T.addCallout(pres, s, "ok", "How to upload to a sleeping board",
    "Build first. Press RESET or button A: the board stays awake 10 s. Click Upload in that window. " +
    "Last resort: hold BOOT, press RESET, release BOOT - the board waits for an upload.",
    T.MARGIN, 3.95, T.CONTENT_W, 1.05);
  notes(s, "The 10 s window after reset or button is a named constant (AWAKE_WINDOW_MS) in every " +
    "lab. On Windows the port can take longer to come back; that is why 10 s and not 3 s.");

  // ------------------------------------------------------------------ lab 1
  s = T.contentSlide(pres, "Lab 1 - Deep Sleep", next());
  T.addBullets(s, [
    "Open M4_L19/lab1-sleep, build, upload, open the monitor",
    "Watch: cold start -> 10 s awake -> sleep 20 s -> timer wake",
    "Press button A while it sleeps: button wake, 10 s awake",
    "Press RESET: bootCount goes back to 1? Check",
    "Unplug USB and plug back in: bootCount?",
  ], T.MARGIN, T.BODY_TOP, 4.2, 3.4, 13);
  T.addCode(s, [
    "#3 wake: timer",
    "   prev awake 2014 ms",
    "   sleeping 20 s",
    "#4 wake: button",
    "   awake 10 s - upload now",
    "   prev awake 2011 ms",
    "   sleeping 20 s",
  ].join("\n"), 4.9, T.BODY_TOP, 4.6, 2.35, 11, "Serial output (example)");
  T.addCallout(pres, s, "info", "Where do the 2000 ms come from?",
    "Almost all of it is SERIAL_WAIT_MS for the USB monitor. The real work takes a few ms.",
    4.9, 3.95, 4.6, 0.95);
  notes(s, "15 min. RESET on the C3 is a full chip reset: RTC memory may or may not survive " +
    "depending on the reset kind - let them find out. A power cycle always clears it. " +
    "Ask them to change SERIAL_WAIT_MS to 0 and see which lines disappear from the monitor.");

  // ------------------------------------------------------------------ PART 2
  s = T.sectionSlide(pres, { kicker: "PART 2", title: "Exact Time", note: "millis() starts at 0 on every wake. The real clock does not." });
  next();

  // ------------------------------------------------------------------ NTP
  s = T.contentSlide(pres, "NTP: Ask the Internet for the Time", next());
  T.addCode(s, [
    "// Tallinn: EET (UTC+2), EEST (UTC+3) from the last",
    "// Sunday of March 03:00 to the last Sunday of October 04:00",
    "constexpr const char *TZ_TALLINN = \"EET-2EEST,M3.5.0/3,M10.5.0/4\";",
    "constexpr const char *NTP_SERVER = \"pool.ntp.org\";",
    "",
    "configTzTime(TZ_TALLINN, NTP_SERVER);  // WiFi must be up",
    "struct tm now;",
    "if (getLocalTime(&now, NTP_TIMEOUT_MS)) {",
    "  char text[24];",
    "  strftime(text, sizeof(text), \"%Y-%m-%d %H:%M:%S\", &now);",
    "  Serial.println(text);",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 3.45, 10);
  s.addText("NTP gives UTC. The TZ string turns it into local time, summer time included. getLocalTime() waits until the first answer arrives.", {
    x: T.MARGIN, y: 4.75, w: T.CONTENT_W, h: 0.35, fontSize: 12, fontFace: T.BODY, color: T.ICE, margin: 0,
  });
  notes(s, "The POSIX TZ string: name, offset with inverted sign, summer name, then the rules " +
    "M3.5.0/3 = month 3, last week, Sunday, at 03:00 local. One request is a single UDP packet " +
    "to port 123; the time is good to a few milliseconds.");

  // ------------------------------------------------------------------ time across sleep
  s = T.contentSlide(pres, "Time Across Deep Sleep", next());
  T.addCards(pres, s, [
    { title: "Kept", body: "The RTC timer keeps counting in deep sleep. time() is still right after a wake - no WiFi needed.", accent: T.GREEN, iconData: icClock },
    { title: "Lost", body: "The TZ setting lives in RAM. After every wake: setenv(\"TZ\", ...) and tzset(), or you print UTC.", accent: T.RED, iconData: icMoon },
    { title: "Drifts", body: "Our boards run the RTC from the internal 136 kHz RC oscillator. It drifts with temperature. Resync now and then.", accent: T.AMBER, iconData: icBattery },
  ], T.BODY_TOP + 0.1, 2.75);
  T.addCallout(pres, s, "info", "The cure for drift costs power",
    "A 32 kHz crystal on the board, or a WiFi wake to resync. The homework measures how big the drift is.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.85);
  notes(s, "IDF docs, System Time: with the internal RC the time may drift in deep and light sleep; " +
    "an external 32 kHz crystal is more stable and costs about 1 uA. No ppm figure is given for " +
    "the RC - that is why we measure it in the homework.");

  // ------------------------------------------------------------------ lab 2
  s = T.contentSlide(pres, "Lab 2 - NTP Once, Then Sleep", next());
  addFlow(pres, s, [
    { title: "Cold start", body: "WiFi on, NTP, WiFi off" },
    { title: "Sleep 20 s", body: "RTC timer runs" },
    { title: "Wake", body: "set TZ, print time" },
    { title: "Sleep 20 s", body: "no WiFi at all" },
  ], T.BODY_TOP + 0.05, 0.9, [0]);
  T.addCode(s, [
    "   WiFi 1840 ms, NTP 212 ms",
    "   awake 10 s - upload now",
    "2026-11-06 19:12:15  #1  cold    awake 2068 ms (+12000 waiting)",
    "2026-11-06 19:12:37  #2  timer   awake 3 ms (+2000 waiting)",
    "2026-11-06 19:12:59  #3  timer   awake 3 ms (+2000 waiting)",
  ].join("\n"), T.MARGIN, 2.25, T.CONTENT_W, 1.75, 11);
  T.addBullets(s, [
    "Copy include/secrets.example.h to secrets.h, class WiFi from the whiteboard",
    "Write down two numbers: awake time on a cold start, awake time on a timer wake",
  ], T.MARGIN, 4.05, T.CONTENT_W, 0.95, 13);
  notes(s, "15 min. The two numbers are the bridge to part 3: seconds with WiFi, milliseconds " +
    "without. Collect them from everyone on the whiteboard. Break after this lab.");

  // ------------------------------------------------------------------ PART 3
  s = T.sectionSlide(pres, { kicker: "PART 3", title: "ESP-NOW", note: "Send a packet without joining the network" });
  next();

  // ------------------------------------------------------------------ why
  s = T.contentSlide(pres, "Why ESP-NOW? Look at Your Lab 2 Numbers", next());
  T.addTable(s,
    ["", "WiFi + HTTP / MQTT", "ESP-NOW"],
    [
      ["Join a network (auth, DHCP)", "yes, every wake", "no"],
      ["Router or broker needed", "yes", "no"],
      ["Awake before the first packet", "seconds", "milliseconds"],
      ["Payload", "anything (TCP)", "up to 250 bytes"],
      ["Delivery confirmed by", "TCP / the server", "MAC-level ACK"],
      ["Reaches the internet", "yes", "no - needs a gateway"],
    ],
    T.MARGIN, T.BODY_TOP + 0.1, T.CONTENT_W, [3.4, 2.8, 2.8], 12);
  T.addCallout(pres, s, "ok", "Always on USB power and on the network? Then WiFi is fine",
    "ESP-NOW pays off when a node sleeps, has no router nearby, or must answer in milliseconds (a remote button).",
    T.MARGIN, 4.0, T.CONTENT_W, 0.95);
  notes(s, "Start from the whiteboard numbers of Lab 2. Put them into the battery formula from part 1. " +
    "Then ask: what if the board could send without joining the network at all?");

  // ------------------------------------------------------------------ basics
  s = T.contentSlide(pres, "ESP-NOW in One Slide", next());
  T.addBullets(s, [
    "Espressif's own protocol on top of 802.11 frames: same radio, no access point, no IP",
    "Addressed by MAC address - print your own with WiFi.macAddress()",
    "Unicast to a peer is acknowledged by the receiver's radio -> the sender gets SUCCESS or FAIL",
    "Broadcast to FF:FF:FF:FF:FF:FF: everyone on the channel hears it, nobody acknowledges",
    "Up to 250 bytes per packet, up to 20 peers",
    "Both sides must be on the same WiFi channel",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 3.0, 14);
  T.addCallout(pres, s, "warn", "SUCCESS means \"a radio heard it\", not \"the program used it\"",
    "The ACK comes from the receiver's WiFi hardware. Whether its code handled the packet is another question.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.9);
  notes(s, "Peers can be encrypted (pairwise key), we do not use that today. " +
    "Range is similar to WiFi; with no ACK on broadcast, broadcast delivery is never confirmed.");

  // ------------------------------------------------------------------ send code
  s = T.contentSlide(pres, "Init, Peer, Send", next());
  T.addCode(s, [
    "#include <WiFi.h>",
    "#include <esp_now.h>",
    "",
    "WiFi.mode(WIFI_STA);                 // the radio must be started",
    "esp_now_init();",
    "",
    "esp_now_peer_info_t peer = {};",
    "memcpy(peer.peer_addr, GATEWAY_MAC, 6);",
    "peer.channel = 0;                    // 0 = the current channel",
    "peer.encrypt = false;",
    "esp_now_add_peer(&peer);",
    "",
    "esp_now_send(GATEWAY_MAC, (const uint8_t *)&msg, sizeof(msg));",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.75, 10);
  T.addBullets(s, [
    "No WiFi.begin(): we never join a network",
    "Every call returns esp_err_t - check it",
    "esp_now_send() only queues the packet",
    "The result arrives later, in the send callback",
  ], 6.6, T.BODY_TOP, 2.9, 3.4, 13);
  notes(s, "esp_now_send returning ESP_OK only means the packet was queued. " +
    "Delivery is reported in the callback on the next slide.");

  // ------------------------------------------------------------------ callbacks
  s = T.contentSlide(pres, "Callbacks - Core 2.x Signatures", next());
  T.addCode(s, [
    "static void onSent(const uint8_t *mac, esp_now_send_status_t status) {",
    "  sendOk = (status == ESP_NOW_SEND_SUCCESS);   // volatile bool",
    "  sendDone = true;                 // setup() waits for this flag",
    "}",
    "static void onRecv(const uint8_t *mac, const uint8_t *data, int len) {",
    "  // copy, check len, set a flag - nothing slow here",
    "}",
    "esp_now_register_send_cb(onSent);",
    "esp_now_register_recv_cb(onRecv);",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.7, 10);
  T.addCallout(pres, s, "warn", "Same trap as ledcAttach: examples online are core 3.x",
    "In core 3.x onRecv gets (const esp_now_recv_info_t *info, ...). It does not compile here. " +
    "The callbacks run in the WiFi task: no Serial bursts, no delay(), no blocking.",
    T.MARGIN, 4.0, T.CONTENT_W, 1.05);
  notes(s, "Same discipline as the timer ISR in session 11: record, set a flag, work elsewhere. " +
    "These are not ISRs, but blocking them blocks the WiFi task.");

  // ------------------------------------------------------------------ payload
  s = T.contentSlide(pres, "One Message Format on Both Sides", next());
  T.addCode(s, [
    "constexpr uint8_t MSG_VERSION = 1;",
    "",
    "struct __attribute__((packed)) NodeMsg {",
    "  uint8_t  version;     // MSG_VERSION",
    "  uint8_t  flags;       // bit 0: sensor ok",
    "  uint16_t seq;         // counts every send",
    "  int16_t  t_c10;       // temperature x 10 (215 = 21.5 C)",
    "  uint16_t rh_10;       // humidity x 10",
    "  uint16_t awake_ms;    // how long the previous wake took",
    "  char     name[9];     // \"esp-xxxx\"",
    "};",
    "static_assert(sizeof(NodeMsg) == 19, \"layout changed\");",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.45, 10);
  T.addBullets(s, [
    "The same header (msg.h) in node and gateway, byte for byte",
    "packed: no padding bytes between fields",
    "Integers in tenths instead of float",
    "version first: an old node is recognised, not misread",
    "Receiver checks len == sizeof(NodeMsg)",
  ], 6.6, T.BODY_TOP, 2.9, 3.6, 12);
  notes(s, "This is the struct the Lab 3 node sends. The static_assert fails the build if someone " +
    "adds a field on one side only. Ask: why not send the text \"21.5 C\"? (size, parsing, units).");

  // ------------------------------------------------------------------ channel rule
  s = T.contentSlide(pres, "The Channel Rule", next());
  addFlow(pres, s, [
    { title: "Node", body: "finds the router's channel by scanning once, keeps it in RTC memory" },
    { title: "Gateway", body: "joined to the router: its channel is the router's channel" },
    { title: "Browser", body: "page from the gateway: every node, its last packet and time" },
  ], T.BODY_TOP + 0.05, 1.5, [1]);
  T.addBullets(s, [
    "A board joined to WiFi sends and listens on the router's channel - ESP-NOW peers must use it too",
    "The node never joins the network: it only borrows the channel number (the SSID, no password)",
    "3 failed sends in a row -> scan again on the next wake (the router may have moved)",
    "Gateway: WiFi.setSleep(false), or modem sleep makes it miss packets",
  ], T.MARGIN, 2.85, T.CONTENT_W, 2.2, 13);
  notes(s, "The channel is set with esp_wifi_set_channel(). Scanning costs about 2 s of awake time, " +
    "which is why it happens once and the result goes into RTC memory. The gateway prints its MAC " +
    "and channel at start; both go on the whiteboard.");

  // ------------------------------------------------------------------ lab 3
  s = T.contentSlide(pres, "Lab 3 - A Sleeping Sensor Node", next());
  addFlow(pres, s, [
    { title: "Wake", body: "timer, 20 s" },
    { title: "Read", body: "AHT10" },
    { title: "Send", body: "ESP-NOW to gateway" },
    { title: "Wait", body: "send callback" },
    { title: "Sleep", body: "awake ms -> RTC" },
  ], T.BODY_TOP + 0.05, 0.95, [2]);
  T.addBullets(s, [
    "Open M4_L19/lab3-node. In include/class_net.h: CLASS_SSID and GATEWAY_MAC from the whiteboard",
    "Upload, watch the monitor: channel found, delivery SUCCESS / FAIL",
    "Open the gateway page: your board, your temperature, your awake_ms",
    "Unplug the gateway for a minute: what does your node print? What does it do next?",
  ], T.MARGIN, 2.25, T.CONTENT_W, 1.8, 13);
  T.addCallout(pres, s, "info", "Compare with Lab 2",
    "awake_ms on the page vs your cold-start WiFi time. Same job, two ways to get the number out.",
    T.MARGIN, 4.15, T.CONTENT_W, 0.85);
  notes(s, "The instructor's board runs lab3-gateway on the class router. Write its MAC and the " +
    "router SSID on the whiteboard before the lab. 30 min plus 10 min discussion. " +
    "Unplugged gateway: sends FAIL, after 3 fails the node rescans on the next wake.");

  // ------------------------------------------------------------------ homework
  s = T.contentSlide(pres, "Homework 19 - A Clock That Sleeps", next());
  T.addBullets(s, [
    "Cold start: WiFi -> NTP -> WiFi off -> deep sleep",
    "Wakes by itself every 10 min, blinks the green LED, prints the exact local time",
    "Button A wakes it at once - and the next 10 min count from the press",
    "Before every sleep: print the time of the next wake",
    "Once an hour: WiFi on, resync NTP, print how far the RTC clock drifted",
    "One board, at home, no gateway needed",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 3.0, 14);
  T.addCallout(pres, s, "warn", "Details in M4_L19/homework/README.md",
    "Interval and resync period as named constants. Let it run overnight and paste the drift lines into the commit message.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.9);
  notes(s, "Everything needed is in Lab 1 and Lab 2. The new parts: the drift measurement and " +
    "deciding on each wake whether a resync is due (RTC memory again).");

  // ------------------------------------------------------------------ summary
  T.summarySlide(pres, next(), [
    "Deep sleep is a reboot: RTC memory and the RTC timer survive, nothing else",
    "The awake time decides the battery life, not the sleep current",
    "NTP once, then the RTC keeps the time - set TZ again after every wake, resync for drift",
    "ESP-NOW: no network to join, milliseconds awake, MAC-level ACK, same channel as the gateway",
  ], "A clock that sleeps: wake every 10 min or on button A, exact time on Serial, hourly NTP resync with drift.");

  await T.save(pres, "lesson19.pptx");
  console.log("OK: lesson19.pptx, " + NUM.n + " slides");
})().catch((e) => {
  console.error("BUILD FAILED:", e.message);
  process.exit(1);
});
