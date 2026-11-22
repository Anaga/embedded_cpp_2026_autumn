// Pins and LoRa settings shared by node and gateway. Both ends must match.
#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#if defined(BOARD_S3)
constexpr int PIN_SCK = 12;
constexpr int PIN_MISO = 13;
constexpr int PIN_MOSI = 11;
constexpr int PIN_NSS = 10;
constexpr int PIN_RST = 9;
constexpr int PIN_DIO0 = 14;
#elif defined(BOARD_C3)
constexpr int PIN_SCK = 4;
constexpr int PIN_MISO = 5;
constexpr int PIN_MOSI = 6;
constexpr int PIN_NSS = 7;
constexpr int PIN_RST = 10;
constexpr int PIN_DIO0 = -1;  // not connected
#else
#error "Define BOARD_S3 (gateway) or BOARD_C3 (node) in platformio.ini"
#endif

constexpr long FREQUENCY_HZ = 434000000;  // inside the 433.05-434.79 MHz band
constexpr int TX_POWER_DBM = 10;          // EU limit
constexpr int SPREADING_FACTOR = 9;
constexpr long BANDWIDTH_HZ = 125000;
constexpr int CODING_RATE_DENOM = 5;
constexpr int SYNC_WORD = 0x12;           // private network, not LoRaWAN (0x34)

inline bool radioBegin() {
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_NSS);
  LoRa.setPins(PIN_NSS, PIN_RST, PIN_DIO0);
  if (!LoRa.begin(FREQUENCY_HZ)) return false;

  LoRa.setTxPower(TX_POWER_DBM, PA_OUTPUT_PA_BOOST_PIN);  // Ra-01 antenna is on PA_BOOST
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH_HZ);
  LoRa.setCodingRate4(CODING_RATE_DENOM);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.enableCrc();
  return true;
}

inline void printRadioSettings() {
  Serial.printf("radio: %.1f MHz, %d dBm, SF%d, %ld kHz, CR 4/%d, sync 0x%02X, CRC on\n",
                FREQUENCY_HZ / 1e6, TX_POWER_DBM, SPREADING_FACTOR, BANDWIDTH_HZ / 1000,
                CODING_RATE_DENOM, SYNC_WORD);
}
