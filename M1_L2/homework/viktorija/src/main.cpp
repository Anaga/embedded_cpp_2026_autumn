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

static const uint32_t SLOW_MS = 200U;
static const uint32_t SLOW_MS_LONG = 600U;

void setup() {
    Serial.begin(115200U);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(SLOW_MS);
    digitalWrite(LED_PIN, HIGH);  // start with the LED off
    delay(SLOW_MS);
    digitalWrite(LED_PIN, LOW);  // set the LED on
    delay(SLOW_MS);
    digitalWrite(LED_PIN, HIGH);  // put the LED off
    delay(SLOW_MS);
    Serial.println("lesson-02 demo: simple blink");
}

void loop() {

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    Serial.print("S");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("S");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println("S");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println("S");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS); 

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    Serial.print("O");
    delay(SLOW_MS_LONG);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println("O");
    delay(SLOW_MS_LONG);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println("O");
    delay(SLOW_MS_LONG);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    Serial.println("S");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println("S");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println(" ");
    delay(SLOW_MS); 

    digitalWrite(LED_PIN, LOW);   // ON (active low)
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    // Serial.println("S");
    delay(SLOW_MS);

    digitalWrite(LED_PIN, HIGH);  // OFF
    if (digitalRead(BUTTON_PIN) == LOW) {
        Serial.println(" ");
        Serial.println("Button pressed");
        return;
    }
    Serial.println(" ");
    delay(SLOW_MS); 

    delay(SLOW_MS_LONG);
    Serial.println(".");

}
