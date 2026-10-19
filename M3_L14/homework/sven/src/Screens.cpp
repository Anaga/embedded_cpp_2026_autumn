#include "Screens.h"

static const uint8_t FONT_SIZE = 1U;
static const uint8_t LINE_HEIGHT_PX = 8U;
static const uint8_t TEMP_Y = 2U * LINE_HEIGHT_PX;
static const uint8_t HUMIDITY_Y = 3U * LINE_HEIGHT_PX;
static const uint8_t VALUE_X = 24U;
static const uint8_t UNIT_X = 72U;
static const uint8_t VALUE_WIDTH = UNIT_X - VALUE_X;
static const uint8_t DECIMAL_PLACES = 1U;

static const uint8_t SCALE_LABEL_WIDTH = 30U;  // Five characters, e.g. -50.0.
static const uint8_t TICK_WIDTH = 3U;
static const uint8_t AXIS_X = SCALE_LABEL_WIDTH + 1U;
static const uint8_t CHART_X = AXIS_X + TICK_WIDTH;
static const uint8_t CHART_WIDTH = LCDWIDTH - CHART_X;
static const uint8_t BAR_WIDTH = 2U;
static const uint8_t BAR_GAP = 1U;
static const uint8_t BAR_STEP = BAR_WIDTH + BAR_GAP;
static const uint8_t CHART_HEIGHT = LCDHEIGHT;
static const uint8_t MIN_BAR_HEIGHT = 2U;

static_assert(Aht10::HISTORY_CAPACITY * BAR_STEP <= CHART_WIDTH,
              "History bars must fit beside the scale");

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

void HistoryScreen::drawScaleLabel(float value, uint8_t tickY) {
    int16_t labelY = static_cast<int16_t>(tickY)
                      - static_cast<int16_t>(LINE_HEIGHT_PX / 2U);
    if (labelY < 0) {
        labelY = 0U;
    } else if (labelY > LCDHEIGHT - LINE_HEIGHT_PX) {
        labelY = LCDHEIGHT - LINE_HEIGHT_PX;
    }

    m_lcd.setCursor(0U, labelY);
    m_lcd.print(value, DECIMAL_PLACES);
    m_lcd.drawFastHLine(AXIS_X, tickY, TICK_WIDTH, BLACK);
}

void HistoryScreen::show(const Aht10::History &history) {
    m_lcd.clearDisplay();
    m_lcd.setTextSize(FONT_SIZE);
    m_lcd.setTextColor(BLACK);
    m_lcd.setTextWrap(false);

    const uint8_t count = history.count();
    if (count == 0U) {
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
    const float range = highest - lowest;
    m_lcd.drawFastVLine(AXIS_X, 0U, CHART_HEIGHT, BLACK);
    drawScaleLabel(lowest, LCDHEIGHT - MIN_BAR_HEIGHT);
    if (range > 0.0F) {
        drawScaleLabel(highest, 0U);
        const uint8_t middleY = LCDHEIGHT - MIN_BAR_HEIGHT
                                - (CHART_HEIGHT - MIN_BAR_HEIGHT) / 2U;
        drawScaleLabel((lowest + highest) / 2.0F, middleY);
    }

    const uint8_t firstX = LCDWIDTH - count * BAR_STEP;
    for (uint8_t i = 0U; i < count; ++i) {
        uint8_t height = MIN_BAR_HEIGHT;
        if (range > 0.0F) {
            const float fraction = (history.at(i) - lowest) / range;
            height += static_cast<uint8_t>(fraction * (CHART_HEIGHT - MIN_BAR_HEIGHT));
        }
        m_lcd.fillRect(firstX + i * BAR_STEP, LCDHEIGHT - height, BAR_WIDTH, height, BLACK);
    }
    m_lcd.display();
}
