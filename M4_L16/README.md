# Lesson 16 demo - STM32F411 Black Pill, CubeMX and HAL

What it does:

- Blue LED (PC13) blinks fast until the PC has set up the USB connection,
  then toggles every 500 ms
- Prints over USB (virtual COM port): SYSCLK, an I2C scan, the MPU-6050
  WHO_AM_I value, then every 500 ms light (lux), acceleration (mg) and the
  MPU-6050 chip temperature
- KEY (PA0) prints the banner again and re-initialises the sensors, so a
  sensor plugged in later is picked up

Example output:

```
Lesson 16 - STM32F411 Black Pill, HAL, USB CDC
SYSCLK 96000000 Hz
I2C1 scan (PB6 SCL, PB7 SDA):
  0x23
  0x68
  2 device(s)
MPU-6050 WHO_AM_I = 0x68 (0x68 on a genuine part)
BH1750 ok
Press KEY to show this again.

    4512 ms  lux 213.3  acc 12 -40 1003 mg  temp 26.4 C
```

This is not a PlatformIO project. CubeMX generates the project, and only two
files are ours: `Core/Src/app.c` and `Core/Inc/app.h`.

## Wiring

| Module pin | GY-521 (MPU-6050) | BH1750 | Black Pill |
|---|---|---|---|
| VCC | VCC | VCC | 3V3 |
| GND | GND | GND | GND |
| SCL | SCL | SCL | PB6 |
| SDA | SDA | SDA | PB7 |
| Address | AD0 to GND (or open): 0x68 | ADDR to GND (or open): 0x23 | - |

Both modules have their own pull-up resistors. Power them from 3V3, never
from 5V.

## 1. CubeMX

New project -> MCU/MPU Selector -> `STM32F411CEUx` -> Start Project.

**Pinout and Configuration**

| Where | Setting |
|---|---|
| System Core -> RCC | High Speed Clock (HSE): Crystal/Ceramic Resonator |
| System Core -> SYS | Debug: Serial Wire |
| PC13 (click the pin) | GPIO_Output. In System Core -> GPIO: User Label `LED`, GPIO output level High |
| PA0 (click the pin) | GPIO_Input. In System Core -> GPIO: User Label `KEY`, Pull-up |
| Connectivity -> I2C1 | Mode: I2C. Standard mode, 100 kHz. Check the pins are PB6 (SCL) / PB7 (SDA) |
| Connectivity -> USB_OTG_FS | Mode: Device_Only |
| Middleware -> USB_DEVICE | Class For FS IP: Communication Device Class (Virtual Port Com) |

The labels must be exactly `LED` and `KEY`: `app.c` uses `LED_Pin`,
`LED_GPIO_Port`, `KEY_Pin` and `KEY_GPIO_Port`, which CubeMX writes into
`main.h` from those labels.

**Clock Configuration**

- PLL Source Mux: HSE, input frequency 25 MHz
- System Clock Mux: PLLCLK
- Type `96` into HCLK and press Enter. CubeMX finds:
  /M = 25, *N = 192, /P = 2, /Q = 4 -> SYSCLK 96 MHz, 48 MHz for USB
- APB1 prescaler /2 (48 MHz), APB2 /1 (96 MHz)

Why not 100 MHz: USB needs exactly 48 MHz, and no /Q gives 48 MHz when
SYSCLK is 100 MHz. Try it - the USB clock box turns red.

**Project Manager**

- Project Name `lesson16`, choose a location without spaces
- Toolchain / IDE: **CMake**
- Linker Settings: Minimum Stack Size `0x1000` (vsnprintf needs more than the
  default 0x400)

Generate Code.

## 2. Add our code

Copy `Core/Src/app.c` and `Core/Inc/app.h` into the same folders of the
generated project.

`Core/Src/main.c` - only inside the USER CODE markers:

```c
/* USER CODE BEGIN Includes */
#include "app.h"
/* USER CODE END Includes */
```

```c
  /* USER CODE BEGIN 2 */
  app_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    app_loop();
  }
  /* USER CODE END 3 */
```

`CMakeLists.txt` in the project root:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/Src/app.c
)
```

Anything outside the USER CODE markers is overwritten the next time you press
Generate Code.

## 3. Build

VS Code -> STM32 extension -> import the generated folder as a CMake project
-> Build. The result is `build/Debug/lesson16.elf`. The exact button names
differ between extension versions; the command `CMake: Build` (F7) always
works.

## 4. Upload over USB (DFU)

1. Enter the bootloader: hold BOOT0, press and release NRST, release BOOT0.
   The LED stays dark.
2. STM32CubeProgrammer: connection type **USB** (top right), refresh, port
   `USB1`, Connect.
3. Erasing & Programming (left bar): browse to `build/Debug/lesson16.elf`,
   Start Programming.
4. Disconnect, press NRST. The LED blinks fast, then slowly.

Command line alternative (CubeProgrammer is installed with a CLI):

```
STM32_Programmer_CLI -c port=USB1 -w build/Debug/lesson16.elf -v
```

Then press NRST.

If no DFU device shows up: use a data cable (not charge-only), repeat step 1,
and on Windows reinstall STM32CubeProgrammer and accept the driver
installation.

## 5. Watch the output

The board appears as a serial port: `/dev/cu.usbmodem...` on macOS, `COMx` on
Windows. The baud rate is ignored. For example:

```
pio device monitor
```

The banner is printed 2 s after the USB connection is up. Opened the terminal
later? Press KEY.

## Things to try

- Tilt the GY-521: which axis reads about 1000 mg when the board lies flat,
  and why?
- Cover the BH1750, then hold it under a lamp. What is the range?
- Unplug a sensor while it runs. What prints? Plug it back in and press KEY.
- Change `0x68U << 1` to `0x68U`. What does the scan still find, and why does
  everything else fail?
- Capture the 14-byte burst in PulseView: find the register byte 0x3B and the
  repeated START.
- Print a float with `%f`. What comes out, and why?
