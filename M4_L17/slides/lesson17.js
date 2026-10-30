// Lesson 17 - STM32: from a ready project to a working demo (Black Pill F411)
const T = require("./slides-theme");
const { FaMicrochip } = require("react-icons/fa");

const L = T.MARGIN;
const W = T.CONTENT_W;
const TOP = T.BODY_TOP;

function arrowRow(pres, s, steps, y, h, x0, w0) {
  // Boxes joined by "->" labels; x computed from index
  const n = steps.length;
  const arrowW = 0.35;
  const left = x0 === undefined ? L : x0;
  const width = w0 === undefined ? W : w0;
  const boxW = (width - arrowW * (n - 1)) / n;
  steps.forEach((st, i) => {
    const x = left + i * (boxW + arrowW);
    s.addShape(pres.shapes.RECTANGLE, {
      x: x, y: y, w: boxW, h: h,
      fill: { color: T.CARD_BG }, line: { color: st.accent || T.MID, width: st.accent ? 2 : 1 },
    });
    s.addText(T.assertAscii(st.title), {
      x: x, y: y + 0.12, w: boxW, h: 0.35,
      fontSize: 14, fontFace: T.HEADING, color: T.WHITE, bold: true, align: "center", margin: 0,
    });
    s.addText(T.assertAscii(st.body), {
      x: x + 0.08, y: y + 0.5, w: boxW - 0.16, h: h - 0.6,
      fontSize: 11, fontFace: T.BODY, color: T.ICE, align: "center", valign: "top", margin: 0,
    });
    if (i < n - 1) {
      s.addText("->", {
        x: x + boxW, y: y + h / 2 - 0.2, w: arrowW, h: 0.4,
        fontSize: 16, fontFace: T.CODE, color: T.AMBER, bold: true, align: "center", margin: 0,
      });
    }
  });
}

