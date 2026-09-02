// Lesson 02 - First Firmware. Instructor demo.
//
// Everything from today in one program:
//   - blink the blue status LED (GPIO 8, active low)
//   - report state over Serial
//   - read the BOOT button (GPIO 9, active low)
//
// Behaviour: the LED blinks slowly. While the BOOT button is held
// down, it blinks fast. Serial prints the current mode on every
// toggle.
//
// GPIO 9 is a strapping pin: held LOW during reset it puts the chip
// into the serial bootloader. After boot it is just a button.

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
    digitalWrite(LED_PIN, HIGH);  // start with the LED off
    Serial.println("lesson-02 demo: hold BOOT for fast blink");
}

void loop() {
    // Button pressed reads LOW: the pin is pulled up inside the chip
    // and the button connects it to ground.
    const bool pressed = (digitalRead(BUTTON_PIN) == LOW);
    const uint32_t interval = pressed ? FAST_MS : SLOW_MS;

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    Serial.println(pressed ? "on  (fast)" : "on  (slow)");
    delay(interval);

    digitalWrite(LED_PIN, HIGH);  // OFF
    Serial.println(pressed ? "off (fast)" : "off (slow)");
    delay(interval);
}
