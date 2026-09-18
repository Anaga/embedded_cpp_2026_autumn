# Google Test version of the homework tests

This is the local test project for the homework. It contains an Arduino
substitute, parameterized conversion cases and ADC, averaging, timing,
deadband and output tests.

The test source includes the original `../src/main.cpp` once. No changes to the
homework source or the firmware configuration are needed.
No homework solution is supplied here.

## Run and debug in CLion

1. Use **File > Open** to open this directory's `CMakeLists.txt` as a project.
   Choose a new window to keep the existing firmware project open.
2. Use your local compiler/toolchain and the **Debug** CMake profile. Keep the
   build directory inside `tests`, using `cmake-build-clion-debug` for CLion.
3. Allow CMake to download Google Test 1.17.0 on the first configuration.
4. Run a test or suite using the green gutter icon in
   `test/test_temperature/test_main.cpp`, or select the Google Test configuration
   for `temperature_tests` in the toolbar.
5. Use **Debug** for breakpoints in the tests or the original homework source.

If the toolbar selects **gtest**, that is the framework library target. Create
a configuration for the homework tests:

1. Click the **gtest** dropdown in the top toolbar and select **Edit
   Configurations**, or use **Run > Edit Configurations**.
2. Click **+ > Google Test**.
3. Name the configuration **Temperature tests**.
4. Set both **Target** and **Executable** to **temperature_tests**, and choose
   to run all tests.
5. Click **Apply**, then **Run**.

The **CMake** panel showing **Configuring done**, **Generating done** and
**[Finished]** means project configuration succeeded. Running **Temperature
tests** opens the test-results panel with individual passing and failing tests.

Conversion cases have readable names, for example
`Temperatures/CelsiusConversion.ConvertsToWholeDegrees/FMinus40ToCMinus40`.
You can inspect individual results and rerun failed cases in CLion's test window.

### Fix an incompatible CMake generator

If CLion reports **Cannot generate into ...** and **It was created with
incompatible generator 'Unix Makefiles'**, the build directory was previously
configured with a different generator, such as a terminal build using Makefiles
while CLion uses Ninja.

1. Open **Settings > Build, Execution, Deployment > CMake**.
2. Select the **Debug** profile.
3. Set **Build directory** to `cmake-build-clion-debug`, separate from the
   directory with the incompatible cache.
4. Click **Apply** and allow CMake to reload.

If that directory also has an incompatible cache, choose another unused
`cmake-build-*` directory. These build directories stay inside `tests` and are
covered by its `.gitignore`.

## Run with PlatformIO

From the parent `homework` directory:

```sh
~/.platformio/penv/bin/pio test -d homework_sven/tests -e native
```

Or from this `tests` directory:

```sh
~/.platformio/penv/bin/pio test -e native
```

Use `pio` instead of the full path if it is on your PATH. PlatformIO installs
Google Test 1.17.0 in this project's `.pio` directory on the first run.

To run only conversion cases from `tests`:

```sh
~/.platformio/penv/bin/pio test -e native -a '--gtest_filter=Temperatures/CelsiusConversion.*'
```

## Adding tests and interpreting results

Add conversion cases in `test/test_temperature/test_main.cpp` by extending
`CELSIUS_CASES` with `{fahrenheit, expected_celsius}` rows. Use independently
calculated expected values that follow the required rounding behavior. Include
boundary values, values just either side of a boundary and representative
intermediate inputs. `TEST_P` defines the check once, and `ValuesIn` registers
each row as a separately named test; no manual runner registration is needed.

For other behavior, add a `TEST_F(HomeworkTest, DescriptiveName)` in the same
file. Set up the input, call the function under test and assert the expected
result. Use `fake_arduino` to supply ADC readings, advance simulated time and
inspect captured serial output. `HomeworkTest::SetUp()` resets the fake hardware
and firmware globals before each test, including each conversion case, so tests
must establish their own state.

Run the tests through CLion or PlatformIO and interpret the results as follows:

- **Passed:** the assertions matched for that test's inputs. This covers the
  tested scenario, not every possible input or hardware condition.
- **Failed:** inspect the test name, assertion location, expected value and
  actual value. For conversion cases, the name identifies the input and expected
  output. Check both the implementation and the expectation against the
  requirements, then rerun the affected test and the full suite.
- **Build or configuration error:** resolve the reported compiler, dependency
  or CMake error before interpreting test results; the tests have not run.

The simulated hardware checks ordinary C++ behavior. Physical ADC behavior,
real USB output and hardware timing still need verification on the board.

## Generated files and Git exclusions

The PlatformIO and CLion builds are independent and can coexist. Generated
files stay in `.pio` and `cmake-build-clion-debug` under this directory. The local
`.gitignore` excludes PlatformIO and CMake build directories, local CLion settings
and macOS metadata. This checkout also excludes the entire `tests` directory
through Git's local `.git/info/exclude`, keeping the test project out of commits
and pushes.
