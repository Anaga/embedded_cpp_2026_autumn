#include <Arduino.h>

/*
 * Wiring:
 *   potentiometer middle pin -> GPIO 4
 *   one outer pin            -> 3V3 (through a 3.3k resistor if fitted)
 *   other outer pin          -> GND
 *
 * Serial monitor: 115200
*/

uint8_t map_function(uint16_t pot_val);
uint8_t map_pwm(uint8_t percent);

static const uint8_t POT_PIN = 4U;              // ADC1 channel on GPIO 4
static const uint8_t LED_PIN = 5U; 
static const uint8_t CHANNEL_RED = 0U;
static const uint32_t PWM_FREQ_HZ = 5000U;
static const uint8_t PWM_BITS = 8U;
static const uint8_t PWM_MAX = 255U;

void setup(void) {
    Serial.begin(115200);
    delay(3000U);  // native USB CDC needs a moment before the first print

    analogSetPinAttenuation(POT_PIN, ADC_11db);
    ledcSetup(CHANNEL_RED, PWM_FREQ_HZ, PWM_BITS);
    ledcAttachPin(LED_PIN, CHANNEL_RED);

    Serial.println();
    Serial.println("Lesson 9 Pot");
}

void loop(void) {
  uint16_t adc_raw_val = (uint16_t)analogRead(POT_PIN);
  uint8_t procents = map_function(adc_raw_val);
  uint8_t led_level = map_pwm(procents);
  Serial.printf("adc_raw_val is  %d; procent %d, PWM level: %d\n", adc_raw_val, procents, led_level);
  ledcWrite(CHANNEL_RED, led_level);
  delay(1000U); 
}

uint8_t map_function(uint16_t pot_val){
  return pot_val/35u;
}

uint8_t map_pwm(uint8_t percent){
  return (PWM_MAX - (uint8_t)(percent*2.55));
}