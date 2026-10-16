#include "Aht10.h"

void Aht10::begin() {
  Wire.begin();  // Pico defaults: SDA GP4, SCL GP5
  m_state = State::INIT_WRITE;
  m_state_ms = millis();
  m_temp_c100 = 0;
  m_rh_pct100 = 0;
}

void Aht10::start() {
  if (m_state != State::IDLE) {
    return;  // initialization or a measurement is already in progress
  }
  if (writeCommand(0xACU, 0x33U, 0x00U)) {
    m_state = State::MEASURING;
    m_state_ms = millis();
  } else {
    // The sensor stopped answering mid-way: retry the init in 2 s
    m_state = State::RETRY_LATER;
    m_state_ms = millis();
  }
}

Aht10Status Aht10::poll() {
  const uint32_t now = millis();

  switch (m_state) {
  case State::INIT_WRITE:
    if (writeCommand(0xE1U, 0x08U, 0x00U)) {
      m_state = State::INIT_WAIT;
      m_state_ms = now;
      return Aht10Status::BUSY;
    }
    m_state = State::RETRY_LATER;
    m_state_ms = now;
    return Aht10Status::NO_SENSOR;

  case State::INIT_WAIT:
    if (now - m_state_ms < INIT_SETTLE_MS) {
      return Aht10Status::BUSY;
    }
    {
      uint8_t status = 0U;
      const int got = Wire.requestFrom(ADDR, 1U);
      if (got < 1) {
        m_state = State::RETRY_LATER;
        m_state_ms = now;
        return Aht10Status::NO_SENSOR;
      }
      status = (uint8_t)Wire.read();
      if ((status & 0x08U) != 0U) {  // bit 3: calibrated
        m_state = State::IDLE;
        return Aht10Status::BUSY;
      }
      // Not calibrated: soft reset, then try again later
      writeCommand(0xBAU, 0x00U, 0x00U);
      m_state = State::RETRY_LATER;
      m_state_ms = now;
      return Aht10Status::NO_SENSOR;
    }

  case State::IDLE:
    return Aht10Status::BUSY;

  case State::MEASURING:
    if (now - m_state_ms < MEASURE_WAIT_MS) {
      return Aht10Status::BUSY;
    }
    {
      uint8_t bytes[5];
      uint8_t status = 0U;
      if (!readResult(&status, bytes)) {
        m_state = State::RETRY_LATER;
        m_state_ms = now;
        return Aht10Status::NO_SENSOR;
      }
      if ((status & 0x80U) != 0U) {  // bit 7: still busy
        m_state_ms = now;  // wait another 80 ms
        return Aht10Status::BUSY;
      }
      unpack(bytes[0U], bytes[1U], bytes[2U], bytes[3U], bytes[4U]);
      m_state = State::IDLE;
      return Aht10Status::READY;
    }

  case State::RETRY_LATER:
    if (now - m_state_ms >= RETRY_PERIOD_MS) {
      m_state = State::INIT_WRITE;
    }
    return Aht10Status::NO_SENSOR;
  }
  return Aht10Status::NO_SENSOR;
}

int16_t Aht10::temperatureHundredths() const {
  return m_temp_c100;
}

uint16_t Aht10::humidityHundredths() const {
  return m_rh_pct100;
}

// One I2C write transaction: 3 command bytes.
// true when the whole transaction was ACKed (the address is present)
bool Aht10::writeCommand(uint8_t b1, uint8_t b2, uint8_t b3) {
  Wire.beginTransmission(ADDR);
  Wire.write(b1);
  Wire.write(b2);
  Wire.write(b3);
  return Wire.endTransmission() == 0U;
}

// One I2C read transaction: the 6-byte measurement result.
// bytes[0] is the status byte, bytes[1..4] are b1..b5
bool Aht10::readResult(uint8_t *status, uint8_t *bytes) {
  const int got = Wire.requestFrom(ADDR, 6U);
  if (got < 6) {
    return false;
  }
  *status = (uint8_t)Wire.read();
  for (uint8_t i = 0U; i < 5U; i++) {
    bytes[i] = (uint8_t)Wire.read();
  }
  return true;
}

void Aht10::unpack(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5) {
  // Humidity raw: 20 bits = b1, b2, high nibble of b3
  const uint32_t rh_raw =
      ((uint32_t)b1 << 12) | ((uint32_t)b2 << 4) | (b3 >> 4);
  // RH % = raw / 2^20 * 100; in hundredths: raw * 10000 / 2^20.
  // raw * 10000 does not fit in 32 bits, so go through 64.
  m_rh_pct100 = (uint16_t)(((uint64_t)rh_raw * 10000U) >> 20U);

  // Temperature raw: 20 bits = low nibble of b3, b4, b5
  const uint32_t t_raw =
      ((uint32_t)(b3 & 0x0FU) << 16) | ((uint32_t)b4 << 8) | (uint32_t)b5;
  // T degC = raw / 2^20 * 200 - 50; in hundredths: raw * 20000 / 2^20 - 5000
  m_temp_c100 =
      (int16_t)(((int32_t)(((uint64_t)t_raw * 20000U) >> 20U)) - 5000);
}
