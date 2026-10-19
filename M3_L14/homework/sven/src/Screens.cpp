#include "Screens.h"

static const uint8_t FONT_SIZE = 1U;
static const uint8_t LINE_HEIGHT_PX = 8U;
static const uint8_t TEMP_Y = 2U * LINE_HEIGHT_PX;
static const uint8_t HUMIDITY_Y = 3U * LINE_HEIGHT_PX;
static const uint8_t VALUE_X = 24U;
static const uint8_t UNIT_X = 72U;
static const uint8_t VALUE_WIDTH = UNIT_X - VALUE_X;
static const uint8_t DECIMAL_PLACES = 1U;

static const uint8_t BAR_WIDTH = 5U;
static const uint8_t BAR_GAP = 1U;
static const uint8_t BAR_STEP = BAR_WIDTH + BAR_GAP;
static const uint8_t CHART_HEIGHT = LCDHEIGHT - LINE_HEIGHT_PX;
static const uint8_t MIN_BAR_HEIGHT = 2U;

static_assert(Aht10::HISTORY_CAPACITY * BAR_STEP == LCDWIDTH,
              "History bars must fill the LCD width");

NowScreen::NowScreen(Adafruit_PCD8544 &lcd) : m_lcd(lcd) {
}

void NowScreen::show(bool valid, float temperature, float humidity) {
    m_lcd.clearDisplay();
    m_lcd.setTextSize(FONT_SIZE);
    m_lcd.setTextColor(BLACK);
    m_lcd.setTextWrap(false);

    m_lcd.setCursor(0U, 0U);
    m_lcd.print("Now");
    m_lcd.setCursor(0U, TEMP_Y);
    m_lcd.print("T");
    m_lcd.setCursor(UNIT_X, TEMP_Y);
    m_lcd.print("C");
    m_lcd.setCursor(0U, HUMIDITY_Y);
    m_lcd.print("RH");
    m_lcd.setCursor(UNIT_X, HUMIDITY_Y);
    m_lcd.print("%");

    update(valid, temperature, humidity);
}

void NowScreen::update(bool valid, float temperature, float humidity) {
    m_lcd.fillRect(VALUE_X, TEMP_Y, VALUE_WIDTH, LINE_HEIGHT_PX, WHITE);
    m_lcd.fillRect(VALUE_X, HUMIDITY_Y, VALUE_WIDTH, LINE_HEIGHT_PX, WHITE);

    m_lcd.setCursor(VALUE_X, TEMP_Y);
    if (valid) {
        m_lcd.print(temperature, DECIMAL_PLACES);
    } else {
        m_lcd.print("--");
    }
    m_lcd.setCursor(VALUE_X, HUMIDITY_Y);
    if (valid) {
        m_lcd.print(humidity, DECIMAL_PLACES);
    } else {
        m_lcd.print("--");
    }
    m_lcd.display();
}

HistoryScreen::HistoryScreen(Adafruit_PCD8544 &lcd) : m_lcd(lcd) {
}

void HistoryScreen::show(const Aht10::History &history) {
    m_lcd.clearDisplay();
    m_lcd.setTextSize(FONT_SIZE);
    m_lcd.setTextColor(BLACK);
    m_lcd.setTextWrap(false);
    m_lcd.setCursor(0U, 0U);
    m_lcd.print("T ");

    const uint8_t count = history.count();
    if (count == 0U) {
        m_lcd.print("--..--");
        m_lcd.display();
        return;
    }

    float lowest = history.at(0U);
    float highest = lowest;
    for (uint8_t i = 1U; i < count; ++i) {
        const float value = history.at(i);
        if (value < lowest) {
            lowest = value;
        }
        if (value > highest) {
            highest = value;
        }
    }
    m_lcd.print(lowest, DECIMAL_PLACES);
    m_lcd.print("..");
    m_lcd.print(highest, DECIMAL_PLACES);

    const float range = highest - lowest;
    const uint8_t firstX = (Aht10::HISTORY_CAPACITY - count) * BAR_STEP;
    for (uint8_t i = 0U; i < count; ++i) {
        uint8_t height = MIN_BAR_HEIGHT;
        if (range > 0.0F) {
            const float fraction = (history.at(i) - lowest) / range;
            height += static_cast<uint8_t>(fraction * (CHART_HEIGHT - MIN_BAR_HEIGHT));
        }
        m_lcd.fillRect(firstX + i * BAR_STEP, LCDHEIGHT - height,
                       BAR_WIDTH, height, BLACK);
    }
    m_lcd.display();
}
