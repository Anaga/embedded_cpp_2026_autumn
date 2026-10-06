# Lesson 11 homework - the Raspberry Pi Pico in PlatformIO

Next session we move from the ESP32-C3 to the Raspberry Pi Pico (RP2040).
The Pico toolchain is a large download, and setting it up is where things go
wrong. So you do it at home, with time to fix problems, and we start session 12
with every laptop ready.

Your task: get your Pico building, uploading and blinking from PlatformIO, and
write down what happened on the way.

## What you take home

- One Raspberry Pi Pico (the original board, RP2040).
- `homework/platformio.ini` for the Pico, in this folder.

The board has no headers yet. That is fine: for this homework you only need
the USB cable.

- Hold it by the edges. Do not put it down on anything metal.
- Keep it in its antistatic bag when you are not using it.
- Do not solder the headers at home. We do that together in session 12.
- Bring it back to session 12.

## Part 1 - prepare your computer

You already have VS Code, PlatformIO and git from the first sessions.
PlatformIO needs git to fetch the Pico platform, so check that `git --version`
works in a terminal.

**Windows only: turn on long file paths first.** Without this, the
installation fails with "Filename too long".

1. Open a terminal **as Administrator** and run:

   ```
   git config --system core.longpaths true
   ```

2. In the same Administrator terminal (PowerShell), run:

   ```
   New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
   ```

3. Restart the computer.

macOS needs no preparation. On Linux, PlatformIO needs udev rules to reach the
board - see the Arduino-Pico installation guide.

## Part 2 - first build

1. Pull the cohort repository.
2. Create your folder: `lesson-11/homework/<your-name>/src/`.
3. Open the `lesson-11/homework` folder in VS Code.
4. In `platformio.ini`, uncomment **only** your own `src_dir` line.
5. Write `main.cpp` in your `src` folder (the task is below).
6. Build.

The first build downloads the platform, the Arduino-Pico core and the
compiler. It takes a long time. Start it on a good connection, not a phone
hotspot, and let it finish. Later builds are fast.

## Part 3 - first upload

1. Hold the **BOOTSEL** button on the Pico.
2. While holding it, plug the USB cable into the computer.
3. Release the button. A drive called `RPI-RP2` appears.
4. Upload from PlatformIO.

After this first time you do not need BOOTSEL any more: PlatformIO resets the
board through its USB serial port before each upload.

**If the upload fails:** put the board into BOOTSEL mode again (steps 1-3),
then copy `.pio/build/pico/firmware.uf2` onto the `RPI-RP2` drive by hand. The
board restarts and runs your program. Write down that you needed this.

## The task

Write `<your-name>/src/main.cpp` for the Pico.

1. The on-board LED (`LED_BUILTIN`, GP25 on the original Pico) blinks:
   500 ms on, 500 ms off.
2. No `delay()` in `loop()`. Use the `millis()` pattern from K3.
3. Once per second, print one line to the serial monitor with the time
   since start in seconds, for example:

   ```
   Pico alive, uptime 12 s
   ```

4. Course style: named constants at the top, fixed-width types (`uint32_t`),
   `U` suffixes on constants, ASCII only - also in comments and Serial output.

**Bonus:** print the CPU clock frequency once at startup. The function is in
the Arduino-Pico documentation - find it there.

## Write down what happened

Create `<your-name>/pico-setup.md` with short answers:

- Operating system and version
- How long the first build took
- Every error you saw, and what fixed it - or that nothing did
- Did the first upload work, or did you need the manual UF2 copy?

If something does not work and you cannot fix it, write the exact error
message into `pico-setup.md` and push anyway. A clearly documented failure is
a valid result for this homework: it tells us what to fix before session 12.

## Done when

- [ ] The first build finished without errors
- [ ] The LED blinks once per second and a line appears every second
- [ ] `<your-name>/src/main.cpp` and `<your-name>/pico-setup.md` are committed
      and pushed (not `platformio.ini`)
- [ ] The Pico is back in its bag and comes with you to session 12
