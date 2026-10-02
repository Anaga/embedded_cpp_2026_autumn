#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <stdint.h>

class Button {
public:
 explicit Button(uint8_t pin);
 void begin();
 bool update();
 bool wasPressed();
 bool isDown() const;

private:
 uint8_t _pin;
 bool _state;
 bool _last_raw_state;
 bool _was_pressed_flag;
 uint32_t _last_debounce_time;
 static const uint32_t DEBOUNCE_DELAY_MS = 30U;
};

#endif // BUTTON_H