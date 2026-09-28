/**
 * Wraps potentiometer access
 */

#include <Arduino.h>

#include <stdint.h>

class Potentiometer {
public:
    Potentiometer();
    void begin(void);
    uint16_t readPotentiometer(void);
    uint16_t readPotentiometerAsRing(void);
};
