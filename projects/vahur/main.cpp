/*
 * PISI - ESP32-C3 Super Mini microgreens controller
 * TEST MODE: same Taimer state logic with 2-min pump intervals, 10-s watering,
 * 2-min day / 1-min night and 10-s sunrise / sunset fades.
 * Final GPIO map: RGB 5/6/7; pump 3; light 10; AHT10 SDA0/SCL1;
 * potentiometer 4; optional digital light sensor DO20.
 *
 * Needs: Arduino-ESP32 core 2.x, Adafruit AHTX0 library.
 *
 * IMPORTANT HARDWARE NOTES
 * - The JT-160 is a BRUSHLESS 6-12 V DC pump. Its ability to accept supply-PWM
 *   is NOT confirmed. Test the 1-second ramp with water and check for stalling,
 *   clicking or repeated restarts. Set PUMP_USE_PWM_RAMP=false if unreliable.
 * - MOSFET modules must accept 3.3 V, active-HIGH control signals.
 * - Connect the ESP32 to REGULATED 5 V by USB, NEVER directly to 12 V.
 * - Keep 12 V pump/LED current OFF the solderless breadboard.
 * - This is a prototype; a software timer is NOT a hardware safety cut-off.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <math.h>
#include <stdint.h>

#if !ARDUINO_USB_CDC_ON_BOOT
#error "Enable USB CDC on boot; GPIO20 is reserved for the light sensor."
#endif

#if ESP_ARDUINO_VERSION_MAJOR != 2
#error "This project uses Arduino-ESP32 core 2.x (LEDC API)."
#endif

// --------------------------- GPIO assignments -----------------------------
static const uint8_t PIN_PUMP = 3U;       // 12 V pump MOSFET PWM input
static const uint8_t PIN_LIGHT = 10U;     // 12 V grow-light MOSFET PWM input
static const uint8_t PIN_POT = 4U;        // potentiometer ADC input
static const uint8_t PIN_RGB_RED = 5U;    // common-anode RGB, active LOW
static const uint8_t PIN_RGB_GREEN = 6U;
static const uint8_t PIN_RGB_BLUE = 7U;
static const uint8_t PIN_AHT_SDA = 0U;    // AHT10 I2C data (SDA)
static const uint8_t PIN_AHT_SCL = 1U;    // AHT10 I2C clock (SCL)

// ------------------------- Schedule and timing ----------------------------
static const uint32_t TICK_MS = 20UL;
static const uint32_t WATER_INTERVAL_MS = 2UL * 60UL * 1000UL; // TEST: 2 minutes
static const uint32_t WATER_FIXED_SEC = 10UL;      // TEST: 10 seconds
static const bool WATER_ON_BOOT = false;           // safer while assembling

// Leave false for the 10-second test. Later you can re-enable the
// original 1-6 minute potentiometer control after changing the test timing.
static const bool USE_POT_FOR_WATER_DURATION = false;
static const uint32_t WATER_MIN_SEC = 60UL;
static const uint32_t WATER_MAX_SEC = 360UL;
static const uint32_t WATER_STEP_SEC = 10UL;
static const uint32_t POT_MV_LOW = 100UL;          // original resistor network
static const uint32_t POT_MV_HIGH = 2400UL;
static const bool POT_REVERSED = false;

static const bool PUMP_USE_PWM_RAMP = true;        // TEST WITH WATER FIRST
static const uint32_t PUMP_RAMP_MS = 1000UL;
// Beginning at 65% is more likely to start a brushless pump than 0%.
// If it does not reliably start, disable the ramp for full ON/OFF.
static const uint32_t PUMP_START_DUTY_PERCENT = 65UL;
static const uint32_t PUMP_HARD_LIMIT_MS = 20UL * 1000UL; // TEST: 20 s fault cutoff

static const uint32_t LIGHT_ON_MS = 2UL * 60UL * 1000UL; // TEST: 2 min
static const uint32_t LIGHT_OFF_MS = 1UL * 60UL * 1000UL; // TEST: 1 min
static const uint32_t LIGHT_CYCLE_MS = LIGHT_ON_MS + LIGHT_OFF_MS;
static const uint32_t LIGHT_FADE_MS = 10UL * 1000UL;      // TEST: 10 s fade in/out

// Prevent the enclosed growing area from overheating.
// These are PROTOTYPE thresholds, not universal limits for all species.
static constexpr float TEMP_LIGHTS_OFF_C = 28.0f;
static constexpr float TEMP_LIGHTS_ON_C = 26.0f;
static const uint32_t TEMP_MIN_OFF_MS = 300UL * 1000UL;
static const uint32_t TEMP_SAMPLE_MS = 2000UL;
static const uint32_t SENSOR_RETRY_MS = 5000UL;

// Status LED: a 150 ms flash every 1000 ms when both pump and light are on.
static const uint32_t RGB_HEARTBEAT_PERIOD_MS = 1000UL;
static const uint32_t RGB_HEARTBEAT_ON_MS = 150UL;
static const uint32_t RGB_ERROR_BLINK_MS = 250UL;

static const uint32_t SERIAL_BAUD = 115200UL;
static const uint32_t STATUS_PRINT_MS = 5000UL;

// GPIO20 is a DIGITAL-only input. Set true once the light sensor DO
// wire is installed. Its level is printed but does not control the lights.
static const uint8_t PIN_LIGHT_SENSOR_DO = 20U;
static const bool LIGHT_SENSOR_INSTALLED = false;

// Arduino-ESP32 core 2.x LEDC PWM channels.
static const uint8_t PUMP_PWM_CHANNEL = 2U;
static const uint8_t LIGHT_PWM_CHANNEL = 0U;
static const uint32_t PUMP_PWM_HZ = 1000UL;
static const uint32_t LIGHT_PWM_HZ = 1000UL;
static const uint8_t PWM_BITS = 12U;
static const uint32_t PWM_MAX = (1UL << PWM_BITS) - 1UL;

static_assert(WATER_FIXED_SEC * 1000UL < PUMP_HARD_LIMIT_MS,
              "Watering must end before the emergency limit.");
static_assert(LIGHT_FADE_MS * 2UL <= LIGHT_ON_MS,
              "Both light fades must fit inside the light-on period.");
static_assert(PUMP_START_DUTY_PERCENT <= 100UL,
              "Pump starting PWM duty must not exceed 100 percent.");

// ------------------------------ State -------------------------------------
Adafruit_AHTX0 aht;

static bool ahtInitialized = false;
static bool sensorOK = false;
static float temperatureC = NAN;
static float humidityPct = NAN;
static uint32_t lastSensorReadMs = 0UL;
static uint32_t lastSensorRetryMs = 0UL;

static bool thermalPause = false;
static uint32_t thermalPauseStartMs = 0UL;

static bool pumpOn = false;
static bool pumpFault = false;
static uint32_t pumpStartMs = 0UL;
static uint32_t lastWaterStartMs = 0UL;
static uint32_t activeWaterSec = WATER_FIXED_SEC;

static uint32_t lightCycleStartMs = 0UL;
static bool lightOn = false;
static uint32_t lightDuty = 0UL;
static uint32_t pumpDuty = 0UL;

// ------------------------ Hardware helper functions -----------------------
static void rgbWrite(bool red, bool green, bool blue) {
  // Common-anode LED: LOW = ON. Each color MUST have its own resistor.
  digitalWrite(PIN_RGB_RED, red ? LOW : HIGH);
  digitalWrite(PIN_RGB_GREEN, green ? LOW : HIGH);
  digitalWrite(PIN_RGB_BLUE, blue ? LOW : HIGH);
}

static void pumpPwmWrite(uint32_t duty) {
  if (duty > PWM_MAX) duty = PWM_MAX;
  if (duty != pumpDuty) {
    ledcWrite(PUMP_PWM_CHANNEL, duty);
    pumpDuty = duty;
  }
}

static void lightPwmWrite(uint32_t duty) {
  if (duty > PWM_MAX) duty = PWM_MAX;
  if (duty != lightDuty) {
    ledcWrite(LIGHT_PWM_CHANNEL, duty);
    lightDuty = duty;
  }
  lightOn = duty > 0UL;
}

// Read the original resistor-limited potentiometer circuit.
// In TEST mode, keep the pump duration fixed at 10 seconds.
static uint32_t selectedWaterSec() {
  if (!USE_POT_FOR_WATER_DURATION) return WATER_FIXED_SEC;

  uint32_t mv = (uint32_t)analogReadMilliVolts(PIN_POT);
  if (mv < POT_MV_LOW) mv = POT_MV_LOW;
  if (mv > POT_MV_HIGH) mv = POT_MV_HIGH;
  uint32_t fraction = mv - POT_MV_LOW;
  if (POT_REVERSED) fraction = POT_MV_HIGH - mv;

  const uint32_t steps = (WATER_MAX_SEC - WATER_MIN_SEC) / WATER_STEP_SEC;
  uint32_t stepIndex = (fraction * steps +
                        (POT_MV_HIGH - POT_MV_LOW) / 2UL) /
                       (POT_MV_HIGH - POT_MV_LOW);
  return WATER_MIN_SEC + stepIndex * WATER_STEP_SEC;
}

static void stopPump(uint32_t now) {
  pumpPwmWrite(0UL);
  pumpOn = false;
  Serial.printf("[%lu s] pump OFF\n", (unsigned long)(now / 1000UL));
}

static void startPump(uint32_t now) {
  if (pumpFault || pumpOn) return;
  pumpStartMs = now;
  lastWaterStartMs = now;   // 60-minute interval is start-to-start
  activeWaterSec = selectedWaterSec();
  pumpOn = true;
  // We deliberately do not output PWM until updatePump() runs.
  // For first tests keep the pump under water; use the w command if needed.
  Serial.printf("[%lu s] pump ON for %lu s\n",
                (unsigned long)(now / 1000UL),
                (unsigned long)activeWaterSec);
}

static void updatePump(uint32_t now) {
  if (pumpFault) {
    pumpPwmWrite(0UL);
    return;
  }

  if (pumpOn) {
    const uint32_t elapsed = now - pumpStartMs;
    if (elapsed >= PUMP_HARD_LIMIT_MS) {
      stopPump(now);
      pumpFault = true;
      Serial.println("FAULT: pump time limit exceeded; reset required");
      return;
    }
    if (elapsed >= activeWaterSec * 1000UL) {
      stopPump(now);
      return;
    }

    if (!PUMP_USE_PWM_RAMP || elapsed >= PUMP_RAMP_MS) {
      pumpPwmWrite(PWM_MAX);
    } else {
      const uint32_t initialDuty =
          (PUMP_START_DUTY_PERCENT * PWM_MAX) / 100UL;
      const uint32_t duty = initialDuty +
          (uint32_t)(((uint64_t)(PWM_MAX - initialDuty) * elapsed) /
                     PUMP_RAMP_MS);
      pumpPwmWrite(duty);
    }
  } else if ((uint32_t)(now - lastWaterStartMs) >= WATER_INTERVAL_MS) {
    startPump(now);
  }
}

// --------------------------- Temperature guard ----------------------------
static void updateSensor(uint32_t now) {
  if (!ahtInitialized) {
    if ((uint32_t)(now - lastSensorRetryMs) < SENSOR_RETRY_MS) return;
    lastSensorRetryMs = now;
    ahtInitialized = aht.begin(&Wire);
    if (!ahtInitialized) {
      sensorOK = false;
      Serial.println("ERROR: AHT10 not found. Grow lights are OFF.");
    } else {
      Serial.println("AHT10 connected.");
      lastSensorReadMs = now - TEMP_SAMPLE_MS;
    }
    return;
  }

  if ((uint32_t)(now - lastSensorReadMs) < TEMP_SAMPLE_MS) return;
  lastSensorReadMs = now;

  sensors_event_t humidity;
  sensors_event_t temperature;
  const bool readOK = aht.getEvent(&humidity, &temperature);
  const float t = temperature.temperature;
  const float h = humidity.relative_humidity;
  sensorOK = readOK && isfinite(t) && isfinite(h) &&
             t >= -10.0f && t <= 60.0f && h >= 0.0f && h <= 100.0f;

  if (!sensorOK) {
    ahtInitialized = false; // try again in 5 seconds
    Serial.println("ERROR: AHT10 read failed. Grow lights are OFF.");
    return;
  }

  temperatureC = t;
  humidityPct = h;

  if (!thermalPause && temperatureC >= TEMP_LIGHTS_OFF_C) {
    thermalPause = true;
    thermalPauseStartMs = now;
    Serial.printf("HEAT: %.1f C >= %.1f C. LEDs OFF for at least 300 s.\n",
                  temperatureC, TEMP_LIGHTS_OFF_C);
  }

  if (thermalPause &&
      (uint32_t)(now - thermalPauseStartMs) >= TEMP_MIN_OFF_MS &&
      temperatureC <= TEMP_LIGHTS_ON_C) {
    thermalPause = false;
    Serial.printf("COOLED: %.1f C <= %.1f C. Lights may resume.\n",
                  temperatureC, TEMP_LIGHTS_ON_C);
  }
}

// ----------------------------- Grow lights --------------------------------
static void updateLights(uint32_t now) {
  // Advance by complete light cycles to keep millis() wrap-around safe.
  while ((uint32_t)(now - lightCycleStartMs) >= LIGHT_CYCLE_MS) {
    lightCycleStartMs += LIGHT_CYCLE_MS;
  }

  const uint32_t phase = now - lightCycleStartMs;
  uint32_t requestedDuty = 0UL;

  if (phase < LIGHT_ON_MS) {
    if (phase < LIGHT_FADE_MS) {
      // Sunrise: slowly brighten during the first 10 seconds.
      requestedDuty = (uint32_t)(((uint64_t)PWM_MAX * phase) /
                                 LIGHT_FADE_MS);
    } else if (phase >= LIGHT_ON_MS - LIGHT_FADE_MS) {
      // Sunset: slowly dim during the last 10 seconds.
      requestedDuty = (uint32_t)(((uint64_t)PWM_MAX *
                                 (LIGHT_ON_MS - phase)) / LIGHT_FADE_MS);
    } else {
      requestedDuty = PWM_MAX;
    }
  }

  // A broken/missing AHT10 sensor, overheating, or a pump fault
  // disables the lights, even during the scheduled daylight phase.
  if (!sensorOK || thermalPause || pumpFault) requestedDuty = 0UL;
  lightPwmWrite(requestedDuty);
}

// ----------------------------- RGB indicator ------------------------------
static void updateRGB(uint32_t now) {
  if (pumpFault || !sensorOK || thermalPause) {
    const bool redPulse =
        ((now / RGB_ERROR_BLINK_MS) % 2UL) == 0UL;
    rgbWrite(redPulse, false, false);
    return;
  }

  if (lightOn && pumpOn) {
    // Both run: GREEN + BLUE flash together once per second (cyan heartbeat).
    const bool pulse =
        (now % RGB_HEARTBEAT_PERIOD_MS) < RGB_HEARTBEAT_ON_MS;
    rgbWrite(false, pulse, pulse);
  } else if (lightOn) {
    rgbWrite(false, true, false);   // green steady: grow light is ON
  } else if (pumpOn) {
    rgbWrite(false, false, true);   // blue steady: pump is ON
  } else {
    rgbWrite(false, false, false); // normal: both OFF
  }
}

static void printStatus(uint32_t now) {
  const uint32_t phase = now - lightCycleStartMs;
  Serial.printf("[%lu s] T=%.1f C RH=%.1f%% sensor=%s ",
                (unsigned long)(now / 1000UL), temperatureC, humidityPct,
                sensorOK ? "OK" : "ERROR");
  Serial.printf("pump=%s light=%lu%% phase=%s heatPause=%s fault=%s "
                "waterTime=%lu s",
                pumpOn ? "ON" : "OFF",
                (unsigned long)((lightDuty * 100UL) / PWM_MAX),
                phase < LIGHT_ON_MS ? "DAY" : "NIGHT",
                thermalPause ? "YES" : "NO",
                pumpFault ? "YES" : "NO",
                (unsigned long)(pumpOn ? activeWaterSec : selectedWaterSec()));
  if (LIGHT_SENSOR_INSTALLED) {
    Serial.printf(" lightDO=%s",
                  digitalRead(PIN_LIGHT_SENSOR_DO) ? "HIGH" : "LOW");
  }
  Serial.println();
}

static void processSerial(uint32_t now) {
  while (Serial.available() > 0) {
    const char command = (char)Serial.read();
    switch (command) {
      case 'w':
        if (!pumpFault) startPump(now);
        else Serial.println("Pump FAULT; reset first.");
        break;
      case 'p':
        if (pumpOn) stopPump(now);
        break;
      case 's':
        printStatus(now);
        break;
      case 'h':
        Serial.println("Commands: w=water now, p=stop, s=status, h=help");
        break;
      case '\r':
      case '\n':
        break;
      default:
        Serial.println("Unknown command. Type h for help.");
        break;
    }
  }
}

void setup() {
  // Start with all 12 V loads OFF (assuming 3.3 V active-HIGH MOSFETs).
  pinMode(PIN_PUMP, OUTPUT);
  digitalWrite(PIN_PUMP, LOW);
  pinMode(PIN_LIGHT, OUTPUT);
  digitalWrite(PIN_LIGHT, LOW);

  pinMode(PIN_RGB_RED, OUTPUT);
  pinMode(PIN_RGB_GREEN, OUTPUT);
  pinMode(PIN_RGB_BLUE, OUTPUT);
  rgbWrite(false, false, false);

  Serial.begin(SERIAL_BAUD);
  delay(1500UL);  // USB CDC time to connect after reset
  Serial.println("PISI controller starting (ESP32-C3 / Arduino core 2.x)");

  ledcSetup(PUMP_PWM_CHANNEL, PUMP_PWM_HZ, PWM_BITS);
  ledcAttachPin(PIN_PUMP, PUMP_PWM_CHANNEL);
  ledcSetup(LIGHT_PWM_CHANNEL, LIGHT_PWM_HZ, PWM_BITS);
  ledcAttachPin(PIN_LIGHT, LIGHT_PWM_CHANNEL);
  ledcWrite(PUMP_PWM_CHANNEL, 0UL);
  ledcWrite(LIGHT_PWM_CHANNEL, 0UL);

  if (LIGHT_SENSOR_INSTALLED) {
    // Connect digital DO, not analog AO, to GPIO20.
    pinMode(PIN_LIGHT_SENSOR_DO, INPUT);
  }

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_POT, ADC_11db);
  pinMode(PIN_POT, INPUT);

  Wire.begin(PIN_AHT_SDA, PIN_AHT_SCL);
  Wire.setClock(100000UL);
  Wire.setTimeOut(50);

  // The first sensor connection attempt starts in loop() after startup.
  const uint32_t now = millis();
  lightCycleStartMs = now;
  lastWaterStartMs = now;
  lastSensorRetryMs = now - SENSOR_RETRY_MS;
  lastSensorReadMs = now - TEMP_SAMPLE_MS;

  Serial.println("TEST: grow lights 120 s ON / 60 s OFF, 10 s fade in/out");
  Serial.println("TEST: pump 10 s every 120 s; 1 s experimental PWM ramp");
  Serial.println("High temperature: lights OFF at 28 C; resume <=26 C after 300 s");
  Serial.println("TEST: 20 s pump fault cutoff. Pump starts on command w or at t=120 s.");
  Serial.println("Commands: w=water, p=stop, s=status, h=help");
  if (WATER_ON_BOOT) startPump(now);
}

void loop() {
  static uint32_t lastTickMs = 0UL;
  static uint32_t lastStatusMs = 0UL;
  const uint32_t now = millis();
  processSerial(now);

  if ((uint32_t)(now - lastTickMs) < TICK_MS) return;
  lastTickMs = now;

  updateSensor(now);
  updatePump(now);
  updateLights(now);
  updateRGB(now);

  if ((uint32_t)(now - lastStatusMs) >= STATUS_PRINT_MS) {
    lastStatusMs = now;
    printStatus(now);
  }
}
