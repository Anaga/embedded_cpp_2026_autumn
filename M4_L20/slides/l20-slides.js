// Lesson 20 - MQTT and network diagnosis. Build: node l20-slides.js
const T = require("./slides-theme");
const { FaServer, FaBroadcastTower, FaMicrochip, FaSearch, FaLaptop, FaThermometerHalf } = require("react-icons/fa");

const M = T.MARGIN, W = T.CONTENT_W, TOP = T.BODY_TOP;

(async () => {
  const pres = T.newDeck("Lesson 20 - MQTT and Network Diagnosis",
    "Lesson 20  |  MQTT and Network Diagnosis");
  const icBroker = await T.icon(FaServer, "#CADCFC");
  const icPub = await T.icon(FaMicrochip, "#4CAF50");
  const icSub = await T.icon(FaLaptop, "#FFB300");
  const icTower = await T.icon(FaBroadcastTower, "#CADCFC");
  const icSearch = await T.icon(FaSearch, "#FFB300");
  const icTherm = await T.icon(FaThermometerHalf, "#4CAF50");
  let n = 1;
  let s;

  // 1 -----------------------------------------------------------------------
  s = T.titleSlide(pres, {
    tag: "LESSON 20",
    title: "MQTT and Network Diagnosis",
    subtitle: "Our own broker, the last will, and finding the fault",
    course: "Embedded Software Development in C/C++",
    meta: "ESP32-C3 Super Mini  |  Mosquitto  |  MQTT Explorer",
    iconData: icTower,
  });
  s.addNotes("Session 20, Mon 2026-11-09. Booked to M5: the diagnosis lab is the reason. Bring the router, laptop with Mosquitto installed, MQTT Explorer open.");

  // 2 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "Today", ++n);
  T.addTable(s, ["Part", "What", "Min"], [
    ["0", "Homework 19: the sleeping clock, RTC drift overnight", "15"],
    ["1", "MQTT properly: topics, QoS, retained, keepalive, last will", "35"],
    ["2", "Our own broker: Mosquitto, its log, MQTT Explorer", "20"],
    ["3", "Lab 1: AHT10 node on the class broker", "45"],
    ["4", "Diagnosis method and PubSubClient states", "10"],
    ["5", "Lab 2: three bug reports, find the fault", "45"],
    ["6", "Summary and homework", "10"],
  ], M, TOP + 0.1, W, [0.8, 7.0, 1.2], 13);
  s.addNotes("Total 180 min. If lab 1 runs long, shorten the debrief, not lab 2: lab 2 is what books this session to M5.");

  // 3 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "Homework 19: How Much Did Your Clock Drift?", ++n);
  T.addBullets(s, [
    "Show the drift lines from your commit message: ms per hourly resync",
    "Positive or negative? Did it change during the night?",
    "Where does it come from: the internal 136 kHz RC oscillator, not a crystal",
    "Why it moves: temperature - was the board near a window or a radiator?",
    "Did the button wake restart the 10 min interval as the spec says?",
  ], M, TOP, W, 2.7);
  T.addCallout(pres, s, "info", "Connection to today",
    "The clock had to go online once an hour to fix its time. Today the device stays online - and we learn what to do when it cannot.",
    M, 4.0, W, 0.95);
  s.addNotes("15 min. Collect one drift number per student on the board. Typical RC drift is seconds per hour, so ms values of several thousand are normal. Ask who saw a deep-sleep board reconnect to the monitor without trouble.");

  // 4 -----------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 1", title: "MQTT Properly", note: "What session 18 skipped" });
  n++;

  // 5 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "What We Already Did in Session 18", ++n);
  T.addCards(pres, s, [
    { title: "Worked", body: "Board published its status every 5 s to broker.hivemq.com and switched the LED from gamma-ed/2026/<id>/led", accent: T.GREEN },
    { title: "Public broker", body: "Anyone on the internet could read your topics and switch your LED. No control, no log to look at.", accent: T.AMBER },
    { title: "Silent death", body: "Unplug a board and its last status stays on the broker. Nobody can tell a dead node from a quiet one.", accent: T.RED },
  ], TOP + 0.15, 2.6);
  T.addBullets(s, ["Today: our own broker on the class router, a status that turns \"offline\" by itself, and tools to see what is on the wire"],
    M, 4.05, W, 0.8, 14);
  s.addNotes("Quick recap, 3 min. Ask: who tried the MQTT extra in homework 18?");

  // 6 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "Publish / Subscribe: the Broker in the Middle", ++n);
  T.addCards(pres, s, [
    { title: "Publisher", body: "Sends a message to a topic. Does not know who listens - maybe nobody.", iconData: icPub, accent: T.GREEN },
    { title: "Broker", body: "Receives every message and forwards it to every client subscribed to a matching topic.", iconData: icBroker, accent: T.ICE },
    { title: "Subscriber", body: "Tells the broker which topics it wants. Does not know who sends.", iconData: icSub, accent: T.AMBER },
  ], TOP + 0.1, 2.75);
  T.addBullets(s, [
    "Clients never talk to each other - only to the broker, over one TCP connection each (port 1883)",
    "One client can publish and subscribe at the same time: our node does both",
  ], M, 4.0, W, 0.95, 13);
  s.addNotes("Contrast with UART in session 12: there the two ends knew each other and the protocol decided who talks first. Here nobody addresses anybody.");

  // 7 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "Topics and Wildcards", ++n);
  T.addTable(s, ["Pattern", "Matches", "Does not match"], [
    ["gamma-ed/2026/esp-9494/aht10", "exactly that topic", "Gamma-ed/2026/esp-9494/aht10"],
    ["gamma-ed/2026/+/aht10", "aht10 of every node", "gamma-ed/2026/esp-9494/led"],
    ["gamma-ed/2026/esp-9494/#", "everything of one node", "gamma-ed/2026/esp-aab8/led"],
    ["gamma-ed/2026/#", "the whole class", "gamma_ed/2026/esp-9494/led"],
  ], M, TOP + 0.1, W, [3.4, 2.4, 3.2], 12);
  T.addBullets(s, [
    "Levels are separated by /.  + is exactly one level,  # is the rest (only at the end)",
    "Wildcards are for subscribing only. You publish to one exact topic",
    "Case-sensitive, no wildcard for typos: the broker never says \"no such topic\"",
  ], M, 3.1, W, 1.6, 13);
  s.addNotes("A topic exists the moment someone publishes to it; a subscription to a misspelled topic is perfectly valid and simply never receives anything.");

  // 8 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "Our Topics Today", ++n);
  T.addTable(s, ["Topic  (<p> = gamma-ed/2026)", "Direction", "Payload", "Retained"], [
    ["<p>/<id>/status", "node -> all", "online / offline", "yes"],
    ["<p>/<id>/aht10", "node -> all, every 5 s", "{\"t\":21.8,\"rh\":58.1,\"seq\":12}", "no"],
    ["<p>/<id>/led/set", "anyone -> node", "1 / 0", "never"],
    ["<p>/<id>/led", "node -> all", "1 / 0 (the real state)", "yes"],
  ], M, TOP + 0.1, W, [2.6, 2.1, 3.1, 1.2], 12);
  T.addBullets(s, [
    "<id> is esp-xxxx from the last two MAC bytes - no editing per student",
    "Command and state are separate topics: led/set asks, led reports what happened",
    "seq counts up by one per reading: a gap in MQTT Explorer means a lost message",
  ], M, 3.1, W, 1.6, 13);
  s.addNotes("Client id is gamma-ed-esp-xxxx. Ask why we do not just retain led/set (answer on the retained slide).");

  // 9 -----------------------------------------------------------------------
  s = T.contentSlide(pres, "Quality of Service", ++n);
  T.addTable(s, ["QoS", "Promise", "Cost", "PubSubClient"], [
    ["0", "at most once - fire and forget", "1 packet", "publish: yes (only this)"],
    ["1", "at least once - repeated until PUBACK", "2 packets, duplicates possible", "subscribe: yes"],
    ["2", "exactly once - four-way handshake", "4 packets", "no"],
  ], M, TOP + 0.1, W, [0.7, 3.3, 2.6, 2.4], 12);
  T.addBullets(s, [
    "QoS is per hop: node -> broker, then broker -> each subscriber, each with its own QoS",
    "On a LAN with TCP, QoS 0 loses messages mostly when a connection dies",
    "QoS does not tell the sender that the application understood the message - that is what our led state echo is for",
  ], M, 2.75, W, 1.9, 13);
  s.addNotes("PubSubClient publishes QoS 0 only and subscribes with QoS 0 or 1. We subscribe led/set with QoS 1 and publish the last will with QoS 1.");

  // 10 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Retained Messages", ++n);
  T.addBullets(s, [
    "Publish with retain = true: the broker keeps the LAST message of that topic",
    "A new subscriber gets it immediately, without waiting for the next publish",
    "Right for state: status, led, a setpoint. The board can be asked \"what is it now?\"",
    "Wrong for commands and events: a retained led/set fires again on every reconnect",
    "To delete: publish an empty retained message to the same topic",
    "Our Mosquitto runs with persistence false: a broker restart forgets all retained messages",
  ], M, TOP, W, 3.2, 14);
  T.addCallout(pres, s, "warn", "MQTT Explorer",
    "Retained topics are marked in the tree. The trash icon next to a topic publishes the empty retained message for you.",
    M, 4.05, W, 0.9);
  s.addNotes("Ask: what should a node do after a broker restart so that its status is right again? (Publish online again after every connect - lab1-node does.)");

  // 11 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Keepalive and the Last Will", ++n);
  T.addCode(s, [
    "// connect(id, willTopic, willQos, willRetain, willMessage)",
    "mqtt.setKeepAlive(MQTT_KEEPALIVE_S);   // 15 s",
    "mqtt.connect(clientId, topicStatus, QOS_1, true, \"offline\");",
    "mqtt.publish(topicStatus, \"online\", true);",
  ].join("\n"), M, TOP + 0.05, W, 1.55, 12);
  T.addBullets(s, [
    "The will is handed to the broker at connect time and stays there",
    "Nothing to send for 15 s? mqtt.loop() sends PINGREQ, the broker answers PINGRESP",
    "Silence for 1.5 x keepalive (about 22 s): the broker publishes the will for us",
    "A clean disconnect() does NOT trigger the will - only an unexpected death does",
    "Client id must be unique: a second client with the same id kicks the first one off",
  ], M, 2.85, W, 2.15, 13);
  s.addNotes("Unique id symptom: two boards with the same id reconnect to each other's disadvantage every few seconds, and the log shows 'already connected, closing old connection'. We build the id from the MAC, so it cannot happen today - mention it, do not demo it.");

  // 12 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "The Callback Trap", ++n);
  T.addCode(s, [
    "void onMessage(char *topic, uint8_t *payload, unsigned int length) {",
    "    if (strcmp(topic, topicLedSet) != 0) { return; }",
    "    // payload: inside PubSubClient's buffer, NOT zero-terminated",
    "    const size_t n = (length < CMD_LEN - 1U) ? length : CMD_LEN - 1U;",
    "    memcpy(cmdBuf, payload, n);",
    "    cmdBuf[n] = '\\0';",
    "    cmdPending = true;          // loop() calls handleCommand()",
    "}",
  ].join("\n"), M, TOP + 0.05, W, 2.6, 11);
  T.addCallout(pres, s, "error", "Do not publish inside the callback",
    "publish() reuses the same buffer the payload points to. Copy, set a flag, return - the same rule as an ISR in session 11.",
    M, 3.85, W, 0.95);
  s.addNotes("Two bugs prevented here: reading past the payload end (no terminator) and corrupting the payload by publishing from inside the callback.");

  // 13 ----------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 2", title: "Our Own Broker", note: "Mosquitto on the instructor laptop, MQTT Explorer on yours" });
  n++;

  // 14 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Mosquitto on the Instructor Laptop", ++n);
  T.addCode(s, [
    "# broker/class.conf",
    "listener 1883",
    "allow_anonymous true",
    "persistence false",
    "connection_messages true",
    "",
    "mosquitto -c class.conf -v",
  ].join("\n"), M, TOP + 0.05, 4.2, 2.35, 11);
  T.addBullets(s, [
    "Mosquitto 2.x without a config file listens on localhost only - boards cannot reach it",
    "listener 1883 opens it to the network, allow_anonymous lets clients in without a password",
    "Firewall: allow port 1883 (macOS asks once, Windows Defender needs an inbound rule)",
    "MQTT_HOST in secrets.h = the laptop IP on the class router",
  ], 4.95, TOP, 4.55, 3.7, 13);
  s.addNotes("Write the laptop IP on the board. Anonymous access is acceptable only because the router is ours and offline from the world. Never do this on a broker reachable from the internet.");

  // 15 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Reading the Broker Log", ++n);
  T.addCode(s, [
    "New connection from 192.168.3.43:51234 on port 1883.",
    "New client connected from 192.168.3.43:51234 as gamma-ed-esp-9494 (p2, c1, k15).",
    "Received SUBSCRIBE from gamma-ed-esp-9494",
    "    gamma-ed/2026/esp-9494/led/set (QoS 1)",
    "Received PUBLISH from gamma-ed-esp-9494 (d0, q0, r0, m0, 'gamma-ed/2026/esp-9494/aht10', ... (34 bytes))",
    "Received PINGREQ from gamma-ed-esp-9494",
    "Client gamma-ed-esp-9494 has exceeded timeout, disconnecting.",
  ].join("\n"), M, TOP + 0.05, W, 2.45, 10);
  T.addBullets(s, [
    "k15 = keepalive 15 s.  r0 / r1 = retained.  q0 / q1 = QoS",
    "Every subscription and every topic is printed exactly as the client sent it",
    "The log is evidence: it shows what the broker received, not what the board thinks it sent",
  ], M, 3.7, W, 1.35, 13);
  s.addNotes("Timestamps removed for space; the real log starts every line with a Unix time. Project the live log during lab 1.");

  // 16 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "MQTT Explorer", ++n);
  T.addBullets(s, [
    "New connection: host = laptop IP, port 1883, no user name, then Connect",
    "Topic tree: every topic anyone published, with the latest value",
    "Retained topics are marked; history and a chart per topic (try t of your node)",
    "Publish panel: topic, payload, retain checkbox - this is how you switch the LED",
    "It subscribes to # by default: it sees everything, including topics you did not expect",
  ], M, TOP, W, 3.0, 14);
  T.addCallout(pres, s, "info", "Three evidence sources from now on",
    "Node Serial = what the board thinks.  Broker log = what arrived.  MQTT Explorer = what a subscriber sees.",
    M, 4.05, W, 0.9);
  s.addNotes("Students install MQTT Explorer from mqtt-explorer.com (Mac: the dmg works on Apple Silicon). Give them 5 minutes; most had it from session 18.");

  // 17 ----------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 3", title: "Lab 1: AHT10 Node", note: "M4_L20/lab1-node" });
  n++;

  // 18 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "What lab1-node Does", ++n);
  T.addCards(pres, s, [
    { title: "AHT10", body: "Reading every 5 s through Adafruit AHTX0. SDA GPIO 8, SCL GPIO 9, 3V3 - as in session 18", iconData: icTherm },
    { title: "Node", body: "Publishes status, aht10, led. Subscribes led/set. Reconnects WiFi and MQTT without blocking", iconData: icPub, accent: T.GREEN },
    { title: "Broker", body: "Mosquitto on the instructor laptop. Its log is projected on the wall", iconData: icBroker },
    { title: "You", body: "MQTT Explorer: watch the class, switch your own LED", iconData: icSub, accent: T.AMBER },
  ], TOP + 0.1, 2.75);
  T.addBullets(s, [
    "RGB LED: red = no connection to the broker,  green = led/set command",
    "GPIO 8 and 9 are strapping pins; GPIO 9 is BOOT - do not press it during a reading",
  ], M, 4.0, W, 0.95, 13);
  s.addNotes("Wiring unchanged from session 18; RGB on 5/6/7 common anode. The blue on-board LED flickering is I2C traffic on GPIO 8.");

  // 19 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "The Loop: Nothing Waits", ++n);
  T.addCode(s, [
    "void loop() {",
    "    const uint32_t now = millis();",
    "    watchWifi();        // prints WiFi changes",
    "    watchMqtt(now);     // connect at most every 5 s",
    "    mqtt.loop();        // keepalive, messages -> onMessage()",
    "    handleCommand();    // the flag from the callback",
    "    if (now - lastPublishMs >= PUBLISH_PERIOD_MS) {",
    "        lastPublishMs = now;",
    "        publishReading();",
    "    }",
    "    setChannel(PIN_RED, link != Link::Online);",
    "}",
  ].join("\n"), M, TOP + 0.05, 5.4, 3.5, 10);
  T.addBullets(s, [
    "enum class Link: NoWifi, NoBroker, Online",
    "mqtt.connect() is the one call that can block for seconds - so it is rate-limited",
    "Without mqtt.loop() no keepalive: the broker publishes your will after 22 s",
  ], 6.1, TOP + 0.1, 3.4, 3.4, 13);
  s.addNotes("Same IDLE / WAITING idea as session 12, now with three states. Ask what happens if publishReading() is called while link is NoBroker (it still prints, skips the publish).");

  // 20 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Known-Good Output", ++n);
  T.addCode(s, [
    "Lab 1 - MQTT node esp-9494, client id gamma-ed-esp-9494",
    "AHT10: found at 0x38",
    "MQTT: broker 192.168.3.10:1883",
    "  publish   gamma-ed/2026/esp-9494/status, gamma-ed/2026/esp-9494/aht10, gamma-ed/2026/esp-9494/led",
    "  subscribe gamma-ed/2026/esp-9494/led/set",
    "WiFi: connected, IP 192.168.3.43, -55 dBm",
    "MQTT: connected",
    "esp-9494 | WiFi -55 dBm | MQTT ok | 21.8 C  58.1 % | LED off | seq 1",
    "LED: on",
  ].join("\n"), M, TOP + 0.05, W, 2.75, 10);
  T.addCallout(pres, s, "warn", "Save it",
    "Copy your own Serial output and a screenshot of your node in MQTT Explorer into a notes file. Lab 2 compares against this.",
    M, 4.05, W, 0.9);
  s.addNotes("The broker IP on the slide is an example; use the real one. A diagnosis without a known-good reference is guessing.");

  // 21 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Lab 1: Try It", ++n);
  T.addBullets(s, [
    "Copy include/secrets.example.h to secrets.h, fill in WiFi and the broker IP, upload",
    "Find your node in MQTT Explorer. status must be online, retained",
    "Publish 1 to <p>/<id>/led/set. Green on? led follows?",
    "Publish \"maybe\" to led/set. What does Serial say?",
    "Pull the USB cable and start a stopwatch: when does status turn offline? Why that long?",
    "Plug it back in: who sets status to online again?",
  ], M, TOP, W, 3.5, 14);
  s.addNotes("Expected offline after about 22 s (1.5 x 15 s). The node republishes online itself after connect. If a board does not connect: check MQTT_HOST, the laptop firewall, and WIFI_POWER_8_5dBm.");

  // 22 ----------------------------------------------------------------------
  T.sectionSlide(pres, { kicker: "PART 4", title: "Network Diagnosis", note: "Symptom -> hypothesis -> evidence -> conclusion" });
  n++;

  // 23 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "The Method, as in Session 15", ++n);
  T.addCards(pres, s, [
    { title: "Symptom", body: "What exactly is wrong, from the bug report. Who sees it? Since when?", accent: T.RED },
    { title: "Hypothesis", body: "WiFi? Broker? Topic? Payload? Write two or three, not one", accent: T.AMBER },
    { title: "Evidence", body: "Serial, broker log, MQTT Explorer, ping. Compare with your known-good", iconData: icSearch, accent: T.ICE },
    { title: "Conclusion", body: "One sentence that explains every observation. Then the fix", accent: T.GREEN },
  ], TOP + 0.1, 2.8);
  T.addBullets(s, [
    "Split the path: WiFi -> TCP to the broker -> MQTT session -> topic -> payload. Find the first link that fails",
  ], M, 4.1, W, 0.8, 13);
  s.addNotes("In session 15 the evidence came from the analyzer. Here there is no wire to clip onto, so the broker log and MQTT Explorer play that role.");

  // 24 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "PubSubClient States", ++n);
  T.addTable(s, ["state()", "Meaning", "First suspect"], [
    ["-4", "timeout: no answer in time", "broker overloaded, weak WiFi"],
    ["-3", "connection lost", "broker stopped, WiFi dropped"],
    ["-2", "TCP connect failed", "broker down, wrong IP or port, firewall"],
    ["-1", "disconnected cleanly", "your own disconnect()"],
    ["0", "connected", "not the connection - look at topics"],
    ["2", "client id rejected", "id empty or not allowed"],
    ["4 / 5", "bad credentials / not authorized", "user name, password, ACL"],
  ], M, TOP + 0.05, W, [1.2, 3.6, 4.2], 12);
  s.addNotes("lab1-node prints the number and the text on every failed attempt. State 0 with a missing message is the most interesting row.");

  // 25 ----------------------------------------------------------------------
  s = T.contentSlide(pres, "Lab 2: Three Bug Reports", ++n);
  T.addBullets(s, [
    "The instructor flashes a mystery firmware on your board, or changes something in the class network",
    "You get a bug report. You do not get the source code",
    "Write in your notes: symptom, hypotheses, evidence (where you saw it), conclusion",
    "Evidence must be something you can show: a Serial line, a log line, a topic in MQTT Explorer",
    "Afterwards re-upload lab1-node and check it against your known-good output",
  ], M, TOP, W, 3.0, 14);
  T.addCallout(pres, s, "info", "Report format",
    "Case: ...  Symptom: ...  Hypotheses: ...  Evidence: ...  Conclusion: ...  Fix: ...",
    M, 4.05, W, 0.9);
  s.addNotes("Bug reports, flashing commands and the answer key: solution/lab2-mystery/INSTRUCTOR.md in embedded-cpp-materials. The deck goes to the cohort repository, so nothing about the faults is written here.");

  // 26 ----------------------------------------------------------------------
  s = T.summarySlide(pres, ++n, [
    "Broker in the middle: topics, + and #, case-sensitive, no error for a wrong topic",
    "QoS per hop; PubSubClient publishes QoS 0 only",
    "Retained for state, never for commands; last will turns a dead node into \"offline\"",
    "Callback: copy, flag, return - work in loop()",
    "Diagnosis: Serial, broker log, MQTT Explorer - compare with a known-good",
  ], "MQTT thermostat at home: retained setpoint 15.0 .. 30.0, RGB HEAT / OK / COOL, mode topic, last will - plus a short report on one fault you caused yourself. Details: homework/README.md");
  s.addNotes("Homework works with broker.hivemq.com or Mosquitto at home (brew install mosquitto, or the Windows installer) with broker/class.conf.");

  await T.save(pres, "M4_L20-mqtt-diagnosis.pptx");
  console.log("OK", n, "slides");
})().catch((e) => { console.error(e.message); process.exit(1); });
