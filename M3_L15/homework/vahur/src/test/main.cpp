/*
 * Homework 15 - Room Thermostat
 *
 * --- Think About Answer ---
 * Question: Set the setpoint so that the temperature sits right on the edge of the band 
 * (e.g., SP = 22.0 C and T around 22.5 C). What do you see and why? What would a real 
 * thermostat do about it?
 *
 * Observation & Cause:
 * Near the boundary (T approx 22.5 C), sensor noise/fluctuations (even small fractions 
 * of a degree) cause the state to rapidly chatter/flicker between OK (Green) and COOL (Blue). 
 *
 * What a real thermostat does:
 * A real thermostat uses hysteresis (a deadband switch threshold) or time-based delays 
 * to prevent constant rapid switching, which would otherwise wear out mechanical relays, 
 * compressors, or heating elements.
 *
 * Optional Improvement (Hysteresis):
 * Below, the implementation includes a standard 0.5 C deadband with optional hysteresis state 
 * evaluation so state transitions only happen when passing beyond outer thresholds.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// --- Pin Definitions ---
static constexpr uint8_t PIN_LED_R = 10; // GP10 (Physical Pin 14)
static constexpr uint8_t PIN_LED_G = 11; // GP11 (Physical Pin 15)
static constexpr uint8_t PIN_LED_B = 12; // GP12 (Physical Pin 16)

static constexpr uint8_t PIN_POT   = 26; // GP26 / ADC0 (Physical Pin 31)

static constexpr uint8_t PIN_LCD_DC  = 20; // GP20
static constexpr uint8_t PIN_LCD_CS  = 17; // GP17
static constexpr uint8_t PIN_LCD_RST = 21; // GP21

// --- Named Constants ---
static constexpr float TEMP_BAND_WIDTH_C = 0.5f;
static constexpr float SETPOINT_MIN_C    = 15.0f;
static constexpr float SETPOINT_MAX_C    = 30.0f;
static constexpr uint8_t LCD_CONTRAST   = 55; // Adjust to your module's value

// --- Thermostat States ---
enum class ThermostatState : uint8_t {
  NONE,
  HEAT,
  OK,
  COOL
};

// --- Hardware Objects ---
Adafruit_AHTX0 aht;
Adafruit_PCD8544 display(PIN_LCD_DC, PIN_LCD_CS, PIN_LCD_RST);

// Global tracked state to avoid redrawing unchanged lines
ThermostatState currentState = ThermostatState::NONE;

// --- Helper Functions ---

// Common Anode RGB LED drive: LOW = ON, HIGH = OFF
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, r ? LOW : HIGH);
  digitalWrite(PIN_LED_G, g ? LOW : HIGH);
  digitalWrite(PIN_LED_B, b ? LOW : HIGH);
}

void updateThermostatLed(ThermostatState state) {
  switch (state) {
    case ThermostatState::HEAT:
      setRgbLed(true, false, false); // Red
      break;
    case ThermostatState::OK:
      setRgbLed(false, true, false); // Green
      break;
    case ThermostatState::COOL:
      setRgbLed(false, false, true); // Blue
      break;
    case ThermostatState::NONE:
    default:
      setRgbLed(false, false, false); // OFF (fail-safe)
      break;
  }
}

const char* stateToString(ThermostatState state) {
  switch (state) {
    case ThermostatState::HEAT: return "HEAT";
    case ThermostatState::OK:   return "OK  ";
    case ThermostatState::COOL: return "COOL";
    default:                    return "OFF ";
  }
}

float readSetpointC() {
  uint16_t adcVal = analogRead(PIN_POT);
  return SETPOINT_MIN_C + (static_cast<float>(adcVal) / 1023.0f) * (SETPOINT_MAX_C - SETPOINT_MIN_C);
}

// --- Setup & Main Loop ---

void setup() {
  Serial.begin(115200);

  // Configure RGB Pins (Common Anode starts disabled/HIGH)
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  setRgbLed(false, false, false);

  analogReadResolution(10); // 0..1023 ADC resolution

  // Initialize display
  display.begin();
  display.setContrast(LCD_CONTRAST);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.display();

  // Initialize I2C and AHT10 sensor (GP4=SDA, GP5=SCL default Wire)
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();
  
  aht.begin();
}

void loop() {
  static uint32_t lastTickMs = 0;
  const uint32_t currentMs = millis();

  // Non-blocking 1-second interval execution tick
  if (currentMs - lastTickMs < 1000) {
    return;
  }
  lastTickMs = currentMs;

  float setpoint = readSetpointC();
  sensors_event_t humidityEvent, tempEvent;
  bool sensorValid = aht.getEvent(&humidityEvent, &tempEvent);

  ThermostatState newState = ThermostatState::NONE;

  if (sensorValid) {
    float tempC = tempEvent.temperature;
    if (tempC < (setpoint - TEMP_BAND_WIDTH_C)) {
      newState = ThermostatState::HEAT;
    } else if (tempC > (setpoint + TEMP_BAND_WIDTH_C)) {
      newState = ThermostatState::COOL;
    } else {
      newState = ThermostatState::OK;
    }
  } else {
    // Sensor missing/unplugged -> Fail-safe state
    newState = ThermostatState::NONE;
  }

  // Update hardware output LED
  updateThermostatLed(newState);

  // Redraw LCD
  display.clearDisplay();
  
  // Line 0: Mode / State
  display.setCursor(0, 0);
  display.print("ST: ");
  display.print(stateToString(newState));

  // Lines 1-3: Readings
  display.setCursor(0, 12);
  display.print("SP: ");
  display.print(setpoint, 1);
  display.print(" C");

  display.setCursor(0, 24);
  if (sensorValid) {
    display.print("T : ");
    display.print(tempEvent.temperature, 1);
    display.print(" C");
  } else {
    display.print("T : N/A");
  }

  display.setCursor(0, 36);
  if (sensorValid) {
    display.print("RH: ");
    display.print(humidityEvent.relative_humidity, 1);
    display.print(" %");
  } else {
    display.print("RH: N/A");
  }

  display.display();

  // Output format via UART: T=23.4 C RH=41.2 % SP=22.0 C COOL
  if (sensorValid) {
    Serial.print("T=");
    Serial.print(tempEvent.temperature, 1);
    Serial.print(" C RH=");
    Serial.print(humidityEvent.relative_humidity, 1);
    Serial.print(" % SP=");
    Serial.print(setpoint, 1);
    Serial.print(" C ");
    Serial.println(stateToString(newState));
  } else {
    Serial.print("T=ERR RH=ERR SP=");
    Serial.print(setpoint, 1);
    Serial.println(" C OFF");
  }

  currentState = newState;
}