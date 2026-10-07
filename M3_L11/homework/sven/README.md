Using Intellij CLion, the Platform IO plugin picked up required config from platform.ini with no problems.CLion

Once installed, build works.

Also upload works properly.

Made sure main.cpp follows README.md and blinks.

My board is a Pico 2. `sven/platformio.ini` overrides the shared `board = pico`
setting with `board = rpipico2`. In the homework root `platformio.ini`, enable
the following local settings:

```ini
[platformio]
extra_configs = sven/platformio.ini
src_dir = sven/src
```

Keep these root configuration edits uncommitted; commit only the files under
`sven/`. Continue opening the homework root in CLion and using the `pico`
environment for build and upload.
