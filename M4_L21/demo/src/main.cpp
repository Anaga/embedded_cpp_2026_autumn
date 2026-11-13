/*
 * Lesson 21 - CAN bus through TWAI
 * ESP32-C3 Super Mini + TJA1050, two nodes on one 500 kbit/s bus.
 *
 * What it does:
 *   - every 1 s: heartbeat frame, ID 0x100 + NODE_ID, 4 bytes:
 *       counter (big-endian 16 bit), potentiometer (big-endian 16 bit)
 *   - button A: LED command frame, ID 0x200, 4 bytes:
 *       target node (the OTHER node: 1 -> 2, 2 -> 1), R, G, B
 *       colour depends on the potentiometer position
 *   - button B: same frame with R = G = B = 0 (LED off on the other node)
 *   - every 5 s: TWAI status line (state, TEC, REC, bus errors)
 *   - alerts: error passive, bus off, automatic bus-off recovery
 *
 * NODE_ID comes from platformio.ini (-D NODE_ID=n). Do not edit it here.
 * CAN_SELFTEST (env selftest) switches the controller to TWAI_MODE_NO_ACK so
 * one board alone hears its own frames.
 *
 * Wiring (see README.md): CAN TX GPIO 10 -> TJA1050 TXD,
 * TJA1050 RXD -> 10k / 20k divider -> GPIO 3. TJA1050 VCC = 5 V, common GND.
 */

#include <Arduino.h>
#include <stdint.h>
#include "driver/twai.h"

#ifndef NODE_ID
#error "NODE_ID is not defined. Build with -e node1, node2 or selftest."
#endif

// ---------------------------------------------------------------- pins
static const gpio_num_t CAN_TX_PIN = GPIO_NUM_10;
static const gpio_num_t CAN_RX_PIN = GPIO_NUM_3;

static const uint8_t BUTTON_A_PIN = 0U;   // INPUT_PULLUP, active low
static const uint8_t BUTTON_B_PIN = 1U;   // INPUT_PULLUP, active low
static const uint8_t POT_PIN      = 4U;   // ADC1, 10k pot with 3.3k in series
static const uint8_t LED_R_PIN    = 5U;   // common anode: LOW = on
static const uint8_t LED_G_PIN    = 6U;
static const uint8_t LED_B_PIN    = 7U;

// ---------------------------------------------------------------- CAN protocol
static const uint32_t HEARTBEAT_BASE_ID = 0x100U;   // + NODE_ID
static const uint32_t CMD_LED_ID        = 0x200U;
static const uint8_t  HEARTBEAT_DLC     = 4U;
static const uint8_t  CMD_LED_DLC       = 4U;
static const uint8_t  NODE_COUNT        = 2U;

// ---------------------------------------------------------------- timing
static const uint32_t HEARTBEAT_PERIOD_MS = 1000U;
static const uint32_t STATUS_PERIOD_MS    = 5000U;
static const uint32_t DEBOUNCE_MS         = 40U;
static const uint32_t TX_TIMEOUT_MS       = 10U;

// ---------------------------------------------------------------- PWM (core 2.x channel API)
static const uint8_t  PWM_CH_R   = 0U;
static const uint8_t  PWM_CH_G   = 1U;
static const uint8_t  PWM_CH_B   = 2U;
static const uint32_t PWM_FREQ   = 5000U;
static const uint8_t  PWM_BITS   = 8U;
static const uint16_t ADC_MAX    = 4095U;

// ---------------------------------------------------------------- state
static uint16_t g_counter = 0U;
static uint32_t g_lastHeartbeatMs = 0U;
static uint32_t g_lastStatusMs = 0U;

struct Button {
    uint8_t  pin;
    bool     lastStable;
    bool     lastRaw;
    uint32_t lastChangeMs;
};

static Button g_buttonA = { BUTTON_A_PIN, true, true, 0U };
static Button g_buttonB = { BUTTON_B_PIN, true, true, 0U };

// ---------------------------------------------------------------- helpers
static void setRgb(uint8_t r, uint8_t g, uint8_t b) {
    // Common anode: 255 = off, 0 = full on, so invert the duty.
    ledcWrite(PWM_CH_R, 255U - r);
    ledcWrite(PWM_CH_G, 255U - g);
    ledcWrite(PWM_CH_B, 255U - b);
}

// Potentiometer position -> a colour on a simple hue wheel (R -> G -> B).
static void potToColour(uint16_t pot, uint8_t *r, uint8_t *g, uint8_t *b) {
    const uint32_t third = (ADC_MAX + 1U) / 3U;
    uint32_t pos = pot;
    if (pos < third) {
        *r = (uint8_t)(255U - (pos * 255U) / third);
        *g = (uint8_t)((pos * 255U) / third);
        *b = 0U;
    } else if (pos < 2U * third) {
        pos -= third;
        *r = 0U;
        *g = (uint8_t)(255U - (pos * 255U) / third);
        *b = (uint8_t)((pos * 255U) / third);
    } else {
        pos -= 2U * third;
        *r = (uint8_t)((pos * 255U) / third);
        *g = 0U;
        *b = (uint8_t)(255U - (pos * 255U) / third);
    }
}

