# Pico setup - Антон

- **OS:** Windows 10 (MagicBook, LAPTOP-2M2SG5QQ)
- **Preparation:**
  - Enabled Windows LongPaths support in Registry (`LongPathsEnabled` = 1) via PowerShell Admin terminal.
  - Enabled Git system-wide longpaths support (`git config --system core.longpaths true`).
  - Pre-downloaded Zadig 2.9 to `D:\cMCU\drivers\zadig-2.9.exe` for the bootloader USB driver setup.
- **First build:** 
  - Toolchain (`toolchain-rp2040-earlephilhower`) and Pico framework download and build took about 47 seconds total.
  - Compilation of the blink & uptime code succeeded cleanly with zero warnings/errors.
- **First upload:** 
  - *No physical Pico board is connected right now.*
  - The driver installer Zadig is prepared and located at `D:\cMCU\drivers\zadig-2.9.exe`. When the board is plugged in for the first time in **BOOTSEL** mode, I will launch Zadig, select **RP2 Boot (Interface 1)**, select **WinUSB**, and install the driver to enable uploading via PlatformIO.
