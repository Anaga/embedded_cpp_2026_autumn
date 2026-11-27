#pragma once
#include "Config.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <math.h>

class TemperatureMonitor {
public:
  void begin(uint32_t now) {
    Wire.begin(cfg::PIN_AHT_SDA, cfg::PIN_AHT_SCL);
    Wire.setClock(100000UL);
    Wire.setTimeOut(50);
    lastRetryMs_ = now - cfg::SENSOR_RETRY_MS;
    lastReadMs_ = now - cfg::TEMP_SAMPLE_MS;
  }
  void update(uint32_t now) {
    if (!connected_) {
      if ((uint32_t)(now - lastRetryMs_) < cfg::SENSOR_RETRY_MS) return;
      lastRetryMs_ = now;
      connected_ = sensor_.begin(&Wire);
      if (!connected_) {
        ok_ = false;
        Serial.println("ERROR: AHT10 not found; grow lights held OFF");
      } else {
        Serial.println("AHT10 connected");
        lastReadMs_ = now - cfg::TEMP_SAMPLE_MS;
      }
      return;
    }
    if ((uint32_t)(now - lastReadMs_) < cfg::TEMP_SAMPLE_MS) return;
    lastReadMs_ = now;
    sensors_event_t h, t;
    const bool success = sensor_.getEvent(&h, &t);
    const float temperature = t.temperature;
    const float humidity = h.relative_humidity;
    ok_ = success && isfinite(temperature) && isfinite(humidity) &&
          temperature >= -10.0f && temperature <= 60.0f &&
          humidity >= 0.0f && humidity <= 100.0f;
    if (!ok_) {
      connected_ = false;
      Serial.println("ERROR: AHT10 read failed; grow lights held OFF");
      return;
    }
    temperatureC_ = temperature;
    humidityPct_ = humidity;
    if (!thermalPause_ && temperatureC_ >= cfg::TEMP_LIGHTS_OFF_C) {
      thermalPause_ = true;
      pauseStartMs_ = now;
      Serial.println("HEAT: grow lights OFF for at least 300 s");
    }
    if (thermalPause_ && (uint32_t)(now - pauseStartMs_) >= cfg::TEMP_MIN_OFF_MS &&
        temperatureC_ <= cfg::TEMP_LIGHTS_ON_C) {
      thermalPause_ = false;
      Serial.println("COOLED: grow lights can resume");
    }
  }
  bool ok() const { return ok_; }
  bool thermalPause() const { return thermalPause_; }
  float temperatureC() const { return temperatureC_; }
  float humidity() const { return humidityPct_; }
private:
  Adafruit_AHTX0 sensor_;
  bool connected_ = false;
  bool ok_ = false;
  bool thermalPause_ = false;
  float temperatureC_ = NAN;
  float humidityPct_ = NAN;
  uint32_t lastRetryMs_ = 0U;
  uint32_t lastReadMs_ = 0U;
  uint32_t pauseStartMs_ = 0U;
};
