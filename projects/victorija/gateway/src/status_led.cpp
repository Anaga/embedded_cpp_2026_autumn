#include "status_led.h"

#include "config.h"
#include "shared.h"

// Colours as 0xRRGGBB. The steady ones are dim on purpose: the WS2812 is
// very bright, and it sits on a desk next to you.
constexpr uint32_t OFF = 0x000000U;
constexpr uint32_t GREEN = 0x000300U;
constexpr uint32_t RED = 0x0C0000U;
constexpr uint32_t BLUE = 0x00000CU;
constexpr uint32_t YELLOW = 0x080500U;
constexpr uint32_t CYAN = 0x00060AU;
constexpr uint32_t ERROR_RED = static_cast<uint32_t>(LED_FLASH_LEVEL) << 16;

void StatusLed::begin() { write(OFF); }

void StatusLed::showError() { write(ERROR_RED); }

void StatusLed::write(uint32_t rgb) {
  neopixelWrite(PIN_RGB, (rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
  shown_ = rgb;
  lastWrite_ = millis();
}

void StatusLed::flash(Flash f) {
  constexpr uint8_t L = LED_FLASH_LEVEL;
  switch (f) {
    case Flash::MOVEMENT: neopixelWrite(PIN_RGB, 0U, 0U, L); break;
    case Flash::WARN: neopixelWrite(PIN_RGB, L, L * 2U / 3U, 0U); break;  // a packet was missed, or a sensor fault
    case Flash::GOOD: neopixelWrite(PIN_RGB, 0U, L, 0U); break;
  }
  flashUntil_ = millis() + LED_FLASH_MS;
  if (flashUntil_ == 0U) flashUntil_ = 1U;  // 0 means "no flash"
  shown_ = UINT32_MAX;                       // redraw the state colour after it
}

uint32_t StatusLed::stateColour(uint32_t now) const {
  bool firstHalf = (now / LED_BLINK_MS) % 2U == 0U;
  bool fault = !shared::online || alarms_.anySystemFault();
  if (alarms_.on(Alarm::TAMPER)) return firstHalf ? RED : BLUE;
  if (alarms_.on(Alarm::BATT_CRIT)) return firstHalf ? RED : OFF;
  if (alarms_.on(Alarm::BATT_LOW)) return RED;
  if (alarms_.on(Alarm::DAMP)) return CYAN;  // steady light blue: water
  if (fault) return firstHalf ? YELLOW : OFF;
  return GREEN;
}

void StatusLed::update(uint32_t now) {
  if (flashUntil_ != 0U) {
    if (static_cast<int32_t>(now - flashUntil_) < 0) return;  // the flash is still showing
    flashUntil_ = 0U;  // no "off" write first: the state colour simply replaces it
  }
  uint32_t colour = stateColour(now);
  if (colour != shown_ || now - lastWrite_ >= LED_REFRESH_MS) write(colour);
}