// Returns true once per press (falling edge after debounce).
static bool buttonPressed(Button *btn, uint32_t nowMs) {
    bool raw = (digitalRead(btn->pin) == HIGH);
    if (raw != btn->lastRaw) {
        btn->lastRaw = raw;
        btn->lastChangeMs = nowMs;
    }
    if ((nowMs - btn->lastChangeMs) >= DEBOUNCE_MS && raw != btn->lastStable) {
        btn->lastStable = raw;
        return !raw;   // went LOW = pressed
    }
    return false;
}

static const char *stateName(twai_state_t st) {
    switch (st) {
        case TWAI_STATE_STOPPED:    return "STOPPED";
        case TWAI_STATE_RUNNING:    return "RUNNING";
        case TWAI_STATE_BUS_OFF:    return "BUS_OFF";
        case TWAI_STATE_RECOVERING: return "RECOVERING";
        default:                    return "?";
    }
}

// ---------------------------------------------------------------- CAN
static void canBegin(void) {
#ifdef CAN_SELFTEST
    const twai_mode_t mode = TWAI_MODE_NO_ACK;
#else
    const twai_mode_t mode = TWAI_MODE_NORMAL;
#endif
    twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, mode);
    g.alerts_enabled = TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_OFF |
                       TWAI_ALERT_BUS_RECOVERED | TWAI_ALERT_ERR_ACTIVE |
                       TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_BUS_ERROR;
    twai_timing_config_t t = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_ERROR_CHECK(twai_driver_install(&g, &t, &f));
    ESP_ERROR_CHECK(twai_start());
    Serial.printf("TWAI started: node %u, TX GPIO %d, RX GPIO %d, 500 kbit/s, mode %s\n",
                  static_cast<unsigned>(NODE_ID), (int)CAN_TX_PIN, (int)CAN_RX_PIN,
                  (mode == TWAI_MODE_NO_ACK) ? "NO_ACK (selftest)" : "NORMAL");
}

static void canSend(const twai_message_t *msg, const char *what) {
    esp_err_t err = twai_transmit(msg, pdMS_TO_TICKS(TX_TIMEOUT_MS));
    if (err != ESP_OK) {
        Serial.printf("TX fail (%s): %s\n", what, esp_err_to_name(err));
    }
}

static void sendHeartbeat(uint16_t pot) {
    twai_message_t msg = {};
    msg.identifier = HEARTBEAT_BASE_ID + NODE_ID;
    msg.extd = 0;
    msg.rtr = 0;
#ifdef CAN_SELFTEST
    msg.self = 1;   // self-reception request: the frame comes back into our RX queue
#endif
    msg.data_length_code = HEARTBEAT_DLC;
    msg.data[0] = (uint8_t)(g_counter >> 8);
    msg.data[1] = (uint8_t)(g_counter);
    msg.data[2] = (uint8_t)(pot >> 8);
    msg.data[3] = (uint8_t)(pot);
    canSend(&msg, "heartbeat");
    g_counter++;
}

static void sendLedCommand(uint8_t targetNode, uint8_t r, uint8_t g, uint8_t b) {
    twai_message_t msg = {};
    msg.identifier = CMD_LED_ID;
    msg.extd = 0;
    msg.rtr = 0;
#ifdef CAN_SELFTEST
    msg.self = 1;
#endif
    msg.data_length_code = CMD_LED_DLC;
    msg.data[0] = targetNode;
    msg.data[1] = r;
    msg.data[2] = g;
    msg.data[3] = b;
    canSend(&msg, "led command");
    Serial.printf("-> LED cmd to node %u: %u %u %u\n",
                  static_cast<unsigned>(targetNode), static_cast<unsigned>(r),
                  static_cast<unsigned>(g), static_cast<unsigned>(b));
}

static void printHeartbeat(const twai_message_t *rx) {
    uint16_t counter = (uint16_t)((rx->data[0] << 8) | rx->data[1]);
    uint16_t pot     = (uint16_t)((rx->data[2] << 8) | rx->data[3]);
    Serial.printf("<- node %lu: counter %u pot %u\n",
                  static_cast<unsigned long>(rx->identifier - HEARTBEAT_BASE_ID),
                  static_cast<unsigned>(counter), static_cast<unsigned>(pot));
}

