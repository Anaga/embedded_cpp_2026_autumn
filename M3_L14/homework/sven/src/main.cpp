// Lesson 14 demo - the AHT10 reading on the Nokia 5110, through libraries.
//
// Adafruit PCD8544 + Adafruit GFX draw into a 504-byte framebuffer in RAM.
// display() sends the changed part of that buffer to the display over SPI0.
// Adafruit AHTX0 reads the sensor over I2C0.
//
// This is the library version for comparison. The by-hand driver (reset,
// init, ChipSelect, font table) is the lab, so it is not here.
//
// Wiring (course pin map):
//   5110: RST GP21, CE GP17, DC GP20, DIN GP19, CLK GP18, VCC 3V3 OUT, GND
//   AHT10: SDA GP4, SCL GP5, VIN 3V3 OUT, GND

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>

#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

static const uint8_t PIN_LCD_CS = 17U;   // CE on the module
static const uint8_t PIN_LCD_DC = 20U;
static const uint8_t PIN_LCD_RST = 21U;
static const uint8_t PIN_HEARTBEAT = LED_BUILTIN;  // GP25 (PIN_LED is a core macro)

static const uint8_t LCD_CONTRAST = 0x31U;  // Vop: tune for your module
static const uint8_t LCD_BIAS = 0x04U;      // the library sends 0x10 | bias

static const uint8_t LINE_HEIGHT_PX = 8U;   // default GFX font: 6 x 8 cells
static const uint8_t LINE_TITLE = 0U;
static const uint8_t LINE_TEMP = 2U;
static const uint8_t LINE_HUMIDITY = 3U;
static const uint8_t LINE_STATUS = 5U;

static const uint32_t READ_PERIOD_MS = 2000U;
static const uint32_t BLINK_PERIOD_MS = 500U;
static const uint32_t MS_PER_S = 1000U;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

Adafruit_PCD8544 lcd(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_RST);  // hardware SPI0
Adafruit_AHTX0 aht;

static bool sensorFound = false;
static uint32_t lastReadMs = 0U;
static uint32_t lastBlinkMs = 0U;
static bool ledOn = false;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void cursorToLine(uint8_t line)
{
    lcd.setCursor(0, static_cast<int16_t>(line * LINE_HEIGHT_PX));
}

// Draws one screen into the buffer, sends it, returns how long display() took.
static uint32_t drawScreen(bool ok, float tempC, float humidityPct)
{
    lcd.clearDisplay();  // clears the buffer and marks all of it as changed

    cursorToLine(LINE_TITLE);
    lcd.print("AHT10 -> 5110");

    if (ok) {
        cursorToLine(LINE_TEMP);
        lcd.print("T   ");
        lcd.print(tempC, 1);
        lcd.print(" C");
        cursorToLine(LINE_HUMIDITY);
        lcd.print("RH  ");
        lcd.print(humidityPct, 1);
        lcd.print(" %");
    } else {
        cursorToLine(LINE_TEMP);
        lcd.print("SENSOR MISSING");
    }

    cursorToLine(LINE_STATUS);
    lcd.print("up ");
    lcd.print(static_cast<unsigned long>(millis() / MS_PER_S));
    lcd.print(" s");

    const uint32_t startUs = micros();
    lcd.display();  // only now does anything travel over SPI
    return micros() - startUs;
}

static void readAndShow()
{
    if (!sensorFound) {
        sensorFound = aht.begin();  // retry until the sensor answers
    }

    float tempC = 0.0F;
    float humidityPct = 0.0F;
    if (sensorFound) {
        sensors_event_t humidity;
        sensors_event_t temp;
        aht.getEvent(&humidity, &temp);  // blocks for the conversion
        tempC = temp.temperature;
        humidityPct = humidity.relative_humidity;
    }

    const uint32_t lcdUs = drawScreen(sensorFound, tempC, humidityPct);

    if (sensorFound) {
        Serial.print("T ");
        Serial.print(tempC, 1);
        Serial.print(" C  RH ");
        Serial.print(humidityPct, 1);
        Serial.print(" %");
    } else {
        Serial.print("sensor missing");
    }
    Serial.print("  display() took ");
    Serial.print(static_cast<unsigned long>(lcdUs));
    Serial.println(" us");
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    pinMode(PIN_HEARTBEAT, OUTPUT);

    Wire.begin();                       // I2C0 on GP4 / GP5
    lcd.begin(LCD_CONTRAST, LCD_BIAS);  // reset, init, shows the splash image
    sensorFound = aht.begin();

    readAndShow();
    lastReadMs = millis();
}

void loop()
{
    const uint32_t now = millis();

    if (now - lastReadMs >= READ_PERIOD_MS) {
        lastReadMs = now;
        readAndShow();
    }

    if (now - lastBlinkMs >= BLINK_PERIOD_MS) {
        lastBlinkMs = now;
        ledOn = !ledOn;
        digitalWrite(PIN_HEARTBEAT, ledOn ? HIGH : LOW);
    }
}
