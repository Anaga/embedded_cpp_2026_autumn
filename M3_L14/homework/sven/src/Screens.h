#pragma once

#include <Adafruit_PCD8544.h>

#include "Aht10.h"

class NowScreen {
public:
    explicit NowScreen(Adafruit_PCD8544 &lcd);

    void show(bool valid, float temperature, float humidity);
    void update(bool valid, float temperature, float humidity);

private:
    Adafruit_PCD8544 &m_lcd;
};

class HistoryScreen {
public:
    explicit HistoryScreen(Adafruit_PCD8544 &lcd);

    void show(const Aht10::History &history);

private:
    Adafruit_PCD8544 &m_lcd;
};
