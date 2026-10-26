// Lesson 15 demo - three buses and a potentiometer on one Pico
//
//   I2C  (GP4 / GP5)        AHT10 temperature and humidity, Adafruit AHTX0
//   SPI  (GP16..GP21)       Nokia 5110, Adafruit PCD8544 + GFX
//   UART (GP0, Serial1)     one status line per second, same as on USB
//   ADC  (GP26)             potentiometer -> setpoint 15.0 .. 30.0 C
//
// Once per second: read the setpoint, read the sensor, redraw only the
// values that changed, print one line. No delay() in loop().
//
// This is the capture target for the logic analyzer in session 15 and the
// starting point for the homework (room thermostat).
//
// Homework question - what happens at the edge of the band?
// At T close to SP +/- 0.5 C, small measurement fluctuations can make the
// LED alternate between OK and HEAT or between OK and COOL. This is called
// chatter. A real thermostat uses hysteresis: separate thresholds for entering
// and leaving a state, so tiny changes do not repeatedly switch equipment.

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <hardware/spi.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Pins (course pin map, settled in session 12)
// ---------------------------------------------------------------------------

static const uint8_t PIN_POT = 26U;      // ADC0
static const uint8_t PIN_LED_RED = 10U;   // common-anode RGB LED, active LOW
static const uint8_t PIN_LED_GREEN = 11U;
static const uint8_t PIN_LED_BLUE = 12U;
static const uint8_t PIN_LCD_CS = 17U;   // SPI0 CSn position, driven as GPIO
static const uint8_t PIN_LCD_DC = 20U;
static const uint8_t PIN_LCD_RST = 21U;

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

static const uint32_t UART_BAUD = 115200UL;
static const uint32_t TICK_PERIOD_MS = 1000U;

static const uint8_t AHT10_ADDRESS = 0x38U;

static const uint8_t LCD_CONTRAST = 50U;  // Vop: use the value you found in session 14
static const uint8_t LCD_BIAS = 4U;       // 0x14 on the wire, as in the course init

// The library always asks for 4 MHz. An SCK of 4 MHz needs 24 MHz sampling
// on the analyzer. If your capture aborts at 24 MHz, set this to 1000000UL
// and sample at 8 MHz.
static const uint32_t SPI_CLOCK_CAP_HZ = 4000000UL;

// Setpoint in tenths of a degree: 150 .. 300 in steps of 5 (0.5 C).
static const int32_t SETPOINT_MIN_DECI = 150;
static const int32_t SETPOINT_MAX_DECI = 300;
static const int32_t SETPOINT_STEP_DECI = 5;
static const int32_t THERMOSTAT_BAND_DECI = 5; // 0.5 C in tenths
static const uint32_t ADC_MAX = 1023U;    // analogRead() is 10 bits by default

// Text layout: 6 x 8 pixels per character, 14 columns x 6 rows.
static const uint8_t CHAR_W = 6U;
static const uint8_t CHAR_H = 8U;
static const uint8_t VALUE_COL = 3U;      // values start in column 3
static const uint8_t VALUE_CHARS = 5U;    // "-12.3" fits
static const uint8_t UNIT_COL = 9U;
static const uint8_t ROW_TITLE = 0U;
static const uint8_t ROW_STATE = 1U;
static const uint8_t ROW_T = 2U;
static const uint8_t ROW_RH = 3U;
static const uint8_t ROW_SP = 4U;
static const uint8_t ROW_STATUS = 5U;
static const uint8_t STATUS_CHARS = 14U;

// ---------------------------------------------------------------------------
// SPI with a clock cap
// ---------------------------------------------------------------------------

// Same SPI0 block and pins as the global SPI object. Every transaction the
// library opens goes through beginTransaction(), so this is the one place
// where the clock can be limited without touching the library.
class ClockCappedSpi : public SPIClassRP2040 {
public:
    ClockCappedSpi()
        : SPIClassRP2040(spi0, PIN_SPI0_MISO, PIN_SPI0_SS, PIN_SPI0_SCK, PIN_SPI0_MOSI) {}

    void beginTransaction(SPISettings settings) override {
        uint32_t hz = settings.getClockFreq();
        if (hz > SPI_CLOCK_CAP_HZ) {
            hz = SPI_CLOCK_CAP_HZ;
        }
        SPIClassRP2040::beginTransaction(
            SPISettings(hz, settings.getBitOrder(), settings.getDataMode()));
    }
};