static void canPoll(void) {
    twai_message_t rx;
    while (twai_receive(&rx, 0) == ESP_OK) {
        if (rx.rtr) { continue; }
        if (rx.identifier == CMD_LED_ID && rx.data_length_code == CMD_LED_DLC) {
            if (rx.data[0] == NODE_ID) {
                setRgb(rx.data[1], rx.data[2], rx.data[3]);
                Serial.printf("<- LED cmd for me: %u %u %u\n",
                              static_cast<unsigned>(rx.data[1]),
                              static_cast<unsigned>(rx.data[2]),
                              static_cast<unsigned>(rx.data[3]));
            }
        } else if (rx.identifier > HEARTBEAT_BASE_ID &&
                   rx.identifier <= HEARTBEAT_BASE_ID + NODE_COUNT &&
                   rx.data_length_code == HEARTBEAT_DLC) {
            printHeartbeat(&rx);
        } else {
            Serial.printf("<- unknown ID 0x%03lX DLC %u\n",
                          static_cast<unsigned long>(rx.identifier),
                          static_cast<unsigned>(rx.data_length_code));
        }
    }
}

static void canHandleAlerts(void) {
    uint32_t alerts = 0U;
    if (twai_read_alerts(&alerts, 0) != ESP_OK) { return; }

    if (alerts & TWAI_ALERT_BUS_ERROR)     { Serial.println("ALERT: bus error (bit, stuff, CRC, form or ACK error)"); }
    if (alerts & TWAI_ALERT_RX_QUEUE_FULL) { Serial.println("ALERT: RX queue full, frames lost - poll more often"); }
    if (alerts & TWAI_ALERT_ERR_PASS)      { Serial.println("ALERT: error passive (TEC or REC >= 128)"); }
    if (alerts & TWAI_ALERT_ERR_ACTIVE)    { Serial.println("ALERT: back to error active"); }
    if (alerts & TWAI_ALERT_BUS_OFF) {
        Serial.println("ALERT: BUS OFF - starting recovery (128 x 11 recessive bits needed)");
        twai_initiate_recovery();
    }
    if (alerts & TWAI_ALERT_BUS_RECOVERED) {
        // After recovery the driver is in STOPPED state; start it again.
        Serial.println("ALERT: bus recovered - restarting");
        twai_start();
    }
}

static void printStatus(void) {
    twai_status_info_t st;
    if (twai_get_status_info(&st) != ESP_OK) { return; }
    Serial.printf("STATUS %s TEC %lu REC %lu busErr %lu txFail %lu rxMissed %lu arbLost %lu\n",
                  stateName(st.state),
                  static_cast<unsigned long>(st.tx_error_counter),
                  static_cast<unsigned long>(st.rx_error_counter),
                  static_cast<unsigned long>(st.bus_error_count),
                  static_cast<unsigned long>(st.tx_failed_count),
                  static_cast<unsigned long>(st.rx_missed_count),
                  static_cast<unsigned long>(st.arb_lost_count));
}

// ---------------------------------------------------------------- Arduino
void setup(void) {
    Serial.begin(115200);
    delay(1500U);   // USB CDC needs a moment before the first line is visible

    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN, INPUT_PULLUP);

    ledcSetup(PWM_CH_R, PWM_FREQ, PWM_BITS);
    ledcSetup(PWM_CH_G, PWM_FREQ, PWM_BITS);
    ledcSetup(PWM_CH_B, PWM_FREQ, PWM_BITS);
    ledcAttachPin(LED_R_PIN, PWM_CH_R);
    ledcAttachPin(LED_G_PIN, PWM_CH_G);
    ledcAttachPin(LED_B_PIN, PWM_CH_B);
    setRgb(0U, 0U, 0U);

    Serial.println();
    Serial.println("Lesson 21 - CAN through TWAI");
    canBegin();
}

void loop(void) {
    const uint32_t now = millis();
    const uint16_t pot = (uint16_t)analogRead(POT_PIN);

    canPoll();
    canHandleAlerts();

    if (now - g_lastHeartbeatMs >= HEARTBEAT_PERIOD_MS) {
        g_lastHeartbeatMs = now;
        sendHeartbeat(pot);
    }

    if (now - g_lastStatusMs >= STATUS_PERIOD_MS) {
        g_lastStatusMs = now;
        printStatus();
    }

#ifdef CAN_SELFTEST
    const uint8_t nextNode = NODE_ID;   // alone on the bench: talk to yourself
#else
    const uint8_t nextNode = (uint8_t)((NODE_ID % NODE_COUNT) + 1U);   // 1 -> 2, 2 -> 1
#endif

    if (buttonPressed(&g_buttonA, now)) {
        uint8_t r = 0U, g = 0U, b = 0U;
        potToColour(pot, &r, &g, &b);
        sendLedCommand(nextNode, r, g, b);
    }
    if (buttonPressed(&g_buttonB, now)) {
        sendLedCommand(nextNode, 0U, 0U, 0U);
    }
}
