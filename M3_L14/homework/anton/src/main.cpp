#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>

#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include "Button.h"
#include "RingBuffer.h"

// ---------------------------------------------------------------------------
// Pins (course pin map, settled in session 12)
// ---------------------------------------------------------------------------
static const uint8_t PIN_LCD_CS = 17U;
static const uint8_t PIN_LCD_CLK = 18U;
static const uint8_t PIN_LCD_DIN = 19U;
static const uint8_t PIN_LCD_DC = 20U;
static const uint8_t PIN_LCD_RST = 21U;
static const uint8_t PIN_BUTTON = 14U;
static const uint8_t PIN_HEARTBEAT = LED_BUILTIN; // GP25

// ---------------------------------------------------------------------------
// Settings and Constants
// ---------------------------------------------------------------------------
static const uint8_t LCD_CONTRAST = 0x31U; // Vop: tune for your module
static const uint8_t LCD_BIAS = 0x04U;     // the library sends 0x10 | bias

static const uint8_t CHAR_W = 6U;
static const uint8_t CHAR_H = 8U;
static const uint8_t LINE_TITLE = 0U;
static const uint8_t LINE_T = 2U;
static const uint8_t LINE_RH = 3U;
static const uint8_t LINE_STATUS = 5U;

static const uint32_t TICK_PERIOD_MS = 1000U;
static const uint32_t BLINK_PERIOD_MS = 500U;

static const uint8_t HISTORY_CAPACITY = 14U;
static const uint8_t SAMPLES_PER_BAR = 5U;
static const uint8_t BAR_WIDTH = 5U;
static const uint8_t BAR_GAP = 1U;
static const uint8_t BAR_STEP_WIDTH = BAR_WIDTH + BAR_GAP; // 6 pixels

static const uint8_t CHART_ROW_START_Y = 12U;
static const uint8_t CHART_HEIGHT_MAX = 36U; // Total vertical pixels under text line
static const uint8_t MIN_BAR_HEIGHT = 2U;

// Setpoint formatting
static const uint8_t VALUE_COL = 3U;
static const uint8_t VALUE_CHARS = 5U;

// ---------------------------------------------------------------------------
// Types and Enumerations
// ---------------------------------------------------------------------------
enum class Screen : uint8_t { Now, History };

// ---------------------------------------------------------------------------
// State and Objects
// ---------------------------------------------------------------------------
Adafruit_PCD8544 lcd(PIN_LCD_CLK, PIN_LCD_DIN, PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_RST);
Adafruit_AHTX0 aht;
Button button(PIN_BUTTON);

static Screen activeScreen = Screen::Now;
static bool sensorFound = false;

static uint32_t lastTickMs = 0U;
static uint32_t lastBlinkMs = 0U;
static bool ledOn = false;

// Running sum and count for the current 5-reading average
static float runningSumT = 0.0F;
static uint8_t readingsCount = 0U;

// History buffer
static RingBuffer<float, HISTORY_CAPACITY> historyBuffer;

// Track what is currently shown on Screen 1 to redraw only when changed
static char lastShownT[VALUE_CHARS + 1U] = "";
static char lastShownRh[VALUE_CHARS + 1U] = "";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void drawText(uint8_t col, uint8_t row, const char* text) {
    lcd.setCursor(static_cast<int16_t>(col * CHAR_W), static_cast<int16_t>(row * CHAR_H));
    lcd.print(text);
}

static void drawStaticLabelsNow() {
    lcd.clearDisplay();
    drawText(0U, LINE_TITLE, "CLIMATE NOW");
    drawText(0U, LINE_T, "T:");
    drawText(8U, LINE_T, "C");
    drawText(0U, LINE_RH, "RH:");
    drawText(8U, LINE_RH, "%");
    lcd.display();
}

static void updateValueNow(uint8_t row, const char* newValue, char* lastValue) {
    char padded[VALUE_CHARS + 1U];
    snprintf(padded, sizeof(padded), "%5s", newValue); // Right align

    if (strcmp(lastValue, padded) != 0) {
        // Clear area in white
        lcd.fillRect(static_cast<int16_t>(VALUE_COL * CHAR_W), static_cast<int16_t>(row * CHAR_H),
                     static_cast<int16_t>(VALUE_CHARS * CHAR_W), static_cast<int16_t>(CHAR_H), WHITE);
        drawText(VALUE_COL, row, padded);
        strcpy(lastValue, padded);
    }
}

