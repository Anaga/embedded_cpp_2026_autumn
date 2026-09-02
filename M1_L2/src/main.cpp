// Lesson 02 - First Firmware. Most simpele demo.
//
// Everything from today in one program:
//   - blink the blue status LED (GPIO 8, active low)
//   - report state over Serial
// Behaviour: the LED blinks slowly. 

#include <Arduino.h>

// Board pinout - stays the same in every lesson. See README.md.
static const uint8_t LED_PIN = 8U;     // blue status LED, LOW = ON
static const uint8_t BUTTON_PIN = 9U;  // BOOT button, LOW = pressed

static const uint32_t SLOW_MS = 500U;
static const uint32_t FAST_MS = 100U;

void setup() {
    Serial.begin(115200U);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.println("lesson-02 demo: simple blink");
}

void loop() {    
    if (digitalRead(BUTTON_PIN)==LOW)
    {
        digitalWrite(LED_PIN,LOW);
        delay(FAST_MS);
        digitalWrite(LED_PIN,HIGH);
        delay(FAST_MS);
    }
    else
    {
        digitalWrite(LED_PIN,LOW);
        delay(SLOW_MS);
        digitalWrite(LED_PIN,HIGH);
        delay(SLOW_MS);
    }
}
