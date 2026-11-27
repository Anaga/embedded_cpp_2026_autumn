// Shared by the node and the gateway: the pins and the radio settings.
//
// The radio settings MUST be identical on both ends. A receiver on SF9
// cannot hear a sender on SF10, and a different sync word makes the
// receiver ignore the packet. Keeping them in one file makes a mismatch
// impossible.
#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// ---- Pins (checked with the lora-check test) -----------------------------
#if defined(BOARD_S3)
constexpr int8_t PIN_SCK = 12;   // grey
constexpr int8_t PIN_MISO = 13;  // purple
constexpr int8_t PIN_MOSI = 11;  // blue
constexpr int8_t PIN_NSS = 10;   // green
constexpr int8_t PIN_RST = 9;    // orange
constexpr int8_t PIN_DIO0 = 14;  // yellow: "packet received" (used by the real gateway later)
#elif defined(BOARD_C3)
constexpr int8_t PIN_SCK = 4;
constexpr int8_t PIN_MISO = 5;
constexpr int8_t PIN_MOSI = 6;
constexpr int8_t PIN_NSS = 7;
constexpr int8_t PIN_RST = 10;
constexpr int8_t PIN_DIO0 = -1;  // not connected on the node
#else
#error "Define BOARD_S3 (gateway) or BOARD_C3 (node) in platformio.ini"
#endif

// ---- Radio settings (EU rules for 433 MHz) -----------------------------
constexpr uint32_t FREQUENCY_HZ = 434000000UL;  // inside 433.05-434.79 MHz
constexpr uint8_t TX_POWER_DBM = 10U;          // the EU limit here. The module can do 20: don't.
constexpr uint8_t SPREADING_FACTOR = 9U;       // raise to 10, 11, 12 if packets get lost
constexpr uint32_t BANDWIDTH_HZ = 125000UL;
constexpr uint8_t CODING_RATE_DENOM = 5U;      // 4/5: 1 extra bit per 4 for error correction
constexpr uint8_t SYNC_WORD = 0x12U;           // "private network": ignore LoRaWAN traffic (0x34)

// Starts SPI and the radio with the settings above. Returns false if the
// chip doesn't answer (the same 0x12 check as lora-check).
inline bool radioBegin() {
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_NSS);
  LoRa.setPins(PIN_NSS, PIN_RST, PIN_DIO0);
  if (!LoRa.begin(FREQUENCY_HZ)) return false;

  // The Ra-01's antenna is wired to the SX1278's PA_BOOST output, which is
  // the library's default. (The other output, RFO, is not connected.)
  LoRa.setTxPower(TX_POWER_DBM, PA_OUTPUT_PA_BOOST_PIN);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH_HZ);
  LoRa.setCodingRate4(CODING_RATE_DENOM);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.enableCrc();  // the receiver drops packets damaged on the way
  return true;
}

inline void printRadioSettings() {
  Serial.printf("radio: %.1f MHz, %u dBm, SF%u, %lu kHz, CR 4/%u, sync 0x%02X, CRC on\n", FREQUENCY_HZ / 1e6,
                static_cast<unsigned>(TX_POWER_DBM), static_cast<unsigned>(SPREADING_FACTOR),
                static_cast<unsigned long>(BANDWIDTH_HZ / 1000U), static_cast<unsigned>(CODING_RATE_DENOM),
                static_cast<unsigned>(SYNC_WORD));
}
