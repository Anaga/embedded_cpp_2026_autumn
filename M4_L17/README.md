# Lesson 17 - STM32F411 Black Pill: the ready-made project

Last time most of the session went into CubeMX settings. Today you get the
finished, generated project and go straight to build, upload and sensors.

`lesson16/` is the complete CubeMX project from session 16: clocks (96 MHz),
LED, KEY, I2C1, USB CDC and our `app.c`. It is already tested on a real Black
Pill with both sensors.

## 1. Open and build

1. `git pull` in the course repository.
2. VS Code -> File -> Open Folder -> `M4_L17/lesson16` (the folder with
   `lesson16.ioc` in it, not the repository root).
3. STM32Cube panel -> **Set Up STM32Cube project(s)** -> preset **Debug**.
   The first time this downloads the compiler, so give it a few minutes.
4. Build (F7, or `CMake: Build` from the command palette).

Expected at the end of the build:

```
RAM:   12720 B   128 KB   9.70%
FLASH: about 41 KB   512 KB   about 8%
```

The result is `build/Debug/lesson16.elf`.

## 2. Wiring

| Module pin | GY-521 (MPU-6050) | BH1750 | Black Pill |
|---|---|---|---|
| VCC | VCC | VCC | 3V3 |
| GND | GND | GND | GND |
| SCL | SCL | SCL | PB6 |
| SDA | SDA | SDA | PB7 |

Power the modules from 3V3, never from 5V.

## 3. Upload over USB (DFU)

1. Bootloader: **hold BOOT0, press and release NRST, release BOOT0**.
   The LED stays dark.
2. STM32CubeProgrammer: connection type **USB**, refresh, port `USB1`,
   Connect.
3. Erasing & Programming -> browse to `build/Debug/lesson16.elf`. Leave
   Start address empty (the .elf file knows it). Tick **Verify programming**
   and **Run after programming** -> Start Programming.
4. The program starts by itself. The log then shows "Connection to device
   is lost" and "Establishing connection with device failed" in red, and
   the progress bar turns red. **This is normal**: the chip left the
   bootloader and runs your program. Look for "Download verified
   successfully" above those lines.
   Without Run after programming: Disconnect, then press NRST.

No DFU device? Use a data cable, not a charge-only one, and repeat step 1.

## 4. What you should see

- LED blinks fast while the PC sets up USB, then toggles every 500 ms.
- Open the new serial port (any baud rate) in the VS Code Serial Monitor.
  Press KEY to print the banner again if you opened the port late:

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

## 5. CubeMX - only to look

Open `lesson16.ioc` in CubeMX to see where every setting lives. Do not press
Generate Code in this folder today. When you do regenerate later, only code
between `USER CODE BEGIN` and `USER CODE END` survives - that is why our code
sits in `app.c` and `main.c` only calls `app_init()` and `app_loop()`.

## Homework

See `homework/README.md`.
