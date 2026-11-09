// Lesson 20 - Lab 1: AHT10 node on our own MQTT broker (ESP32-C3 Super Mini)
//
// Topics, with <p> = MQTT_PREFIX and <id> = esp-xxxx from the MAC address:
//   <p>/<id>/status   "online" / "offline"   retained, "offline" is the last will
//   <p>/<id>/aht10    {"t":21.8,"rh":58.1,"seq":12}   every 5 s, not retained
//   <p>/<id>/led/set  "1" / "0"   command, the node subscribes (QoS 1)
//   <p>/<id>/led      "1" / "0"   state echo, retained
//
// RGB LED (common anode, LOW = on):
//   red   on while there is no connection to the broker
//   green follows the led/set command
//
// Wiring as in session 18: AHT10 SDA GPIO 8, SCL GPIO 9, power from 3V3.
// GPIO 8 and 9 are strapping pins. GPIO 9 is also the BOOT button: pressing
// it during a reading spoils the reading. The blue on-board LED on GPIO 8
// flickers with every I2C transfer - that is expected.

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_AHTX0.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "secrets.h"

// ---------------------------------------------------------------------------
// Pins
// ---------------------------------------------------------------------------

constexpr uint8_t PIN_SDA   = 8U;  // strapping pin
constexpr uint8_t PIN_SCL   = 9U;  // strapping pin, BOOT button
constexpr uint8_t PIN_RED   = 5U;
constexpr uint8_t PIN_GREEN = 6U;
constexpr uint8_t PIN_BLUE  = 7U;

constexpr uint8_t AHT10_ADDR = 0x38U;

// ---------------------------------------------------------------------------
// Timing and sizes
// ---------------------------------------------------------------------------

constexpr uint32_t SERIAL_BAUD       = 115200U;
constexpr uint32_t PUBLISH_PERIOD_MS = 5000U;
constexpr uint32_t MQTT_RETRY_MS     = 5000U;
constexpr uint16_t MQTT_KEEPALIVE_S  = 15U;   // broker declares us dead after 1.5 x this
constexpr uint16_t MQTT_BUFFER_BYTES = 256U;
constexpr uint8_t  QOS_1             = 1U;

constexpr size_t ID_LEN      = 12U;
constexpr size_t CLIENT_LEN  = 32U;
constexpr size_t TOPIC_LEN   = 64U;
constexpr size_t CMD_LEN     = 8U;
constexpr size_t JSON_LEN    = 96U;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

enum class Link : uint8_t { NoWifi, NoBroker, Online };

WiFiClient     net;
PubSubClient   mqtt(net);
Adafruit_AHTX0 aht;

char nodeId[ID_LEN];          // esp-xxxx
char clientId[CLIENT_LEN];    // gamma-ed-esp-xxxx, must be unique on the broker
char topicStatus[TOPIC_LEN];
char topicAht[TOPIC_LEN];
char topicLedSet[TOPIC_LEN];
char topicLed[TOPIC_LEN];

bool     sensorOk       = false;
bool     ledOn          = false;
uint32_t seq            = 0U;
uint32_t lastPublishMs  = 0U;
uint32_t lastAttemptMs  = 0U;
Link     link           = Link::NoWifi;
wl_status_t lastWifi    = WL_IDLE_STATUS;

// Filled by the MQTT callback, handled in loop() - see onMessage().
char cmdBuf[CMD_LEN];
bool cmdPending = false;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void setChannel(uint8_t pin, bool on) {
    digitalWrite(pin, on ? LOW : HIGH);  // common anode
}

const char *linkText(Link l) {
    switch (l) {
        case Link::NoWifi:   return "no WiFi";
        case Link::NoBroker: return "no broker";
        case Link::Online:   return "MQTT ok";
    }
    return "?";
}

