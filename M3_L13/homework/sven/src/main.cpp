// AHT10: VIN -> 3V3, GND -> GND, SDA -> GP4, SCL -> GP5.

#include <Arduino.h>
#include <pico/time.h>

#include "Aht10.h"

namespace {

Aht10 aht;
repeating_timer_t readTimer;
volatile bool readDue = false;
bool missingReported = false;

bool onReadTimer(repeating_timer_t *) {
    readDue = true;  // Interrupt: signal only. I2C and Serial stay in loop().
    return true;
}

}  // namespace

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    aht.begin();

    // Negative interval: 2000 ms between callback starts. First read in 2 s.
    if (!add_repeating_timer_ms(-2000, onReadTimer, nullptr, &readTimer)) {
        Serial.println("timer unavailable");
    }
}

void loop() {
    // Copy and clear together so an interrupt cannot be lost between them.
    noInterrupts();
    const bool due = readDue;
    readDue = false;
    interrupts();

    if (due) {
        aht.start();
    }

    const Aht10::Status status = aht.poll();
    if (status == Aht10::Status::Ready) {
        missingReported = false;
        Serial.print("T = ");
        Serial.print(aht.temperature(), 2);
        Serial.print(" C   RH = ");
        Serial.print(aht.humidity(), 2);
        Serial.println(" %");
    } else if (status == Aht10::Status::Missing && !missingReported) {
        Serial.println("sensor missing");
        missingReported = true;
    }

    // Heartbeat keeps blinking while the sensor measures or is unplugged.
    digitalWrite(LED_BUILTIN, (millis() / 100U) % 2U);
}
