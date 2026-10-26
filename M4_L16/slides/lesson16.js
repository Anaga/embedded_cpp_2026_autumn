// Lesson 16 - STM32: CubeMX and HAL (Black Pill F411, USB DFU, I2C sensors)
const T = require("./slides-theme");
const { FaMicrochip } = require("react-icons/fa");

const L = T.MARGIN;
const W = T.CONTENT_W;
const TOP = T.BODY_TOP;

function arrowRow(pres, s, steps, y, h) {
  // Boxes joined by "->" labels; x computed from index
  const n = steps.length;
  const arrowW = 0.35;
  const boxW = (W - arrowW * (n - 1)) / n;
  steps.forEach((st, i) => {
    const x = L + i * (boxW + arrowW);
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
    "Lesson 16 - STM32: CubeMX and HAL",
    "Lesson 16  |  STM32: CubeMX and HAL"
  );
  let n = 1;
  const chip = await T.icon(FaMicrochip, "#CADCFC");

  // 1 ---------------------------------------------------------------- title
  let s = T.titleSlide(pres, {
    tag: "LESSON 16  |  OPTIONAL BLOCK S1",
    title: "STM32: CubeMX and HAL",
    subtitle: "Black Pill F411, USB DFU upload, two I2C sensors",
    course: "Embedded Software Development in C/C++",
    meta: "Session 16  |  4 academic hours",
    iconData: chip,
  });
  s.addNotes(
    "Timing (4 x 45 min): install check 20 min, board and layers 15, CubeMX 40, " +
    "build and DFU 30, HAL GPIO and USB log 25, I2C sensors 40, lab the rest.\n" +
    "Three Black Pills: work in pairs, instructor board for the demo."
  );

  // 2 ---------------------------------------------------------------- goals
  s = T.contentSlide(pres, "Today", ++n);
  T.addBullets(s, [
    "A new board with no Arduino layer: STM32F411 Black Pill",
    "CubeMX: pins, clock tree, peripherals -> generated C project",
    "Build in VS Code, upload over USB with the ROM bootloader (DFU)",
    "HAL in polling mode: GPIO, USB virtual COM port, I2C",
    "Two new sensors: MPU-6050 (register based) and BH1750 (command based)",
    "Check the I2C traffic with PulseView, as in session 15",
  ], L, TOP, W, 3.2);
  T.addCallout(pres, s, "info", "Before we start",
    "VS Code STM32 extension, CubeMX with the F4 package, CubeProgrammer. Missing something? Start the download now.",
    L, 4.2, W, 0.8);
  s.addNotes("Install check first. Anyone without the F4 package in CubeMX: start it now, it is the longest download.");

  // 3 ---------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 1", title: "A New Board", note: "What changes when Arduino is gone" });
  n++;

  // 4 ---------------------------------------------------------------- compare
  s = T.contentSlide(pres, "Pico vs Black Pill", ++n);
  T.addTable(s,
    ["", "Raspberry Pi Pico", "Black Pill"],
    [
      ["MCU", "RP2040", "STM32F411CEU6"],
      ["Core", "2 x Cortex-M0+", "1 x Cortex-M4F (FPU, DSP)"],
      ["Clock", "133 MHz", "up to 100 MHz (96 today)"],
      ["Flash / RAM", "2 MB external / 264 KB", "512 KB internal / 128 KB"],
      ["Upload", "UF2 drive (BOOTSEL)", "USB DFU (BOOT0) or SWD probe"],
      ["Framework", "Arduino-Pico", "CubeMX + HAL (plain C)"],
      ["Crystal", "12 MHz", "25 MHz (HSE)"],
    ],
    L, TOP + 0.1, W, [2.0, 3.4, 3.6], 13);
  s.addNotes("Both are Cortex-M, so registers like NVIC and SysTick are the same family. The F411 has an FPU: float math is cheap here, unlike on the M0+.");

  // 5 ---------------------------------------------------------------- board pins
  s = T.contentSlide(pres, "Black Pill: What We Use Today", ++n);
  T.addTable(s,
    ["Function", "Pin", "Notes"],
    [
      ["LED (blue)", "PC13", "Active low: RESET = on. Weak pin, max 3 mA"],
      ["KEY button", "PA0", "To GND, needs the internal pull-up"],
      ["BOOT0 / NRST", "buttons", "Enter the ROM bootloader (DFU)"],
      ["USB-C", "PA11 / PA12", "USB OTG FS: DFU and our COM port"],
      ["I2C1 SCL / SDA", "PB6 / PB7", "Both sensors on one bus"],
      ["Sensor power", "3V3, GND", "Never 5V on the I2C lines"],
    ],
    L, TOP + 0.1, W, [2.2, 1.6, 5.2], 13);
  T.addCallout(pres, s, "warn", "No ST-Link on this board",
    "Upload works over USB. Breakpoints need an external SWD probe - later in the block.",
    L, 4.05, W, 0.85);

  // 6 ---------------------------------------------------------------- layers
  s = T.contentSlide(pres, "Same LED, Three Layers", ++n);
  T.addCode(s, [
    "// Arduino (what we used so far)",
    "digitalWrite(LED_PIN, LOW);",
    "",
    "// HAL (today): port + pin + state",
    "HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);",
    "",
    "// Register: one write to the bit set / reset register",
    "GPIOC->BSRR = GPIO_BSRR_BR13;   // BR = reset bit 13",
  ].join("\n"), L, TOP, W, 2.85, 12);
  T.addBullets(s, [
    "HAL = ST's C library on top of the registers, one per chip family",
    "No setup(), no pinMode(): CubeMX writes the init code for us",
  ], L, 4.1, W, 0.95, 14);
  s.addNotes("Ask: which line is fastest? Which is portable to an ESP32? None of them is portable across vendors - that is the Z1 topic.");

  // 7 ---------------------------------------------------------------- toolchain
  s = T.contentSlide(pres, "The Tool Chain", ++n);
  arrowRow(pres, s, [
    { title: "CubeMX", body: "Pins, clocks, peripherals. Generates a CMake project" },
    { title: "VS Code", body: "STM32 extension: compiler, CMake, Ninja. Builds the .elf" },
    { title: "CubeProgrammer", body: "Writes the .elf to flash over USB DFU" },
    { title: "Board", body: "Press NRST, the program starts", accent: T.GREEN },
  ], TOP + 0.2, 1.7);
  T.addBullets(s, [
    "PlatformIO did all four steps with one button. Now each step is visible",
    "Since CubeIDE 2.0, CubeMX is a separate program even for CubeIDE users",
  ], L, 3.35, W, 1.4, 14);

  // 8 ---------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 2", title: "CubeMX", note: "From an empty chip to a generated project" });
  n++;

  // 9 ---------------------------------------------------------------- cubemx pinout
  s = T.contentSlide(pres, "CubeMX: Pinout and Peripherals", ++n);
  T.addTable(s,
    ["Where", "Setting"],
    [
      ["New project", "MCU selector: STM32F411CEUx"],
      ["System Core -> RCC", "HSE: Crystal/Ceramic Resonator"],
      ["System Core -> SYS", "Debug: Serial Wire (keeps SWD usable)"],
      ["PC13", "GPIO_Output, label LED, output level High"],
      ["PA0", "GPIO_Input, label KEY, Pull-up"],
      ["Connectivity -> I2C1", "I2C, 100 kHz -> lands on PB6 / PB7"],
      ["Connectivity -> USB_OTG_FS", "Mode: Device_Only"],
      ["Middleware -> USB_DEVICE", "Class: Communication Device Class (CDC)"],
    ],
    L, TOP, W, [3.3, 5.7], 13);
  s.addNotes(
    "Labels matter: LED and KEY become LED_Pin / LED_GPIO_Port in main.h.\n" +
    "Check that I2C1 really landed on PB6/PB7 (it may pick PB8/PB9 - click the pin and choose).\n" +
    "Output level High = LED off at reset, because the LED is active low."
  );

  // 10 --------------------------------------------------------------- clock tree
  s = T.contentSlide(pres, "Clock Tree: Why 96 MHz, Not 100", ++n);
  T.addTable(s,
    ["Step", "Value", "Result"],
    [
      ["HSE crystal", "25 MHz", "25 MHz"],
      ["/ M", "25", "1 MHz into the PLL"],
      ["x N", "192", "192 MHz (VCO)"],
      ["/ P -> SYSCLK", "2", "96 MHz"],
      ["/ Q -> USB", "4", "48 MHz, exactly"],
      ["APB1 / APB2", "/2, /1", "48 MHz (max 50), 96 MHz"],
    ],
    L, TOP, 5.6, [1.9, 1.3, 2.4], 13);
  T.addBullets(s, [
    "USB needs exactly 48 MHz",
    "100 MHz SYSCLK leaves no Q that gives 48",
    "HSI (16 MHz RC) is not accurate enough for USB: use the crystal",
    "Try it: type 100 into HCLK and watch the USB box turn red",
  ], 6.35, TOP, 3.2, 3.6, 13);
  s.addNotes("In CubeMX: Clock Configuration tab, PLL Source Mux = HSE, type 96 into HCLK and press Enter - the solver finds M/N/P/Q. Then show 100 failing.");

  // 11 --------------------------------------------------------------- project manager + user code
  s = T.contentSlide(pres, "Generate Code - and the USER CODE Rule", ++n);
  T.addBullets(s, [
    "Project Manager: name lesson16, Toolchain / IDE = CMake",
    "Generate Code writes main.c, startup, HAL, USB middleware, CMakeLists",
    "Regenerate after every change in CubeMX - it rewrites the files",
  ], L, TOP, W, 1.3, 14);
  T.addCode(s, [
    "/* USER CODE BEGIN 2 */",
    "app_init();          // survives Generate Code",
    "/* USER CODE END 2 */",
    "",
    "HAL_Delay(10);       // outside the markers: DELETED next time",
  ].join("\n"), L, 2.45, W, 1.88, 12);
  T.addCallout(pres, s, "error", "Only code between BEGIN and END survives",
    "Our approach: all our code in app.c, main.c gets two calls only.",
    L, 4.4, W, 0.65);

  // 12 --------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 3", title: "Build and Upload", note: "VS Code, then the ROM bootloader" });
  n++;

  // 13 --------------------------------------------------------------- build
  s = T.contentSlide(pres, "Build in VS Code", ++n);
  T.addBullets(s, [
    "STM32 panel -> import the generated folder as a CMake project",
    "Add our file to the root CMakeLists.txt, then Build",
    "Result: build/Debug/lesson16.elf",
  ], L, TOP, W, 1.4, 14);
  T.addCode(s, [
    "# CMakeLists.txt (root) - generated, this block is ours",
    "target_sources(${CMAKE_PROJECT_NAME} PRIVATE",
    "    # Add user sources here",
    "    Core/Src/app.c",
    ")",
  ].join("\n"), L, 2.5, W, 1.86, 12);
  T.addCallout(pres, s, "info", "Linker summary",
    "The build prints FLASH and RAM usage. Compare with the 512 KB / 128 KB from the table.",
    L, 4.35, W, 0.7);
  s.addNotes("Button names differ slightly between extension versions. The CMake: Build command (F7) always works.");

  // 14 --------------------------------------------------------------- DFU
  s = T.contentSlide(pres, "Upload over USB: DFU", ++n);
  T.addCards(pres, s, [
    { title: "1. Enter DFU", body: "Hold BOOT0, press and release NRST, release BOOT0. LED stays dark.", accent: T.AMBER },
    { title: "2. Connect", body: "CubeProgrammer: USB, refresh, port USB1, Connect" },
    { title: "3. Program", body: "Erasing and Programming: choose the .elf, Start Programming" },
    { title: "4. Run", body: "Disconnect, press NRST. Our code starts", accent: T.GREEN },
  ], TOP + 0.05, 2.15);
  T.addBullets(s, [
    "The DFU bootloader lives in ROM (system memory): it cannot be erased by mistake",
    "Not found? Data cable, repeat step 1, on Windows reinstall CubeProgrammer drivers",
  ], L, 3.45, W, 1.4, 13);
  s.addNotes("BOOT0 sampled at reset selects system memory instead of flash. Holding BOOT0 while plugging in works too, but the NRST sequence is more reliable on these boards.");

  // 15 --------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 4", title: "HAL in Our Code", note: "GPIO and the USB virtual COM port" });
  n++;

  // 16 --------------------------------------------------------------- GPIO
  s = T.contentSlide(pres, "GPIO and Time with HAL", ++n);
  T.addCode(s, [
    "static const uint32_t BLINK_MS = 500U;",
    "static uint32_t last_ms = 0U;",
    "void app_loop(void)",
    "{",
    "    const uint32_t now = HAL_GetTick();   // like millis()",
    "    if ((now - last_ms) >= BLINK_MS) {",
    "        last_ms += BLINK_MS;",
    "        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);",
    "    }",
    "    GPIO_PinState key = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);",
    "    if (key == GPIO_PIN_RESET) { /* pressed: active low */ }",
    "}",
  ].join("\n"), L, TOP, 6.2, 3.6, 10.5);
  T.addBullets(s, [
    "HAL_GetTick(): ms since reset, SysTick, same as millis()",
    "LED_Pin, LED_GPIO_Port: from our CubeMX label",
    "Same non-blocking pattern as in K3",
    "No pinMode(): MX_GPIO_Init() did that",
  ], 6.85, TOP, 2.7, 3.6, 13);

  // 17 --------------------------------------------------------------- USB CDC log
  s = T.contentSlide(pres, "A printf over USB", ++n);
  T.addCode(s, [
    "static void log_printf(const char *fmt, ...)",
    "{",
    "    static char buf[LOG_BUF_SIZE];  // USB reads it later",
    "    if (!cdc_wait_idle(CDC_TIMEOUT_MS)) { return; } // nobody reads",
    "    va_list args;",
    "    va_start(args, fmt);",
    "    int len = vsnprintf(buf, sizeof buf, fmt, args);",
    "    va_end(args);",
    "    if (len > 0) {",
    "        CDC_Transmit_FS((uint8_t *)buf, (uint16_t)len);",
    "    }",
    "}",
  ].join("\n"), L, TOP, 6.2, 3.6, 10.5);
  T.addBullets(s, [
    "CDC_Transmit_FS() only starts a transfer",
    "Wait for the previous one before writing buf again",
    "No %f: newlib-nano printf has no floats. Use tenths",
    "Baud rate in the terminal is ignored",
  ], 6.85, TOP, 2.7, 3.6, 13);
  s.addNotes(
    "Simplified for the slide - full version with length clamping in demo/Core/Src/app.c.\n" +
    "cdc_wait_idle() checks hUsbDeviceFS.dev_state and the TxState of the CDC handle.\n" +
    "Before the host has configured the device the handle is NULL - printing in app_init() would be lost, that is why the demo waits.\n" +
    "Terminal: pio device monitor, or any serial monitor; the port is /dev/cu.usbmodem... on macOS, COMx on Windows."
  );

  // 18 --------------------------------------------------------------- section
  T.sectionSlide(pres, { kicker: "PART 5", title: "I2C with HAL", note: "GY-521 (MPU-6050) and BH1750" });
  n++;

  // 19 --------------------------------------------------------------- wiring + address
  s = T.contentSlide(pres, "Wiring and the Address Trap", ++n);
  T.addTable(s,
    ["Module pin", "GY-521", "BH1750", "Black Pill"],
    [
      ["VCC", "VCC", "VCC", "3V3"],
      ["GND", "GND", "GND", "GND"],
      ["SCL", "SCL", "SCL", "PB6"],
      ["SDA", "SDA", "SDA", "PB7"],
      ["Address pin", "AD0 -> GND: 0x68", "ADDR -> GND: 0x23", "-"],
    ],
    L, TOP, W, [1.9, 2.5, 2.5, 2.1], 13);
  T.addCode(s, [
    "// HAL wants the 7-bit address shifted left by one",
    "static const uint16_t MPU_ADDR = (uint16_t)(0x68U << 1);",
  ].join("\n"), L, 3.35, W, 0.95, 12);
  T.addCallout(pres, s, "warn", "Forget the shift and every call returns HAL_ERROR",
    "Arduino Wire shifts for you. HAL does not. Scan with HAL_I2C_IsDeviceReady(&hi2c1, a << 1, ...)",
    L, 4.38, W, 0.67);

  // 20 --------------------------------------------------------------- MPU-6050
  s = T.contentSlide(pres, "MPU-6050: Register Based", ++n);
  T.addCode(s, [
    "uint8_t who = 0U;",
    "HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, 0x75U,   // WHO_AM_I",
    "                 I2C_MEMADD_SIZE_8BIT, &who, 1U, 10U);",
    "uint8_t wake = 0x00U;                       // clear SLEEP",
    "HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, 0x6BU,  // PWR_MGMT_1",
    "                  I2C_MEMADD_SIZE_8BIT, &wake, 1U, 10U);",
    "uint8_t b[14];  // ax ay az temp gx gy gz, big endian",
    "HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, 0x3BU,   // ACCEL_XOUT_H",
    "                 I2C_MEMADD_SIZE_8BIT, b, 14U, 10U);",
  ].join("\n"), L, TOP, W, 2.8, 10.5);
  T.addCallout(pres, s, "info", "Clones",
    "Many GY-521 boards carry a clone chip: WHO_AM_I 0x70, 0x72 or 0x98 instead of 0x68. Print the value, do not hard-fail.",
    L, 4.15, W, 0.8);
  s.addNotes("Numbers are inline for the slide; the demo uses named constants. Scaling at +-2 g: 16384 LSB per g, so mg = raw * 1000 / 16384. Temperature: raw / 340 + 36.53 C. One Mem_Read = write register, repeated START, read.");

  // 21 --------------------------------------------------------------- BH1750
  s = T.contentSlide(pres, "BH1750: Command Based", ++n);
  T.addCode(s, [
    "uint8_t cmd = 0x01U;                       // power on",
    "HAL_I2C_Master_Transmit(&hi2c1, BH_ADDR, &cmd, 1U, 10U);",
    "cmd = 0x10U;                               // continuous, 1 lx",
    "HAL_I2C_Master_Transmit(&hi2c1, BH_ADDR, &cmd, 1U, 10U);",
    "uint8_t b[2];                             // every >= 180 ms",
    "HAL_I2C_Master_Receive(&hi2c1, BH_ADDR, b, 2U, 10U);",
    "uint32_t raw = ((uint32_t)b[0] << 8) | b[1];",
    "uint32_t lux10 = raw * 25U / 3U;   // lux = raw / 1.2",
  ].join("\n"), L, TOP, W, 2.65, 11);
  T.addTable(s,
    ["", "Register based (MPU-6050)", "Command based (BH1750, AHT10)"],
    [
      ["Write", "register address, then data", "a command byte"],
      ["Read", "HAL_I2C_Mem_Read()", "HAL_I2C_Master_Receive()"],
    ],
    L, 3.95, W, [1.2, 3.9, 3.9], 12);

  // 22 --------------------------------------------------------------- PulseView
  s = T.contentSlide(pres, "Check It on the Wire", ++n);
  T.addBullets(s, [
    "D1 SDA = PB7, D2 SCL = PB6, GND to GND. I2C decoder, as in session 15",
    "Sample at 1-2 MHz: 100 kHz bus, 10-20 samples per bit",
    "Find the MPU burst: Write 0x68, 0x3B, repeated START, Read 0x68, 14 bytes",
    "Find the BH1750 read: Read 0x23, 2 bytes, no register byte",
    "Unplug a sensor: what does the NACK look like, what does HAL return?",
  ], L, TOP, W, 2.8, 14);
  T.addCallout(pres, s, "ok", "Same bus, different chip",
    "The waveform does not care whether Arduino Wire or HAL produced it. The logic analyzer is the neutral judge.",
    L, 4.1, W, 0.8);

  // 23 --------------------------------------------------------------- lab
  s = T.contentSlide(pres, "Lab (in pairs)", ++n);
  T.addBullets(s, [
    "1. Your own CubeMX project from the settings on slides 9-10, generated as CMake",
    "2. Blink the LED with HAL_GetTick(), upload with DFU",
    "3. KEY changes the blink period",
    "4. log_printf over USB: print the SYSCLK with HAL_RCC_GetSysClockFreq()",
    "5. I2C scan: both 0x23 and 0x68 must show up",
    "6. Read WHO_AM_I, then lux. Stretch: acceleration in mg",
  ], L, TOP, W, 3.3, 14);
  T.addCallout(pres, s, "info", "Stuck?",
    "Check the order: clock tree -> USB enumerates -> terminal open -> I2C. Do not debug two at once.",
    L, 4.25, W, 0.75);
  s.addNotes("Pairs: three Black Pills. The full demo (app.c) is shown at the end, not handed out before the lab.");

  // 24 --------------------------------------------------------------- summary
  s = T.contentSlide(pres, "Summary", ++n);
  T.addBullets(s, [
    "CubeMX generates init code; our code lives in USER CODE sections and app.c",
    "The clock tree is a design decision: USB forced 96 MHz",
    "DFU: the ROM bootloader, entered with BOOT0 + NRST",
    "HAL calls are explicit: port and pin, handle, timeout, return status",
    "I2C: address << 1, Mem_Read for registers, Master_Receive for commands",
  ], L, TOP, W, 2.9, 15);
  T.addCallout(pres, s, "warn", "Keep your project",
    "The next STM32 session builds on the same CubeMX project and board.",
    L, 4.2, W, 0.75);

  await T.save(pres, "lesson16.pptx");
  console.log("OK slides: " + n);
})().catch((e) => { console.error(e.message); process.exit(1); });