(async () => {
  const pres = T.newDeck(
    "Lesson 17 - STM32: From Project to Working Demo",
    "Lesson 17  |  STM32: From Project to Working Demo"
  );
  let n = 1;
  const chip = await T.icon(FaMicrochip, "#CADCFC");

  // 1 ---------------------------------------------------------------- title
  let s = T.titleSlide(pres, {
    tag: "LESSON 17  |  OPTIONAL BLOCK S1",
    title: "STM32: Working Demo",
    subtitle: "Ready CubeMX project, DFU upload, two I2C sensors",
    course: "Embedded Software Development in C/C++",
    meta: "Session 17  |  4 academic hours",
    iconData: chip,
  });
  s.addNotes(
    "Plan (4 x 45 min): pull and build 20 min, DFU upload 20, bare-board output 10, " +
    "wiring and sensors 30, app.c tour 30, homework start in class the rest.\n" +
    "Goal: every pair sees real sensor values within the first hour."
  );

  // 2 ---------------------------------------------------------------- today
  s = T.contentSlide(pres, "Today", ++n);
  T.addBullets(s, [
    "Last time CubeMX took most of the session - today the project is ready",
    "Pull -> build -> upload over USB (DFU) -> see the output",
    "First on a bare board, then with the GY-521 and the BH1750",
    "A tour of app.c: how our code sits next to the generated code",
    "Start the homework in class: night light and orientation",
  ], L, TOP, W, 2.9);
  T.addCallout(pres, s, "info", "The project",
    "M4_L17/lesson16 - the full CubeMX project from session 16, tested on a real Black Pill.",
    L, 4.2, W, 0.8);

  // 3 ---------------------------------------------------------------- recap
  s = T.contentSlide(pres, "Recap: Session 16 in Four Ideas", ++n);
  T.addCards(pres, s, [
    { title: "CubeMX", body: "Pins, clocks and peripherals are clicked once. CubeMX writes the init code in C." },
    { title: "96 MHz", body: "USB needs exactly 48 MHz. 96 / 2 = 48 works, 100 MHz does not.", accent: T.AMBER },
    { title: "HAL", body: "Explicit calls: port and pin, handle, timeout, a return status to check." },
    { title: "app.c", body: "Our code. main.c only calls app_init() and app_loop() in USER CODE.", accent: T.GREEN },
  ], 1.35, 1.95);
  T.addCallout(pres, s, "ok", "All four are already done in today's project",
    "Nothing to click in CubeMX today. We use the result and look inside it.",
    L, 3.7, W, 0.85);

  // 4 ---------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 1", title: "Build and Upload", note: "From git pull to a blinking LED" });
  n++;

  // 5 ---------------------------------------------------------------- build
  s = T.contentSlide(pres, "Step 1: Open and Build", ++n);
  arrowRow(pres, s, [
    { title: "git pull", body: "Course repository" },
    { title: "Open Folder", body: "M4_L17/lesson16 - the folder with the .ioc" },
    { title: "Set Up", body: "STM32Cube panel -> Set Up project -> Debug", accent: T.AMBER },
    { title: "Build", body: "F7 -> build/Debug/ lesson16.elf", accent: T.GREEN },
  ], 1.3, 1.5);
  T.addCode(s, [
    "RAM:   12720 B   128 KB   9.70%",
    "FLASH: ~41 KB    512 KB   ~8%",
  ].join("\n"), L, 2.95, W, 0.95, 12, "Expected at the end of the build");
  T.addCallout(pres, s, "warn", "Open the project folder, not the repository root",
    "Otherwise the STM32 extension finds no CMakeLists.txt and offers nothing to build.",
    L, 4.32, W, 0.72);

  // 6 ---------------------------------------------------------------- DFU
  s = T.contentSlide(pres, "Step 2: Upload over USB (DFU)", ++n);
  arrowRow(pres, s, [
    { title: "Hold BOOT0", body: "and keep holding" },
    { title: "Tap NRST", body: "press and release" },
    { title: "Release BOOT0", body: "LED stays dark: ROM bootloader runs", accent: T.AMBER },
  ], 1.3, 1.15);
  T.addTable(s,
    ["STM32CubeProgrammer", "Setting"],
    [
      ["Connection", "USB, refresh, port USB1 -> Connect"],
      ["File path", "build/Debug/lesson16.elf"],
      ["Start address", "empty - the .elf knows it"],
      ["Verify / Run after programming", "both ticked"],
    ],
    L, 2.7, W, [3.6, 5.4], 12);
  s.addText(T.assertAscii("No USB1? Data cable (not charge-only) and repeat BOOT0 + NRST."), {
    x: L, y: 4.6, w: W, h: 0.35,
    fontSize: 12, fontFace: T.BODY, color: T.GRAY, italic: true, margin: 0,
  });

  // 7 ---------------------------------------------------------------- red is normal
  s = T.contentSlide(pres, "Red at the End Is Normal", ++n);
  T.addCode(s, [
    "Download verified successfully",
    "RUNNING Program ...",
    "Start operation achieved successfully",
    "Warning: Connection to device 0x431 is lost",
    "Error: Establishing connection with device failed",
  ].join("\n"), L, TOP, 5.6, 1.9, 12, "Last lines of the CubeProgrammer log");
  T.addBullets(s, [
    "Look for: verified successfully",
    "Then the chip leaves the bootloader",
    "The DFU device disappears from USB",
    "CubeProgrammer cannot reconnect: red",
  ], 6.3, TOP, 3.2, 2.2, 13);
  T.addCallout(pres, s, "ok", "Your program is running",
    "The red line means the bootloader is gone - exactly what we want. A new COM port appears instead.",
    L, 3.6, W, 0.85);

  // 8 ---------------------------------------------------------------- bare board
  s = T.contentSlide(pres, "Step 3: Bare Board First", ++n);
  T.addCode(s, [
    "Lesson 16 - STM32F411 Black Pill, HAL, USB CDC",
    "SYSCLK 96000000 Hz",
    "I2C1 scan (PB6 SCL, PB7 SDA):",
    "  0 device(s)",
    "MPU-6050 not found at 0x68",
    "BH1750 not found at 0x23",
    "Press KEY to show this again.",
    "",
    "    5265 ms  lux n/a  acc n/a",
  ].join("\n"), L, TOP, 5.6, 2.65, 10, "VS Code Serial Monitor, any baud rate");
  T.addBullets(s, [
    "Clock: correct",
    "USB virtual COM port: works",
    "I2C: nothing on the bus yet",
    "Port opened late? Press KEY",
  ], 6.3, TOP, 3.2, 2.0, 13);
  T.addCallout(pres, s, "info", "One thing at a time",
    "Clock and USB are proven before a single wire is added. If the sensors fail later, the problem is the wiring.",
    L, 4.15, W, 0.85);

  // 9 ---------------------------------------------------------------- wiring
  s = T.contentSlide(pres, "Step 4: Wire the Sensors", ++n);
  T.addTable(s,
    ["Module pin", "GY-521 (MPU-6050)", "BH1750", "Black Pill"],
    [
      ["VCC", "VCC", "VCC", "3V3"],
      ["GND", "GND", "GND", "GND"],
      ["SCL", "SCL", "SCL", "PB6"],
      ["SDA", "SDA", "SDA", "PB7"],
      ["Address", "0x68 (AD0 low)", "0x23 (ADDR low)", "-"],
    ],
    L, TOP + 0.1, W, [1.8, 2.6, 2.4, 2.2], 13);
  T.addCallout(pres, s, "warn", "3V3 only, never 5V",
    "Both modules carry their own pull-up resistors. Two sensors share the same two wires: that is the point of a bus.",
    L, 3.65, W, 0.85);

  // 10 --------------------------------------------------------------- with sensors
  s = T.contentSlide(pres, "With Sensors: What You Should See", ++n);
  T.addCode(s, [
    "I2C1 scan (PB6 SCL, PB7 SDA):",
    "  0x23",
    "  0x68",
    "  2 device(s)",
    "MPU-6050 WHO_AM_I = 0x68 (0x68 on a genuine part)",
    "BH1750 ok",
    "",
    "    4512 ms  lux 213.3  acc 12 -40 1003 mg  temp 26.4 C",
  ].join("\n"), L, TOP, W, 2.4, 10, "Press KEY after wiring: the scan runs again");
  T.addTable(s,
    ["Scan still empty?", "Check"],
    [
      ["Module LED off", "VCC on 3V3, GND on GND"],
      ["LED on, 0 devices", "SCL and SDA swapped? SCL = PB6, SDA = PB7"],
    ],
    L, 3.95, W, [3.0, 6.0], 12);

  // 11 --------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 2", title: "Inside app.c", note: "Where our code lives and how it runs" });
  n++;

  // 12 --------------------------------------------------------------- app.c structure
  s = T.contentSlide(pres, "app.c: A Small State Machine", ++n);
  T.addCode(s, [
    "/* main.c, USER CODE 2 */",
    "app_init();",
    "",
    "/* main.c, USER CODE 3 */",
    "while (1) {",
    "    app_loop();",
    "}",
  ].join("\n"), L, TOP, 3.4, 2.35, 11, "The only lines we add to main.c");
  arrowRow(pres, s, [
    { title: "WAIT_USB", body: "LED blinks fast" },
    { title: "GRACE", body: "2 s to open the port" },
    { title: "RUN", body: "every 500 ms: read, print", accent: T.GREEN },
  ], TOP, 1.3, 4.1, 5.4);
  T.addBullets(s, [
    "app_loop() never blocks: it checks HAL_GetTick() and returns",
    "Cable pulled -> back to WAIT_USB",
    "Generate Code in CubeMX leaves app.c untouched",
  ], L, 3.9, W, 1.2, 13);

  // 13 --------------------------------------------------------------- two sensor styles
  s = T.contentSlide(pres, "Two Sensors, Two Styles", ++n);
  T.addCode(s, [
    "/* MPU-6050: registers */",
    "HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR,",
    "    0x3B, I2C_MEMADD_SIZE_8BIT,",
    "    b, 14, I2C_TIMEOUT_MS);",
    "/* write 0x3B, repeated START,",
    "   read 14 bytes: accel, temp, gyro */",
  ].join("\n"), L, TOP, 4.35, 2.05, 11);
  T.addCode(s, [
    "/* BH1750: commands, no registers */",
    "bh_send(0x01); /* power on      */",
    "bh_send(0x10); /* continuous H  */",
    "HAL_I2C_Master_Receive(&hi2c1,",
    "    BH_ADDR, b, 2, I2C_TIMEOUT_MS);",
    "/* lux = raw / 1.2 */",
  ].join("\n"), L + 4.65, TOP, 4.35, 2.05, 11);
  T.addCallout(pres, s, "info", "Same bus, different conversation",
    "HAL wants the address shifted left by one: 0x68 << 1. No %f in printf (newlib-nano), so values are printed in tenths.",
    L, 3.45, W, 0.95);

  // 14 --------------------------------------------------------------- homework
  s = T.contentSlide(pres, "Homework 17", ++n);
  T.addCards(pres, s, [
    { title: "Night light", body: "Below 10 lx: LED on. 10..300 lx: toggles. Above 300 lx: off. Limits are named constants.", accent: T.AMBER },
    { title: "Orientation", body: "az >= 700 mg: FLAT. az <= -700 mg: UPSIDE DOWN. Otherwise ON EDGE. Printed at the end of each line.", accent: T.AMBER },
    { title: "Extra", body: "Print \"-> now UPSIDE DOWN\" once per change. Hint: a static variable.", accent: T.GREEN },
  ], TOP + 0.1, 2.3);
  T.addCallout(pres, s, "warn", "Work in your own copy",
    "Copy M4_L17/lesson16 to homework/<name>/lesson16. Change only Core/Src/app.c. Details: homework/README.md.",
    L, 3.85, W, 0.85);

  // 15 --------------------------------------------------------------- summary
  s = T.contentSlide(pres, "Summary", ++n);
  T.addBullets(s, [
    "A generated project is a normal folder: open, set up, build",
    "DFU: BOOT0 + NRST, .elf without a start address, red at the end is fine",
    "Bring up in order: clock -> USB -> bus -> sensors",
    "MPU-6050 is register based, BH1750 is command based",
    "Our code stays in app.c, so CubeMX can regenerate at any time",
  ], L, TOP, W, 2.9, 15);
  T.addCallout(pres, s, "warn", "Homework",
    "Night light and orientation in app.c - see M4_L17/homework/README.md.",
    L, 4.2, W, 0.75);

  await T.save(pres, "lesson17.pptx");
  console.log("OK slides: " + n);
})().catch((e) => { console.error(e.message); process.exit(1); });
