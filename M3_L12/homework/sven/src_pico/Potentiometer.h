/**
 * Wraps potentiometer access
 */

#include <Arduino.h>

#include <stdint.h>

class Potentiometer {
public:
    Potentiometer();
    void begin(void);
    uint16_t read_potentiometer(void);
    uint16_t read_potentiometer_10bit(void);
};
