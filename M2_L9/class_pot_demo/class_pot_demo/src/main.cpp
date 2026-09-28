#include <Arduino.h>

/*
 * Wiring:
 *   potentiometer middle pin -> GPIO 4
 *   one outer pin            -> 3V3 (through a 3.3k resistor if fitted)
 *   other outer pin          -> GND
 *
 * Serial monitor: 115200
*/

static const uint8_t POT_PIN = 4U;              // ADC1 channel on GPIO 4


void setup(void) {
    Serial.begin(115200);
    delay(3000U);  // native USB CDC needs a moment before the first print

    analogSetPinAttenuation(POT_PIN, ADC_11db);

    Serial.println();
    Serial.println("Lesson 9 Pot");
}

void loop(void) {
  uint16_t adc_raw_val = (uint16_t)analogRead(POT_PIN);
  Serial.printf("adc_raw_val is  %d  in hex is %6X\n", adc_raw_val, adc_raw_val);
  delay(1000U); 
}
