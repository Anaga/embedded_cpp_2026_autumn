#include "Button.h"

Button::Button(uint8_t pin)
 : _pin(pin), _state(HIGH), _last_raw_state(HIGH),
 _was_pressed_flag(false), _last_debounce_time(0) {}

void Button::begin() {
 pinMode(_pin, INPUT_PULLUP);
 _state = digitalRead(_pin);
 _last_raw_state = _state;
}

bool Button::update() {
 bool raw_read = digitalRead(_pin);

 if (raw_read != _last_raw_state) {
 _last_debounce_time = millis();
 _last_raw_state = raw_read;
 }

 if ((millis() - _last_debounce_time) > DEBOUNCE_DELAY_MS) {
 if (raw_read != _state) {
 _state = raw_read;
 if (_state == LOW) { // Press event (active LOW)
 _was_pressed_flag = true;
 }
 }
 }

 return _state == LOW;
}

bool Button::wasPressed() {
 if (_was_pressed_flag) {
 _was_pressed_flag = false;
 return true;
 }
 return false;
}

bool Button::isDown() const {
 return _state == LOW;
}