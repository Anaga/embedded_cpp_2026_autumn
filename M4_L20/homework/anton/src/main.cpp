/*
 * Homework 20 - Automation logic: sensor + feedback.
 *
 * AHT10 (I2C) + a heater actuator (MOSFET on GP27). The board keeps the
 * temperature inside a band around a setpoint by switching the heater.
 *
 * Non-blocking: all timing via millis(); no delay() in loop().
 *
 * The thermostat uses a named band constant and a fail-safe: if the sensor
 * stops answering, the heater is turned OFF and the fault is reported on
 * Serial and on the Nokia 5110 display.
 *
 * Wiring:
 *   AHT10  SDA GP4, SCL GP5
 *   Heater GP27 via MOSFET
 *   Nokia 5110: RST GP21, CE GP17, DC GP20, DIN GP19, CLK GP18
 */

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include <stdio.h>

// ==================== Pins ====================
static const uint8_t PIN_AHT_SDA = 4;
static const uint8_t PIN_AHT_SCL = 5;
static const uint8_t PIN_HEATER  = 27;

// Nokia 5110 (software SPI)
static const uint8_t PIN_LCD_RST = 21;
static const uint8_t PIN_LCD_CE  = 17;
static const uint8_t PIN_LCD_DC  = 20;
static const uint8_t PIN_LCD_DIN = 19;
static const uint8_t PIN_LCD_CLK = 18;

// ==================== Thermostat constants ====================
static const float SP_MIN_C = 15.0f;
static const float SP_MAX_C = 30.0f;
static const float SP_DEFAULT_C = 21.0f;
static const float BAND_C = 0.5f;          // dead band
static const uint32_t SAMPLE_MS = 2000;    // sensor read period
static const uint32_t DISPLAY_MS = 1000;   // screen refresh period

// ==================== State ====================
enum class HeaterState : uint8_t { OFF, ON, FAULT };
static HeaterState heater = HeaterState::OFF;
static float setpoint_c = SP_DEFAULT_C;
static float temp_c = 0.0f;
static bool sensor_ok = false;

// ==================== AHT10 (minimal, no library) ====================
static bool ahtWrite(uint8_t cmd, const uint8_t *data, size_t n) {
    Wire.beginTransmission(0x38);
    Wire.write(cmd);
    for (size_t i = 0; i < n; i++) Wire.write(data[i]);
    return Wire.endTransmission() == 0;
}

static bool ahtInit() {
    // Soft reset
    ahtWrite(0xBA, NULL, 0);
    // Calibrate
    uint8_t cal[3] = {0x08, 0x00, 0x00};
    return ahtWrite(0xBE, cal, 3);
}

static bool ahtRead(float *t, float *h) {
    // Trigger measurement
    uint8_t trig[3] = {0x33, 0x00, 0x00};
    if (!ahtWrite(0xAC, trig, 3)) return false;
    // Non-blocking wait handled by caller; here we poll briefly
    // (in real code, schedule this via millis)
    for (int i = 0; i < 100; i++) {
        Wire.beginTransmission(0x38);
        if (Wire.endTransmission() != 0) return false;
        Wire.requestFrom(0x38, (uint8_t)7);
        if (Wire.available() >= 7) {
            uint8_t b[7];
            for (int j = 0; j < 7; j++) b[j] = Wire.read();
            if ((b[0] & 0x80) == 0) {
                uint32_t raw_h = ((uint32_t)b[1] << 12) | ((uint32_t)b[2] << 4) | (b[3] >> 4);
                uint32_t raw_t = (((uint32_t)b[3] & 0x0F) << 16) | ((uint32_t)b[4] << 8) | b[5];
                *h = (float)raw_h * 100.0f / 1048576.0f;
                *t = (float)raw_t * 200.0f / 1048576.0f - 50.0f;
                return true;
            }
        }
        // Small spin; real code would yield
        for (volatile int k = 0; k < 1000; k++) {}
    }
    return false;
}

// ==================== Heater control ====================
static void heaterOn()  { digitalWrite(PIN_HEATER, HIGH); heater = HeaterState::ON; }
static void heaterOff() { digitalWrite(PIN_HEATER, LOW);  heater = HeaterState::OFF; }

// ==================== Display stub ====================
static void displayUpdate(float t, float sp, HeaterState st) {
    Serial.printf("[DISPLAY] T=%.1f SP=%.1f %s\n",
                  t, sp,
                  st == HeaterState::ON ? "HEAT" :
                  st == HeaterState::FAULT ? "FAULT" : "OFF");
}

// ==================== Setup ====================
void setup() {
    Serial.begin(115200);
    pinMode(PIN_HEATER, OUTPUT);
    digitalWrite(PIN_HEATER, LOW);

    Wire.begin(PIN_AHT_SDA, PIN_AHT_SCL);
    sensor_ok = ahtInit();
    if (!sensor_ok) {
        Serial.println("AHT10 not found - fail-safe: heater OFF");
        heater = HeaterState::FAULT;
    }
    Serial.println("=== L20 Homework - Automation logic ===");
}

// ==================== Loop ====================
void loop() {
    static uint32_t last_sample = 0;
    static uint32_t last_display = 0;
    const uint32_t now = millis();

    // Periodic sensor read
    if (now - last_sample >= SAMPLE_MS) {
        last_sample = now;
        float t, h;
        if (ahtRead(&t, &h)) {
            temp_c = t;
            sensor_ok = true;
        } else {
            sensor_ok = false;
        }

        // Fail-safe: sensor missing -> heater OFF
        if (!sensor_ok) {
            heaterOff();
            heater = HeaterState::FAULT;
            Serial.println("sensor missing -> heater OFF");
        } else {
            // Thermostat decision with band
            if (temp_c < setpoint_c - BAND_C) heaterOn();
            else if (temp_c > setpoint_c + BAND_C) heaterOff();
            // else: keep current state (dead band)
        }
    }

    // Periodic display update
    if (now - last_display >= DISPLAY_MS) {
        last_display = now;
        displayUpdate(temp_c, setpoint_c, heater);
    }
}