static ClockCappedSpi lcdSpi;
static Adafruit_PCD8544 lcd(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_RST, &lcdSpi);
static Adafruit_AHTX0 aht;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

struct Reading {
    bool valid;
    int32_t tempDeci;      // 23.4 C -> 234
    int32_t humidityDeci;  // 41.2 % -> 412
};

// A value on the screen. Redrawn only when its text changes, so display()
// sends only the changed rectangle.
struct Field {
    uint8_t row;
    char shown[VALUE_CHARS + 1U];
};

enum class ThermostatState : uint8_t {
    MISSING,
    HEAT,
    OK,
    COOL
};

static ThermostatState shownState = ThermostatState::MISSING;
static bool stateDrawn = false;

static bool sensorOk = false;
static bool statusShown = false;
static bool statusDrawn = false;
static uint32_t lastTickMs = 0U;

static Field fieldT = { ROW_T, "" };
static Field fieldRh = { ROW_RH, "" };
static Field fieldSp = { ROW_SP, "" };

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Select an indicator state from a fresh sensor reading.
static ThermostatState decideState(const Reading& r, int32_t spDeci) {
    if (!r.valid) {
        return ThermostatState::MISSING;
    }
    if (r.tempDeci < (spDeci - THERMOSTAT_BAND_DECI)) {
        return ThermostatState::HEAT;
    }
    if (r.tempDeci > (spDeci + THERMOSTAT_BAND_DECI)) {
        return ThermostatState::COOL;
    }
    return ThermostatState::OK;
}

static const char* stateName(ThermostatState state) {
    switch (state) {
        case ThermostatState::HEAT: return "HEAT";
        case ThermostatState::OK: return "OK";
        case ThermostatState::COOL: return "COOL";
        default: return "MISSING";
    }
}

// Common anode: HIGH is off, LOW lights that colour.
static void setLed(ThermostatState state) {
    digitalWrite(PIN_LED_RED, state == ThermostatState::HEAT ? LOW : HIGH);
    digitalWrite(PIN_LED_GREEN, state == ThermostatState::OK ? LOW : HIGH);
    digitalWrite(PIN_LED_BLUE, state == ThermostatState::COOL ? LOW : HIGH);
}

// Tenths -> "23.4", "-5.0". Integer only, no float formatting needed.
static void formatDeci(char* out, size_t size, int32_t deci) {
    const bool negative = (deci < 0);
    const uint32_t mag = static_cast<uint32_t>(negative ? -deci : deci);
    snprintf(out, size, "%s%lu.%lu", negative ? "-" : "",
             static_cast<unsigned long>(mag / 10U),
             static_cast<unsigned long>(mag % 10U));
}

static int32_t readSetpointDeci(void) {
    const uint32_t raw = static_cast<uint32_t>(analogRead(PIN_POT));
    const uint32_t steps =
        static_cast<uint32_t>((SETPOINT_MAX_DECI - SETPOINT_MIN_DECI) / SETPOINT_STEP_DECI);
    const uint32_t index = (raw * steps + ADC_MAX / 2U) / ADC_MAX;  // 0 .. steps, rounded
    return SETPOINT_MIN_DECI + static_cast<int32_t>(index) * SETPOINT_STEP_DECI;
}

// The library call blocks for the whole conversion (about 50 - 80 ms): it
// polls the busy bit every 10 ms. Fine for a demo, visible on the analyzer.
static Reading readSensor(void) {
    Reading r = { false, 0, 0 };

    if (!sensorOk) {
        sensorOk = aht.begin(&Wire, 0, AHT10_ADDRESS);  // retry once per tick
    }
    if (sensorOk) {
        sensors_event_t humidity;
        sensors_event_t temp;
        if (aht.getEvent(&humidity, &temp)) {
            r.valid = true;
            r.tempDeci = static_cast<int32_t>(lroundf(temp.temperature * 10.0f));
            r.humidityDeci = static_cast<int32_t>(lroundf(humidity.relative_humidity * 10.0f));
        } else {
            sensorOk = false;
        }
    }
    return r;
}

static void drawText(uint8_t col, uint8_t row, const char* text) {
    lcd.setCursor(static_cast<int16_t>(col * CHAR_W), static_cast<int16_t>(row * CHAR_H));
    lcd.print(text);
}

