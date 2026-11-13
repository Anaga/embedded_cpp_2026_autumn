/*
 * lesson21-slides.js
 * Lesson 21 - CAN bus through TWAI and bus diagnosis (ESP32-C3, TJA1050).
 * Build: node lesson21-slides.js  ->  lesson21-can-twai.pptx
 */

const T = require("./slides-theme");
const { FaNetworkWired, FaCarSide, FaMicrochip, FaSearch } = require("react-icons/fa");

(async () => {
  const pres = T.newDeck(
    "Lesson 21 - CAN bus through TWAI",
    "Lesson 21  |  CAN bus through TWAI and bus diagnosis"
  );

  const iconNet = await T.icon(FaNetworkWired, "#CADCFC");
  const iconCar = await T.icon(FaCarSide, "#CADCFC");
  const iconChip = await T.icon(FaMicrochip, "#CADCFC");
  const iconSearch = await T.icon(FaSearch, "#FFB300");

  let n = 1;
  let s;
  const SRC = "Diagram: come-star.com/blog/can-bus-wiki";
  function addPic(slide, file, x, y, w, h) {
    slide.addImage({ path: "img/" + file, x: x, y: y, w: w, h: h, rounding: false });
    slide.addText(T.assertAscii(SRC, "image source"), {
      x: x, y: y + h + 0.02, w: w, h: 0.22, fontSize: 8, fontFace: T.BODY,
      color: T.GRAY, italic: true, align: "right", margin: 0,
    });
  }

  // 1 ---------------------------------------------------------------- title
  T.titleSlide(pres, {
    tag: "LESSON 21",
    title: "CAN bus through TWAI",
    subtitle: "Two ESP32-C3 nodes, TJA1050 transceivers, and a logic analyzer on the bus",
    course: "Embedded Software Development in C/C++",
    meta: "Module M5  |  Fri 2026-11-13  |  4 academic hours",
    iconData: iconNet,
  });
  n++;

  // 2 ---------------------------------------------------------------- goals
  s = T.contentSlide(pres, "Tonight", n++);
  T.addBullets(s, [
    "Why CAN exists and where it runs: cars, trucks, industrial machines, drones",
    "Physical layer: two wires, dominant and recessive, 120 ohm at both ends",
    "Frame anatomy, arbitration by ID, acknowledge, error counters",
    "TWAI = Espressif's name for the CAN controller inside the ESP32-C3",
    "Lab 1: two nodes on one bus, heartbeat frames, LED command frames",
    "Lab 2: break the bus on purpose and read the damage in PulseView",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 3.0);
  T.addCallout(pres, s, "info", "Last technical session",
    "Monday: cross-platform comparison and final project approval. Bring your project idea in writing.",
    T.MARGIN, 4.25, T.CONTENT_W, 0.75);

  // ================================================================ PART 1
  T.sectionSlide(pres, { kicker: "PART 1", title: "What CAN is", note: "A bus built for machines that must not stop talking" });
  n++;

  // 4 ---------------------------------------------------------------- why CAN
  s = T.contentSlide(pres, "Why CAN", n++);
  T.addCards(pres, s, [
    { title: "Multi-master", body: "No master, no polling. Any node sends when the bus is idle. Collisions are resolved by priority, not by retry.", iconData: iconNet },
    { title: "Robust", body: "Differential pair, 15-bit CRC, every receiver acknowledges, faulty nodes remove themselves from the bus.", iconData: iconCar },
    { title: "Cheap", body: "Two wires, one transceiver per node, the controller is inside the MCU. Born at Bosch in 1986 for cars; now everywhere.", iconData: iconChip },
  ], 1.3, 2.7);
  T.addCallout(pres, s, "info", "Compared with what you already know",
    "UART: two nodes only. I2C and SPI: one master, centimetres. CAN: up to 40 m at 1 Mbit/s, dozens of nodes, no master.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // 5 ---------------------------------------------------------------- physical layer
  s = T.contentSlide(pres, "Physical layer: CAN_H and CAN_L", n++);
  addPic(s, "differential.jpg", T.MARGIN, T.BODY_TOP, 4.6, 1.93);
  T.addTable(s,
    ["State", "Bit", "CAN_H", "CAN_L", "Diff"],
    [
      ["Recessive", "1", "2.5 V", "2.5 V", "0 V"],
      ["Dominant", "0", "3.5 V", "1.5 V", "2 V"],
    ],
    5.4, T.BODY_TOP + 0.3, 4.1, [1.3, 0.5, 0.8, 0.8, 0.7], 11);
  T.addBullets(s, [
    "Dominant always wins: one node pulling 0 overrides every node sending 1 (wired-AND)",
    "The transceiver (TJA1050) turns the MCU's TXD / RXD into the differential pair; twisted pair cable, short stubs",
    "120 ohm at each END of the bus: two resistors, 60 ohm measured between CAN_H and CAN_L",
  ], T.MARGIN, 3.35, T.CONTENT_W, 1.1, 12);
  T.addCallout(pres, s, "warn", "Measure before power",
    "Our modules have 120 ohm on board (R1, marked 121): one module ~120 ohm, two wired together ~60 ohm.",
    T.MARGIN, 4.3, T.CONTENT_W, 0.7);

  // 6 ---------------------------------------------------------------- frame
  s = T.contentSlide(pres, "A classic CAN frame (11-bit ID)", n++);
  addPic(s, "frame.jpg", T.MARGIN, T.BODY_TOP, T.CONTENT_W, 3.09);
  T.addCallout(pres, s, "info", "You never build this by hand",
    "The controller does it all. Your code fills ID, DLC and up to 8 data bytes (twai_message_t); the rest is hardware.",
    T.MARGIN, 4.45, T.CONTENT_W, 0.6);

  s = T.contentSlide(pres, "Frame fields", n++);
  T.addTable(s,
    ["Field", "Bits", "What it is"],
    [
      ["SOF", "1", "Start of frame, one dominant bit"],
      ["Identifier", "11", "Message ID and priority: 0x000 .. 0x7FF, lower wins"],
      ["RTR + IDE + r0", "3", "Remote request, extended-ID flag (0 = 11-bit), reserved"],
      ["DLC", "4", "Data length 0 .. 8 bytes"],
      ["Data", "0 .. 64", "The payload"],
      ["CRC + delimiter", "16", "15-bit CRC computed by the controller"],
      ["ACK + delimiter", "2", "Sender sends 1, any receiver that liked the CRC writes 0"],
      ["EOF + IFS", "10", "Seven recessive bits, then three bits of silence"],
    ],
    T.MARGIN, T.BODY_TOP, T.CONTENT_W, [2.0, 1.2, 5.8], 11);
  T.addCallout(pres, s, "info", "Bit stuffing",
    "After five equal bits the controller inserts one opposite bit, the receiver drops it. Frame length on the analyzer varies.",
    T.MARGIN, 4.42, T.CONTENT_W, 0.62);

  // 7 ---------------------------------------------------------------- arbitration
  s = T.contentSlide(pres, "Arbitration: the lower ID wins, bit by bit", n++);
  T.addCode(s, [
    "bit:      10 9 8 7 6 5 4 3 2 1 0",
    "node A    0  0  1  0  0  0  0  0  0  0  1   ID 0x101",
    "node B    0  0  1  0  0  0  0  0  0  1  0   ID 0x102",
    "node C    0  0  1  1  0  0  0  0  0  0  0   ID 0x180",
    "bus       0  0  1  0  .  .  .  .  .  .  .",
    "                    ^ C sent 1, read 0 -> C stops, retries later",
    "                                   ^ B sent 1, read 0 -> B stops",
    "A finishes its frame; B and C send right after, in that order",
  ].join("\n"), T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.45, 10,
    "Every sender reads the bus while writing the ID; the first node to see a 0 where it wrote 1 backs off");
  T.addBullets(s, [
    "No data is lost in a collision: the loser just waits one frame",
    "Consequence: ID is priority. Give the alarm 0x010 and the chatter 0x500",
    "Two nodes must never send the same ID - arbitration cannot separate them",
  ], T.MARGIN, 3.95, T.CONTENT_W, 1.1, 13);

  // 8 ---------------------------------------------------------------- ACK and errors
  s = T.contentSlide(pres, "ACK and the error state machine", n++);
  T.addBullets(s, [
    "ACK: every receiving node overwrites the ACK slot with 0, even if the frame is not for it",
    "No ACK = nobody heard me. The controller retransmits at once, forever, and counts an error",
    "Two counters per node: TEC (transmit errors) and REC (receive errors), +8 per error, -1 per good frame",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 1.4, 14);
  T.addTable(s,
    ["State", "Condition", "Behaviour"],
    [
      ["Error active", "TEC < 128 and REC < 128", "Normal. Signals errors with 6 dominant bits (error flag)"],
      ["Error passive", "TEC or REC >= 128", "Still talks, but may not shout: passive error flag, waits longer"],
      ["Bus off", "TEC > 255", "Off the bus. Needs explicit recovery (128 x 11 recessive bits)"],
    ],
    T.MARGIN, 2.65, T.CONTENT_W, [1.7, 2.6, 4.7], 11);
  T.addCallout(pres, s, "warn", "This is tonight's diagnosis lab",
    "A lonely node, a wrong bit rate, a pulled RXD wire: all end in error passive or bus off. You will watch it happen.",
    T.MARGIN, 4.3, T.CONTENT_W, 0.7);

  // 9 ---------------------------------------------------------------- bit timing
  s = T.contentSlide(pres, "Bit rate and sample point", n++);
  T.addBullets(s, [
    "Everyone on the bus must use the same bit rate. Tonight: 500 kbit/s, 2 us per bit",
    "Each bit is cut into time quanta; the receiver samples at about 75-87 % of the bit",
    "Resynchronisation on every recessive-to-dominant edge; bit stuffing (a 6th equal bit is inserted) keeps edges coming",
    "TWAI hides this: TWAI_TIMING_CONFIG_500KBITS() fills the whole struct",
    "Wrong rate on one node: it sees garbage, flags errors on every frame, and poisons the bus for everyone",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.8, 14);
  T.addCallout(pres, s, "info", "Analyzer math from session 15",
    "500 kbit/s x 10 = 5 MHz sampling is plenty; 8 MHz in PulseView. One second of capture = 8 M samples, fine over USB.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // ================================================================ PART 2
  T.sectionSlide(pres, { kicker: "PART 2", title: "Hardware", note: "TJA1050 is a 5 V part. The ESP32-C3 is not." });
  n++;

  // 11 --------------------------------------------------------------- wiring
  s = T.contentSlide(pres, "TJA1050 module to the ESP32-C3 Super Mini", n++);
  T.addTable(s,
    ["TJA1050 pin", "Goes to", "Note"],
    [
      ["VCC", "C3 5V pin (USB 5 V)", "The transceiver needs 4.75 .. 5.25 V. Not 3V3"],
      ["GND", "C3 GND", "Common ground with every other node too"],
      ["TXD", "C3 GPIO 10", "3.3 V high; the module has 10 ohm (R2) in series. Works in practice"],
      ["RXD", "divider -> C3 GPIO 3", "Swings to 5 V. 10k series, 20k to GND -> 3.3 V"],
      ["CAN_H / CAN_L", "the bus", "Same wire colour for H on every module"],
    ],
    T.MARGIN, T.BODY_TOP, T.CONTENT_W, [2.0, 2.6, 4.4], 11);
  T.addCode(s, [
    "TJA1050 RXD --[ 10k ]--+-- C3 GPIO 3  (5 x 20/30 = 3.33 V)",
    "                         |",
    "                       [ 20k ]",
    "                         |",
    "                        GND",
  ].join("\n"), T.MARGIN, 3.25, 5.6, 1.6, 10);
  T.addCallout(pres, s, "error", "No divider = dead pin",
    "C3 pins are rated 3.6 V max. Check the divider with the multimeter before connecting GPIO 3.",
    6.3, 3.25, 3.2, 1.6);

  // 12 --------------------------------------------------------------- pins and topology
  s = T.contentSlide(pres, "Pin budget and bus topology", n++);
  addPic(s, "topology.jpg", 5.3, T.BODY_TOP, 4.2, 2.3);
  T.addBullets(s, [
    "Kept from sessions 18-20: buttons 0 / 1, pot 4, RGB 5 / 6 / 7, AHT10 on 8 / 9",
    "New: CAN TX GPIO 10, CAN RX GPIO 3. Both were free. 2 is strapping, 20 / 21 are UART0",
    "Real bus: a line with 120 ohm at both ends, nodes on short stubs (picture)",
    "Tonight: two modules = the two ends. Both carry their own 120 ohm, nothing to add",
  ], T.MARGIN, T.BODY_TOP, 4.7, 2.5, 12);
  T.addCode(s, [
    "  node 1 (120R on board)            node 2 (120R on board)",
    "  C3 --- TJA1050  CAN_H ========== CAN_H  TJA1050 --- C3",
    "                  CAN_L ========== CAN_L",
    "  GND ------------------------------------------------ GND",
  ].join("\n"), T.MARGIN, 3.55, T.CONTENT_W, 1.35, 10,
    "One common GND between the two boards, even though CAN is differential. Station 3 = the logic analyzer");

  // ================================================================ PART 3
  T.sectionSlide(pres, { kicker: "PART 3", title: "TWAI in code", note: "driver/twai.h from ESP-IDF, available under Arduino core 2.x" });
  n++;

  // 14 --------------------------------------------------------------- lifecycle
  s = T.contentSlide(pres, "Driver lifecycle: configure, install, start", n++);
  T.addCode(s, [
    "#include \"driver/twai.h\"",
    "",
    "static const gpio_num_t CAN_TX_PIN = GPIO_NUM_10;",
    "static const gpio_num_t CAN_RX_PIN = GPIO_NUM_3;",
    "",
    "void canBegin(void) {",
    "    twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(",
    "        CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);",
    "    twai_timing_config_t t = TWAI_TIMING_CONFIG_500KBITS();",
    "    twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();",
    "",
    "    ESP_ERROR_CHECK(twai_driver_install(&g, &t, &f));",
    "    ESP_ERROR_CHECK(twai_start());",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.95, 10);
  T.addBullets(s, [
    "TWAI = Two-Wire Automotive Interface. Espressif's name, same protocol",
    "Modes: NORMAL, NO_ACK (self-test with one board), LISTEN_ONLY (sniffer)",
    "Filter: accept all tonight; hardware filtering by ID is possible",
    "Stop / uninstall exist too; you need them to change the bit rate",
  ], 6.6, T.BODY_TOP, 3.0, 3.9, 11);

  // 15 --------------------------------------------------------------- transmit
  s = T.contentSlide(pres, "Transmit: fill the struct, hand it to the queue", n++);
  T.addCode(s, [
    "twai_message_t msg = {};",
    "msg.identifier       = 0x100U + NODE_ID;   // 11-bit ID",
    "msg.extd             = 0;                   // standard frame",
    "msg.data_length_code = 4U;",
    "msg.data[0] = (uint8_t)(counter >> 8);",
    "msg.data[1] = (uint8_t)(counter);",
    "msg.data[2] = (uint8_t)(pot >> 8);",
    "msg.data[3] = (uint8_t)(pot);",
    "",
    "esp_err_t err = twai_transmit(&msg, pdMS_TO_TICKS(10U));",
    "if (err != ESP_OK) {",
    "    Serial.printf(\"TX fail: %s\\n\", esp_err_to_name(err));",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.7, 10);
  T.addBullets(s, [
    "twai_transmit() only queues the frame; the controller sends it when the bus is idle and arbitration is won",
    "Timeout 0 = do not block. ESP_ERR_TIMEOUT = queue full, the bus is not draining",
    "Byte order is yours to define: we send big-endian, 2 bytes per value",
    "ESP_ERR_INVALID_STATE = driver not started or bus off",
  ], 6.6, T.BODY_TOP, 3.0, 3.6, 11);

  // 16 --------------------------------------------------------------- receive
  s = T.contentSlide(pres, "Receive: poll the queue in loop(), no blocking", n++);
  T.addCode(s, [
    "void canPoll(void) {",
    "    twai_message_t rx;",
    "    while (twai_receive(&rx, 0) == ESP_OK) {",
    "        if (rx.identifier == CMD_LED_ID",
    "            && rx.data_length_code == 4U",
    "            && rx.data[0] == NODE_ID) {",
    "            setRgb(rx.data[1], rx.data[2], rx.data[3]);",
    "        } else if (rx.identifier >= HEARTBEAT_BASE_ID",
    "                   && rx.identifier <  HEARTBEAT_BASE_ID + 8U) {",
    "            printHeartbeat(rx);",
    "        }",
    "    }",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.7, 10);
  T.addBullets(s, [
    "The driver has an RX queue (default 5 frames); drain it every loop() pass",
    "A frame is received even if nobody is addressed - CAN is broadcast, addressing is a convention inside the data",
    "rx.rtr, rx.extd, rx.data_length_code: always check DLC before reading data[]",
  ], 6.6, T.BODY_TOP, 3.0, 3.4, 11);

  // 17 --------------------------------------------------------------- status
  s = T.contentSlide(pres, "Status, alerts and bus-off recovery", n++);
  T.addCode(s, [
    "twai_status_info_t st;",
    "twai_get_status_info(&st);",
    "// st.state: TWAI_STATE_RUNNING / BUS_OFF / RECOVERING",
    "// st.tx_error_counter, st.rx_error_counter",
    "// st.msgs_to_tx, st.msgs_to_rx, st.bus_error_count",
    "",
    "uint32_t alerts = 0U;",
    "if (twai_read_alerts(&alerts, 0) == ESP_OK) {",
    "    if (alerts & TWAI_ALERT_BUS_OFF) {",
    "        twai_initiate_recovery();   // then wait for BUS_RECOVERED",
    "    }",
    "    if (alerts & TWAI_ALERT_BUS_RECOVERED) { twai_start(); }",
    "}",
  ].join("\n"), T.MARGIN, T.BODY_TOP, 5.9, 3.7, 10);
  T.addBullets(s, [
    "Enable alerts at install: g.alerts_enabled = TWAI_ALERT_BUS_OFF | ERR_PASS | BUS_ERROR | ...",
    "Error passive and bus off are not crashes: the firmware keeps running, only the bus is gone",
    "The demo prints TEC / REC every 5 s. Watch them climb in Lab 2",
  ], 6.6, T.BODY_TOP, 3.0, 3.6, 11);

  // 18 --------------------------------------------------------------- demo
  s = T.contentSlide(pres, "Lab 1: two nodes, one firmware", n++);
  T.addTable(s,
    ["Frame", "ID", "DLC", "Data", "Who sends"],
    [
      ["Heartbeat", "0x101 / 0x102", "4", "counter (2 B), pot (2 B)", "both nodes, every 1 s"],
      ["LED command", "0x200", "4", "target node, R, G, B", "button A on any node"],
      ["LED off", "0x200", "4", "target node, 0, 0, 0", "button B on any node"],
    ],
    T.MARGIN, T.BODY_TOP, T.CONTENT_W, [1.6, 2.2, 0.7, 2.6, 1.9], 11);
  T.addBullets(s, [
    "NODE_ID comes from platformio.ini: envs node1 / node2 set -D NODE_ID=n. Two stations, a pair per station; the third pair runs the analyzer",
    "Button A targets the OTHER node with a colour that depends on your pot position; button B turns it off",
    "Serial shows every received heartbeat and the TEC / REC counters every 5 s",
    "Env selftest: TWAI_MODE_NO_ACK, one board, no bus - the node hears its own frames. Use it at home",
  ], T.MARGIN, 2.7, T.CONTENT_W, 1.6, 12);
  T.addCallout(pres, s, "ok", "Done when",
    "Both monitors show the other node's heartbeat counting up, and a button on one board lights the LED on the other.",
    T.MARGIN, 4.4, T.CONTENT_W, 0.65);

  // ================================================================ PART 4
  T.sectionSlide(pres, { kicker: "PART 4", title: "Lab 2: Bus diagnosis", note: "Symptom -> hypothesis -> capture -> compare -> conclusion" });
  n++;

  // 20 --------------------------------------------------------------- analyzer
  s = T.contentSlide(pres, "Logic analyzer on the CAN bus", n++);
  T.addBullets(s, [
    "Clip D0 to the C3 side of the RXD divider (3.3 V) - never to the 5 V side, never to CAN_H / CAN_L",
    "RXD shows the whole bus: every frame from every node, including ACK bits and error flags",
    "D1 on TXD of the same node: now you can tell what this node sent from what it heard",
    "PulseView: 8 MHz, 2 s; add decoder CAN, bit rate 500000, channel CAN RX = D0",
    "Record a known-good capture of Lab 1 FIRST and save it. Every fault is compared against it",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 2.7, 14);
  T.addCallout(pres, s, "info", "What a good frame looks like",
    "SOF, ID 0x101, DLC 4, four data bytes, CRC, then one dominant ACK bit written by the other nodes. 2 us per bit, about 110 us per frame.",
    T.MARGIN, 4.1, T.CONTENT_W, 0.9);

  // 21 --------------------------------------------------------------- faults
  s = T.contentSlide(pres, "Injected faults", n++);
  T.addTable(s,
    ["Fault", "Symptom in Serial", "Evidence in the capture", "Why"],
    [
      ["One node alone on the bus", "TX fail: ESP_ERR_TIMEOUT, TEC climbs to 128, then stops", "Same frame repeated back to back, ACK slot stays recessive, error flags", "Nobody ACKs; retransmit until error passive"],
      ["One node at 250 kbit/s", "That node: RX nothing, REC climbs. The other: bus errors", "Frames twice as long from it; the other's frames unreadable to it", "Bit timing mismatch"],
      ["RXD wire pulled", "Node sends, hears nothing, no ACK seen -> errors", "Analyzer on the bus side still shows frames", "Fault is local, not on the bus"],
    ],
    T.MARGIN, T.BODY_TOP, T.CONTENT_W, [1.8, 2.5, 2.7, 2.0], 10);
  T.addBullets(s, [
    "Not injected tonight: missing termination. Our 120 ohm are soldered in, and on 30 cm of wire at 500 kbit/s it would not show anyway",
  ], T.MARGIN, 3.3, T.CONTENT_W, 0.8, 12);
  T.addCallout(pres, s, "warn", "Rule of the lab",
    "Write the hypothesis down before you capture. A capture without a hypothesis is a screenshot.",
    T.MARGIN, 4.3, T.CONTENT_W, 0.7);

  // 22 --------------------------------------------------------------- procedure
  s = T.contentSlide(pres, "Lab 2 procedure", n++);
  T.addBullets(s, [
    "1. Known-good capture of the running bus, saved as good.sr",
    "2. The instructor breaks one thing. You see only the serial output of your node",
    "3. Symptom in one sentence, hypothesis in one sentence, in the shared doc",
    "4. Capture at the analyzer station, compare with good.sr, point at the evidence",
    "5. Conclusion. Then fix it and confirm the counters go back to 0 (or recover the bus)",
    "Three rounds, roles rotate between the pairs: node 1, node 2, analyzer",
  ], T.MARGIN, T.BODY_TOP, T.CONTENT_W, 3.0, 14);
  T.addCallout(pres, s, "info", "Recovery",
    "After bus off the node does not come back on its own. twai_initiate_recovery() in the demo, or reset the board. Know which one you did.",
    T.MARGIN, 4.2, T.CONTENT_W, 0.8);

  // 23 --------------------------------------------------------------- summary
  s = T.summarySlide(pres, n++, [
    "CAN: multi-master, differential, priority by ID, every receiver acknowledges",
    "Error counters and three states: active, passive, bus off. Faults are visible, not silent",
    "TJA1050 is 5 V: divider on RXD, common GND, 120 ohm at both ends only",
    "TWAI: install, start, transmit, receive, status, alerts - six functions do the job",
    "Diagnosis: known-good capture first, hypothesis before capture, evidence before conclusion",
  ], "No coding. Fill in homework/final-project-template.md for your final project and push it as students/<you>/final-project.md by Monday 12:00. Approval in session 22.");

  const out = await T.save(pres, "lesson21-can-twai.pptx");
  console.log("OK:", out, "slides:", n - 1);
})().catch((e) => { console.error("BUILD FAILED:", e.message); process.exit(1); });
