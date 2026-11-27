// The bike battery's voltage, through a divider into the C3's ADC.
//
//   battery + --[ 100k ]--+--[ 10k ]-- GND
//                         |
//                       GPIO 3 (ADC), 100 nF from here to GND
//
// The pin sees 10 / (100 + 10) = 1/11 of the battery: 14.8 V -> 1.35 V,
// well inside the ADC's range even with a spike to 25 V (2.3 V).
#pragma once

#include <Arduino.h>
#include <algorithm>

class Battery {
 public:
  // gain and offset: the two-point calibration,
  // real = gain * measured + offset. 1.0 and 0 = uncalibrated.
  Battery(uint8_t pin, float gain, float offsetMv) : pin_(pin), gain_(gain), offsetMv_(offsetMv) {}

  void begin() { analogSetPinAttenuation(pin_, ADC_11db); }  // range 0..~2.5 V

  // Returns the battery voltage in mV (0 if nothing is connected).
  uint16_t readMv() {
    // analogReadMilliVolts() already uses the chip's factory ADC calibration
    // (stored in its eFuses) to turn the raw 0..4095 into millivolts at the
    // pin. The C3's ADC is noisy, and its first readings after a wake are
    // often off. So: throw the first few away, take 64 spread over ~6 ms,
    // sort them, and average only the middle half (a "trimmed mean"):
    // single spikes at either end can't pull the result.
    for (uint8_t i = 0U; i < DISCARD; i++) analogReadMilliVolts(pin_);
    uint16_t v[SAMPLES];
    for (uint8_t i = 0U; i < SAMPLES; i++) {
      v[i] = static_cast<uint16_t>(analogReadMilliVolts(pin_));
      delayMicroseconds(SAMPLE_GAP_US);
    }
    std::sort(v, v + SAMPLES);
    uint32_t sum = 0U;
    for (uint8_t i = SAMPLES / 4U; i < SAMPLES * 3U / 4U; i++) sum += v[i];
    float pinMv = sum / static_cast<float>(SAMPLES / 2U);

    float battMv = pinMv * (R_TOP + R_BOTTOM) / R_BOTTOM;  // undo the divider: x11
    battMv = gain_ * battMv + offsetMv_;
    if (battMv < NOISE_MV) battMv = 0.0f;  // nothing connected: just noise
    Serial.printf("battery: pin %.0f mV -> %.0f mV\n", pinMv, battMv);
    return static_cast<uint16_t>(lroundf(battMv));
  }

 private:
  static constexpr float R_TOP = 100000.0f;
  static constexpr float R_BOTTOM = 10000.0f;
  static constexpr uint8_t DISCARD = 4U;
  static constexpr uint8_t SAMPLES = 64U;
  static constexpr uint32_t SAMPLE_GAP_US = 100U;
  static constexpr float NOISE_MV = 500.0f;

  uint8_t pin_;
  float gain_;
  float offsetMv_;
};