const char *mqttStateText(int state) {
    switch (state) {
        case -4: return "timeout: the broker did not answer in time";
        case -3: return "connection lost";
        case -2: return "no TCP connection: broker down, wrong host or port, firewall";
        case -1: return "disconnected";
        case 0:  return "connected";
        case 1:  return "bad protocol version";
        case 2:  return "client id rejected";
        case 3:  return "broker unavailable";
        case 4:  return "bad user name or password";
        case 5:  return "not authorized";
    }
    return "unknown";
}

void buildNames() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(nodeId, sizeof nodeId, "esp-%02x%02x", mac[4], mac[5]);
    snprintf(clientId, sizeof clientId, "gamma-ed-%s", nodeId);
    snprintf(topicStatus, sizeof topicStatus, "%s/%s/status",  MQTT_PREFIX, nodeId);
    snprintf(topicAht,    sizeof topicAht,    "%s/%s/aht10",   MQTT_PREFIX, nodeId);
    snprintf(topicLedSet, sizeof topicLedSet, "%s/%s/led/set", MQTT_PREFIX, nodeId);
    snprintf(topicLed,    sizeof topicLed,    "%s/%s/led",     MQTT_PREFIX, nodeId);
}

// ---------------------------------------------------------------------------
// MQTT
// ---------------------------------------------------------------------------

// Runs inside mqtt.loop(). The payload points into PubSubClient's own buffer
// and is NOT zero-terminated. Publishing from here would overwrite that
// buffer, so: copy, set a flag, return. loop() does the work - the same rule
// as for an ISR in session 11.
void onMessage(char *topic, uint8_t *payload, unsigned int length) {
    if (strcmp(topic, topicLedSet) != 0) {
        Serial.printf("MQTT: message on an unexpected topic %s\n", topic);
        return;
    }
    const size_t n = (length < CMD_LEN - 1U) ? length : CMD_LEN - 1U;
    memcpy(cmdBuf, payload, n);
    cmdBuf[n] = '\0';
    cmdPending = true;
}

void publishLed() {
    mqtt.publish(topicLed, ledOn ? "1" : "0", true);  // retained: the current state
}

bool mqttConnect() {
    // Last will: if we vanish, the broker publishes "offline" for us (retained).
    const bool ok = mqtt.connect(clientId, topicStatus, QOS_1, true, "offline");
    if (!ok) {
        const int st = mqtt.state();
        Serial.printf("MQTT: connect to %s:%u failed, state %d (%s)\n",
                      MQTT_HOST, static_cast<unsigned>(MQTT_PORT), st, mqttStateText(st));
        return false;
    }
    mqtt.publish(topicStatus, "online", true);
    mqtt.subscribe(topicLedSet, QOS_1);
    publishLed();
    Serial.println("MQTT: connected");
    return true;
}

void handleCommand() {
    if (!cmdPending) {
        return;
    }
    cmdPending = false;
    if (strcmp(cmdBuf, "1") == 0) {
        ledOn = true;
    } else if (strcmp(cmdBuf, "0") == 0) {
        ledOn = false;
    } else {
        Serial.printf("MQTT: led/set ignored, payload \"%s\" (expected 1 or 0)\n", cmdBuf);
        return;
    }
    setChannel(PIN_GREEN, ledOn);
    publishLed();
    Serial.printf("LED: %s\n", ledOn ? "on" : "off");
}

// ---------------------------------------------------------------------------
// Connection watch - never blocks except inside mqtt.connect()
// ---------------------------------------------------------------------------

void watchWifi() {
    const wl_status_t st = WiFi.status();
    if (st == lastWifi) {
        return;
    }
    lastWifi = st;
    if (st == WL_CONNECTED) {
        Serial.printf("WiFi: connected, IP %s, %d dBm\n",
                      WiFi.localIP().toString().c_str(), static_cast<int>(WiFi.RSSI()));
    } else {
        Serial.printf("WiFi: not connected (status %d)\n", static_cast<int>(st));
    }
}

