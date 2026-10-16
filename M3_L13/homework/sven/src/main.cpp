// Lesson 13 demo - I2C scanner and the AHT10 through a library
//
// What it shows:
//   1. An I2C scan at start-up, and again whenever you send 's'.
//   2. A temperature and humidity reading every 2 s, through the
//      Adafruit AHTX0 library.
//   3. How long getEvent() blocks, measured with micros().
//   4. The on-board LED blinks without delay(), so you can see the
//      moments when loop() is not turning.
//
// Wiring (AHT10 -> Pico):
//   VIN -> 3V3 OUT (pin 36)   never VBUS
//   GND -> GND
//   SDA -> GP4 (pin 6)
//   SCL -> GP5 (pin 7)

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <stdint.h>

namespace {

// I2C scan range: 7-bit addresses 0x01 .. 0x7E
constexpr uint8_t SCAN_FIRST_ADDR = 0x01U;
constexpr uint8_t SCAN_END_ADDR = 0x7FU;  // exclusive
constexpr uint8_t AHT10_ADDR = 0x38U;     // expected in the scan

constexpr uint32_t SERIAL_WAIT_MS = 3000U;   // wait for the monitor at start-up
constexpr uint32_t READ_PERIOD_MS = 2000U;   // one reading every 2 s
constexpr uint32_t BLINK_HALF_MS = 100U;     // heartbeat LED half-period
constexpr int RESCAN_KEY = 's';

Adafruit_AHTX0 aht;
bool sensorReady = false;

uint32_t lastReadMs = 0U;
uint32_t lastBlinkMs = 0U;
bool ledOn = false;

void scanBus() {
    Serial.println("I2C scan (SDA GP4, SCL GP5):");
    uint8_t found = 0U;
    bool sawAht10 = false;

    for (uint8_t addr = SCAN_FIRST_ADDR; addr < SCAN_END_ADDR; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0U) {  // 0 = the address was ACKed
            Serial.printf("  found 0x%02X\n", addr);
            found++;
            if (addr == AHT10_ADDR) {
                sawAht10 = true;
            }
        }
    }

    if (found == 0U) {
        Serial.println("  nothing found - check 3.3 V at VIN, GND, SDA/SCL, pull-ups");
    } else if (!sawAht10) {
        Serial.println("  no device at 0x38 - is this really an AHT10?");
    }
    Serial.printf("  %u device(s)\n", static_cast<unsigned>(found));
}

void startSensor() {
    sensorReady = aht.begin();  // default address 0x38, uses Wire
    if (sensorReady) {
        Serial.println("AHT10 ready");
    } else {
        Serial.println("AHT10 not ready - fix the wiring, then send 's'");
    }
}

void readSensor() {
    sensors_event_t humidity;
    sensors_event_t temperature;

    const uint32_t startUs = micros();
    aht.getEvent(&humidity, &temperature);  // waits for the sensor inside
    const uint32_t blockedUs = micros() - startUs;

    Serial.print("T = ");
    Serial.print(temperature.temperature, 2);
    Serial.print(" C   RH = ");
    Serial.print(humidity.relative_humidity, 2);
    Serial.printf(" %%   getEvent() blocked %lu us\n",
                  static_cast<unsigned long>(blockedUs));
}

void blinkHeartbeat(uint32_t now) {
    if ((now - lastBlinkMs) >= BLINK_HALF_MS) {
        lastBlinkMs = now;
        ledOn = !ledOn;
        digitalWrite(LED_BUILTIN, ledOn ? HIGH : LOW);
    }
}

void handleSerial() {
    if (Serial.available() > 0) {
        if (Serial.read() == RESCAN_KEY) {
            scanBus();
            if (!sensorReady) {
                startSensor();
            }
        }
    }
}

}  // namespace

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200U);  // USB CDC: the baud rate is ignored
    const uint32_t waitStart = millis();
    while (!Serial && ((millis() - waitStart) < SERIAL_WAIT_MS)) {
        // give the serial monitor a moment to connect
    }

    Wire.begin();
    Serial.println();
    Serial.println("Lesson 13 demo - I2C scanner and AHT10 (library)");
    scanBus();
    startSensor();
}

void loop() {
    const uint32_t now = millis();

    blinkHeartbeat(now);

    if (sensorReady && ((now - lastReadMs) >= READ_PERIOD_MS)) {
        lastReadMs = now;
        readSensor();
    }

    handleSerial();
}