// Row 1 changes only when the state changes.
static void showState(ThermostatState state) {
    if (stateDrawn && state == shownState) {
        return;
    }
    lcd.fillRect(0, static_cast<int16_t>(ROW_STATE * CHAR_H),
                 static_cast<int16_t>(STATUS_CHARS * CHAR_W),
                 static_cast<int16_t>(CHAR_H), WHITE);
    if (state != ThermostatState::MISSING) {
        drawText(0U, ROW_STATE, stateName(state));
    }
    shownState = state;
    stateDrawn = true;
}

static void showField(Field& f, const char* value) {
    char padded[VALUE_CHARS + 1U];
    snprintf(padded, sizeof(padded), "%5s", value);  // right-aligned

    if (strcmp(f.shown, padded) == 0) {
        return;  // unchanged: draw nothing, the rectangle stays small
    }
    lcd.fillRect(static_cast<int16_t>(VALUE_COL * CHAR_W), static_cast<int16_t>(f.row * CHAR_H),
                 static_cast<int16_t>(VALUE_CHARS * CHAR_W), static_cast<int16_t>(CHAR_H), WHITE);
    drawText(VALUE_COL, f.row, padded);
    strcpy(f.shown, padded);
}

static void showStatus(bool missing) {
    if (statusDrawn && missing == statusShown) {
        return;
    }
    lcd.fillRect(0, static_cast<int16_t>(ROW_STATUS * CHAR_H),
                 static_cast<int16_t>(STATUS_CHARS * CHAR_W), static_cast<int16_t>(CHAR_H), WHITE);
    if (missing) {
        drawText(0U, ROW_STATUS, "SENSOR MISSING");
    }
    statusShown = missing;
    statusDrawn = true;
}

static void drawStaticText(void) {
    drawText(0U, ROW_TITLE, "THERMOSTAT");
    drawText(0U, ROW_T, "T");
    drawText(0U, ROW_RH, "RH");
    drawText(0U, ROW_SP, "SP");
    drawText(UNIT_COL, ROW_T, " C");
    drawText(UNIT_COL, ROW_RH, " %");
    drawText(UNIT_COL, ROW_SP, " C");
}

// Same line on USB (for the monitor) and on UART0 (GP0, for the analyzer).
// Every line starts with 'T' (0x54) - the frame drawn on the slide.
static void printLine(const char* line) {
    Serial.println(line);
    Serial1.println(line);
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup() {
    // Set the three LED channels off before initializing other hardware.
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE, HIGH);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);

    Serial.begin(UART_BAUD);   // USB CDC, baud ignored
    Serial1.begin(UART_BAUD);  // UART0 on GP0 / GP1

    lcd.begin(LCD_CONTRAST, LCD_BIAS);  // shows the Adafruit splash once
    lcd.setTextColor(BLACK, WHITE);
    lcd.clearDisplay();
    drawStaticText();
    lcd.display();

    sensorOk = aht.begin(&Wire, 0, AHT10_ADDRESS);
    lastTickMs = millis() - TICK_PERIOD_MS;  // first tick right away
}

void loop() {
    const uint32_t now = millis();
    if ((now - lastTickMs) < TICK_PERIOD_MS) {
        return;
    }
    lastTickMs = now;

    const int32_t setpointDeci = readSetpointDeci();
    const Reading r = readSensor();
    const ThermostatState state = decideState(r, setpointDeci);
    setLed(state);

    char t[8];
    char rh[8];
    char sp[8];
    if (r.valid) {
        formatDeci(t, sizeof(t), r.tempDeci);
        formatDeci(rh, sizeof(rh), r.humidityDeci);
    } else {
        strcpy(t, "--.-");
        strcpy(rh, "--.-");
    }
    formatDeci(sp, sizeof(sp), setpointDeci);

    showField(fieldT, t);
    showField(fieldRh, rh);
    showField(fieldSp, sp);
    showStatus(!r.valid);
    showState(state);
    lcd.display();  // called every tick: with nothing changed it sends one command

    char line[64];
    snprintf(line, sizeof(line), "T=%s C RH=%s %% SP=%s C %s",
             t, rh, sp, r.valid ? stateName(state) : "sensor missing");
    printLine(line);
}