void watchMqtt(uint32_t now) {
    if (WiFi.status() != WL_CONNECTED) {
        link = Link::NoWifi;
        return;
    }
    if (mqtt.connected()) {
        link = Link::Online;
        return;
    }
    if (link == Link::Online) {
        const int st = mqtt.state();
        Serial.printf("MQTT: lost the broker, state %d (%s)\n", st, mqttStateText(st));
    }
    link = Link::NoBroker;
    // connect() can block for a few seconds, so try at most every 5 s.
    if (lastAttemptMs == 0U || now - lastAttemptMs >= MQTT_RETRY_MS) {
        lastAttemptMs = now;
        if (mqttConnect()) {
            link = Link::Online;
        }
    }
}

// ---------------------------------------------------------------------------
// Sensor
// ---------------------------------------------------------------------------

bool sensorStart() {
    Wire.beginTransmission(AHT10_ADDR);
    if (Wire.endTransmission() != 0U) {
        return false;
    }
    return aht.begin(&Wire);
}

void publishReading() {
    seq++;
    JsonDocument doc;
    float t  = NAN;
    float rh = NAN;
    if (sensorOk) {
        sensors_event_t hum;
        sensors_event_t temp;
        aht.getEvent(&hum, &temp);  // blocks about 50-80 ms
        t  = temp.temperature;
        rh = hum.relative_humidity;
        doc["t"]  = roundf(t * 10.0F) / 10.0F;
        doc["rh"] = roundf(rh * 10.0F) / 10.0F;
    } else {
        doc["error"] = "sensor";
    }
    doc["seq"] = seq;

    char json[JSON_LEN];
    serializeJson(doc, json, sizeof json);
    if (link == Link::Online) {
        mqtt.publish(topicAht, json, false);
    }

    if (sensorOk) {
        Serial.printf("%s | WiFi %d dBm | %s | %.1f C  %.1f %% | LED %s | seq %lu\n",
                      nodeId, static_cast<int>(WiFi.RSSI()), linkText(link),
                      static_cast<double>(t), static_cast<double>(rh),
                      ledOn ? "on" : "off", static_cast<unsigned long>(seq));
    } else {
        Serial.printf("%s | WiFi %d dBm | %s | sensor missing | LED %s | seq %lu\n",
                      nodeId, static_cast<int>(WiFi.RSSI()), linkText(link),
                      ledOn ? "on" : "off", static_cast<unsigned long>(seq));
    }
}

// ---------------------------------------------------------------------------
// setup / loop
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(SERIAL_BAUD);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);
    setChannel(PIN_RED, true);    // no broker yet
    setChannel(PIN_GREEN, false);
    setChannel(PIN_BLUE, false);

    Wire.begin(static_cast<int>(PIN_SDA), static_cast<int>(PIN_SCL));
    sensorOk = sensorStart();

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);  // many Super Mini boards need this
    buildNames();

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMessage);
    mqtt.setKeepAlive(MQTT_KEEPALIVE_S);
    mqtt.setBufferSize(MQTT_BUFFER_BYTES);

    Serial.printf("\nLab 1 - MQTT node %s, client id %s\n", nodeId, clientId);
    Serial.printf("AHT10: %s\n", sensorOk ? "found at 0x38" : "NOT found - check wiring");
    Serial.printf("MQTT: broker %s:%u\n", MQTT_HOST, static_cast<unsigned>(MQTT_PORT));
    Serial.printf("  publish   %s, %s, %s\n", topicStatus, topicAht, topicLed);
    Serial.printf("  subscribe %s\n", topicLedSet);
}

void loop() {
    const uint32_t now = millis();

    watchWifi();
    watchMqtt(now);
    mqtt.loop();        // keepalive, incoming messages -> onMessage()
    handleCommand();

    if (now - lastPublishMs >= PUBLISH_PERIOD_MS) {
        lastPublishMs = now;
        publishReading();
    }

    setChannel(PIN_RED, link != Link::Online);
}
