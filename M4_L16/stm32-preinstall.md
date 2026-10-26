# Session 16 - install before class

Tonight we move to STM32 (STM32F411CE "Black Pill" board). The tools are
large downloads, and one of them needs a free ST account. Please install
everything below **before the session**, so we spend class time on the
board and not on progress bars.

Budget 30-60 minutes, most of it waiting for downloads.

## 1. STM32CubeIDE for Visual Studio Code

1. Open VS Code -> Extensions.
2. Search for **STM32CubeIDE for Visual Studio Code** (publisher
   STMicroelectronics, butterfly logo).
   - There are about fifteen "STM32Cube ..." extensions. Install the one with
     exactly this name: it is the extension pack and pulls in the rest.
3. Let it finish. It downloads the compiler and command-line tools by itself
   (no separate STM32CubeCLT install is needed).

## 2. STM32CubeMX

CubeMX is the graphical configurator: pins, clocks, peripherals. It generates
the start-up code we build on.

1. Download STM32CubeMX from st.com (search "STM32CubeMX").
2. The download asks for a free my.st.com account. Create one - you will need
   it again.
3. Install and start CubeMX once.
4. **Important, saves the most time in class:** Help -> Manage embedded
   software packages -> STM32F4 -> tick the newest version -> Install.
   This is the F4 firmware package (HAL drivers), several hundred MB.

## 3. STM32CubeProgrammer

We upload over USB using the chip's built-in DFU bootloader. No debug probe
is needed.

1. Download STM32CubeProgrammer from st.com (same account) and install it.
2. **Windows:** accept the driver installation during setup. It installs the
   "STM32 BOOTLOADER" (DFU) driver. Without it the board is not seen in DFU
   mode.
3. macOS / Linux: no extra driver.

## 4. Bring

- A **USB-C data cable** (the Black Pill has a USB-C socket). Many cheap
  cables are charge-only and the board will never appear. Mac users with
  USB-C ports: a C-to-C cable works.
- Your logic analyzer from session 15 (PulseView already installed).

## 5. Check

- VS Code shows the STM32 icon in the left activity bar.
- CubeMX: Help -> Manage embedded software packages shows STM32F4 as
  installed (green box).
- STM32CubeProgrammer starts.

If something fails, write the error message down (or take a screenshot) and
bring it. Do not spend more than 20 minutes fighting one problem - we fix
the rest in class.
