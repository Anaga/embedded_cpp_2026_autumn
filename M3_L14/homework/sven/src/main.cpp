// Climate station: AHT10 readings and temperature history on a Nokia 5110.
//
// Wiring:
//   5110: RST GP21, CE GP17, DC GP20, DIN GP19, CLK GP18, VCC 3V3 OUT, GND
//   AHT10: SDA GP4, SCL GP5, VIN 3V3 OUT, GND
//   Button: GP14 / GND, using the internal pull-up

#include <Arduino.h>
#include <stdint.h>

#include "Aht10.h"
#include "Button.h"
#include "Screens.h"

static const uint8_t PIN_LCD_CS = 17U;
static const uint8_t PIN_LCD_DC = 20U;
static const uint8_t PIN_LCD_RST = 21U;
static const uint8_t PIN_BUTTON = 14U;
static const uint8_t PIN_HEARTBEAT = LED_BUILTIN;
static const uint8_t LCD_CONTRAST = 0x31U;
static const uint8_t LCD_BIAS = 0x04U;
static const uint32_t SERIAL_BAUD = 115200U;
static const uint8_t DECIMAL_PLACES = 1U;
static const uint32_t READ_PERIOD_MS = 1000U;
static const uint32_t BLINK_PERIOD_MS = 500U;

enum class Screen : uint8_t { Now, History };

Adafruit_PCD8544 lcd(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_RST);
Aht10 aht;
Button button(PIN_BUTTON);
NowScreen nowScreen(lcd);
HistoryScreen historyScreen(lcd);

static Screen activeScreen = Screen::Now;
static uint32_t lastReadMs = 0U;
static uint32_t lastBlinkMs = 0U;
static bool ledOn = false;

static void showActiveScreen(void) {
    if (activeScreen == Screen::Now) {
        nowScreen.show(aht.valid(), aht.temperature(), aht.humidity());
    } else {
        historyScreen.show(aht.history());
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    pinMode(PIN_HEARTBEAT, OUTPUT);
    button.begin();

    aht.begin(); // Wire defaults to I2C0 on GP4 / GP5.
    lcd.begin(LCD_CONTRAST, LCD_BIAS);
    showActiveScreen();
    lastReadMs = millis();
    lastBlinkMs = lastReadMs;
}

void loop() {
    if (button.wasPressed()) {
        activeScreen = activeScreen == Screen::Now ? Screen::History : Screen::Now;
        showActiveScreen();
        Serial.printf("Button pressed\n");
    }

    const uint32_t now = millis();
    if (now - lastReadMs >= READ_PERIOD_MS) {
        lastReadMs = now;
        aht.read(); // The library waits for conversion; history updates on both screens.

        if (activeScreen == Screen::Now) {
            nowScreen.update(aht.valid(), aht.temperature(), aht.humidity());
        } else if (aht.historyUpdated()) {
            historyScreen.show(aht.history());
        }

        if (aht.valid()) {
            char temperature[READING_TEXT_SIZE];
            char humidity[READING_TEXT_SIZE];
            dtostrf(aht.temperature(), 0, DECIMAL_PLACES, temperature);
            dtostrf(aht.humidity(), 0, DECIMAL_PLACES, humidity);
            Serial.printf("T %s C  RH %s %%\n", temperature, humidity);
        } else {
            Serial.printf("sensor missing\n");
        }
    }

    if (now - lastBlinkMs >= BLINK_PERIOD_MS) {
        lastBlinkMs = now;
        ledOn = !ledOn;
        digitalWrite(PIN_HEARTBEAT, ledOn ? HIGH : LOW);
    }
}