static void drawHistoryScreen() {
    lcd.clearDisplay();

    uint8_t count = historyBuffer.count();
    if (count == 0U) {
        drawText(0U, LINE_TITLE, "NO HISTORY");
        lcd.display();
        return;
    }

    // Find min and max
    float minVal = historyBuffer.at(0U);
    float maxVal = historyBuffer.at(0U);
    for (uint8_t i = 1U; i < count; ++i) {
        float val = historyBuffer.at(i);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    // Print text line: lowest and highest value
    char titleBuf[15];
    char minStr[6];
    char maxStr[6];
    dtostrf(minVal, 0, 1, minStr);
    dtostrf(maxVal, 0, 1, maxStr);
    snprintf(titleBuf, sizeof(titleBuf), "T %s..%s", minStr, maxStr);
    drawText(0U, 0U, titleBuf);

    // Draw bars
    // Chart height available: everything under the text line.
    // Total height = 48 pixels. Text line is ROW 0 (0..7 Y pixels).
    // Let's start the chart area at Y = 8. Height is 48 - 8 = 40.
    // The bottom-most row is Y = 47.
    static const uint8_t CHART_BOTTOM_Y = 47U;
    static const uint8_t CHART_TOP_Y = 8U;
    static const uint8_t CHART_HEIGHT = CHART_BOTTOM_Y - CHART_TOP_Y + 1U; // 40 pixels

    float range = maxVal - minVal;

    for (uint8_t i = 0U; i < count; ++i) {
        float val = historyBuffer.at(i);
        uint8_t barHeight = MIN_BAR_HEIGHT;

        if (range > 0.001F) {
            float normalized = (val - minVal) / range;
            barHeight = static_cast<uint8_t>(MIN_BAR_HEIGHT + normalized * (CHART_HEIGHT - MIN_BAR_HEIGHT));
        } else {
            // When all averages are the same, draw bars with half height to avoid division by zero
            barHeight = CHART_HEIGHT / 2U;
        }

        uint8_t x = i * BAR_STEP_WIDTH;
        uint8_t y = CHART_BOTTOM_Y - barHeight + 1U;

        // Draw bar
        lcd.fillRect(static_cast<int16_t>(x), static_cast<int16_t>(y),
                     static_cast<int16_t>(BAR_WIDTH), static_cast<int16_t>(barHeight), BLACK);
    }

    lcd.display();
}

static void updateScreen(bool sensorOk, float tempC, float humidityPct) {
    if (activeScreen == Screen::Now) {
        if (sensorOk) {
            char tempStr[6];
            char humStr[6];
            dtostrf(tempC, 0, 1, tempStr);
            dtostrf(humidityPct, 0, 1, humStr);
            updateValueNow(LINE_T, tempStr, lastShownT);
            updateValueNow(LINE_RH, humStr, lastShownRh);
        } else {
            updateValueNow(LINE_T, "--.-", lastShownT);
            updateValueNow(LINE_RH, "--.-", lastShownRh);
        }
        lcd.display();
    }
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    pinMode(PIN_HEARTBEAT, OUTPUT);

    button.begin();
    Wire.begin(); // Default I2C0 on GP4 / GP5

    lcd.begin(LCD_CONTRAST, LCD_BIAS);
    lcd.setTextColor(BLACK, WHITE);
    lcd.clearDisplay();

    sensorFound = aht.begin();

    if (activeScreen == Screen::Now) {
        drawStaticLabelsNow();
    } else {
        drawHistoryScreen();
    }

    lastTickMs = millis();
    lastBlinkMs = lastTickMs;
}

void loop() {
    button.update();

    if (button.wasPressed()) {
        activeScreen = activeScreen == Screen::Now ? Screen::History : Screen::Now;
        if (activeScreen == Screen::Now) {
            // Reset cache to force immediate redraw of labels and values
            lastShownT[0] = '\0';
            lastShownRh[0] = '\0';
            drawStaticLabelsNow();
        }

        // Always read immediately on screen switch
        float tempC = 0.0F;
        float humidityPct = 0.0F;
        if (!sensorFound) {
            sensorFound = aht.begin();
        }
        if (sensorFound) {
            sensors_event_t humidity, temp;
            if (aht.getEvent(&humidity, &temp)) {
                tempC = temp.temperature;
                humidityPct = humidity.relative_humidity;
            } else {
                sensorFound = false;
            }
        }

        if (activeScreen == Screen::Now) {
            updateScreen(sensorFound, tempC, humidityPct);
        } else {
            drawHistoryScreen();
        }
    }

    const uint32_t now = millis();

    // A new reading every second
    if (now - lastTickMs >= TICK_PERIOD_MS) {
        lastTickMs = now;

        if (!sensorFound) {
            sensorFound = aht.begin();
        }

        float tempC = 0.0F;
        float humidityPct = 0.0F;
        bool readingOk = false;

        if (sensorFound) {
            sensors_event_t humidity, temp;
            if (aht.getEvent(&humidity, &temp)) {
                tempC = temp.temperature;
                humidityPct = humidity.relative_humidity;
                readingOk = true;

                // Process average calculation
                runningSumT += tempC;
                readingsCount++;

                if (readingsCount >= SAMPLES_PER_BAR) {
                    float avgT = runningSumT / static_cast<float>(SAMPLES_PER_BAR);
                    historyBuffer.push(avgT);

                    // Reset average
                    runningSumT = 0.0F;
                    readingsCount = 0U;

                    // If on history screen, redraw when a new bar arrives
                    if (activeScreen == Screen::History) {
                        drawHistoryScreen();
                    }
                }
            } else {
                sensorFound = false;
            }
        }

        if (activeScreen == Screen::Now) {
            updateScreen(readingOk, tempC, humidityPct);
        }

        // Print to Serial (ASCII only)
        if (readingOk) {
            char tempBuf[10];
            char humBuf[10];
            dtostrf(tempC, 0, 1, tempBuf);
            dtostrf(humidityPct, 0, 1, humBuf);
            Serial.printf("T %s C  RH %s %%\n", tempBuf, humBuf);
        } else {
            Serial.printf("sensor missing\n");
        }
    }

    // Heartbeat LED
    if (now - lastBlinkMs >= BLINK_PERIOD_MS) {
        lastBlinkMs = now;
        ledOn = !ledOn;
        digitalWrite(PIN_HEARTBEAT, ledOn ? HIGH : LOW);
    }
}
