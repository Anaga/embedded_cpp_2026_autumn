/*
 * Homework 17 - Motor control and PWM modulation (robot chassis prep).
 *
 * Two DC motors via L298N (or MX1508) driver. Speed is controlled with
 * LEDC PWM on the enable pins. Direction via two GPIOs per motor.
 *
 * Non-blocking: all movement commands are scheduled with millis().
 *
 * Wiring:
 *   Left motor  : ENA GP13, IN1 GP14, IN2 GP27
 *   Right motor : ENB GP12, IN3 GP26, IN4 GP25
 *   (see include/Config.h)
 *
 * Commands over USB CDC:
 *   fwd  <0..255>  - forward at duty
 *   back <0..255>  - backward at duty
 *   left <0..255>  - turn left (pivot)
 *   right <0..255> - turn right (pivot)
 *   stop            - stop both motors
 *   help            - list commands
 */

#include <Arduino.h>
#include <string>

// ==================== Pins ====================
static const uint8_t PIN_M_L_EN = 13;
static const uint8_t PIN_M_L_IN1 = 14;
static const uint8_t PIN_M_L_IN2 = 27;
static const uint8_t PIN_M_R_EN = 12;
static const uint8_t PIN_M_R_IN1 = 26;
static const uint8_t PIN_M_R_IN2 = 25;

// ==================== PWM config (named constants) ====================
static const uint32_t PWM_FREQ = 1000;       // Hz
static const uint8_t  PWM_RES_BITS = 8;     // 0..255
static const uint8_t  PWM_CH_L = 0;
static const uint8_t  PWM_CH_R = 1;

// ==================== State ====================
enum class MotorState : uint8_t { STOP, FORWARD, BACKWARD, TURN_LEFT, TURN_RIGHT };
static MotorState motor_state = MotorState::STOP;
static uint8_t motor_speed = 0;

// ==================== Helpers ====================
static void setLeft(int duty) {   // duty: -255..255
    if (duty > 0) {
        digitalWrite(PIN_M_L_IN1, HIGH);
        digitalWrite(PIN_M_L_IN2, LOW);
    } else if (duty < 0) {
        digitalWrite(PIN_M_L_IN1, LOW);
        digitalWrite(PIN_M_L_IN2, HIGH);
    } else {
        digitalWrite(PIN_M_L_IN1, LOW);
        digitalWrite(PIN_M_L_IN2, LOW);
    }
    ledcWrite(PWM_CH_L, (uint32_t)(duty < 0 ? -duty : duty));
}

static void setRight(int duty) {  // duty: -255..255
    if (duty > 0) {
        digitalWrite(PIN_M_R_IN1, HIGH);
        digitalWrite(PIN_M_R_IN2, LOW);
    } else if (duty < 0) {
        digitalWrite(PIN_M_R_IN1, LOW);
        digitalWrite(PIN_M_R_IN2, HIGH);
    } else {
        digitalWrite(PIN_M_R_IN1, LOW);
        digitalWrite(PIN_M_R_IN2, LOW);
    }
    ledcWrite(PWM_CH_R, (uint32_t)(duty < 0 ? -duty : duty));
}

static void stopMotors() {
    setLeft(0);
    setRight(0);
    motor_state = MotorState::STOP;
    motor_speed = 0;
    Serial.println("motors: stop");
}

static void moveForward(uint8_t speed) {
    setLeft(speed);
    setRight(speed);
    motor_state = MotorState::FORWARD;
    motor_speed = speed;
    Serial.print("motors: forward duty=");
    Serial.println(speed);
}

static void moveBackward(uint8_t speed) {
    setLeft(-speed);
    setRight(-speed);
    motor_state = MotorState::BACKWARD;
    motor_speed = speed;
    Serial.print("motors: backward duty=");
    Serial.println(speed);
}

static void turnLeft(uint8_t speed) {
    setLeft(-speed);   // pivot: left reverse, right forward
    setRight(speed);
    motor_state = MotorState::TURN_LEFT;
    motor_speed = speed;
    Serial.print("motors: left duty=");
    Serial.println(speed);
}

static void turnRight(uint8_t speed) {
    setLeft(speed);
    setRight(-speed);
    motor_state = MotorState::TURN_RIGHT;
    motor_speed = speed;
    Serial.print("motors: right duty=");
    Serial.println(speed);
}

// ==================== Command dispatch ====================
static void handleCommand(const std::string &line) {
    // Parse "<cmd> <value>"
    std::string cmd;
    int value = -1;
    size_t sp = line.find(' ');
    if (sp == std::string::npos) {
        cmd = line;
    } else {
        cmd = line.substr(0, sp);
        value = atoi(line.c_str() + sp + 1);
    }

    if (cmd == "help") {
        Serial.println("Commands: fwd <0..255> | back <0..255> | left <0..255> | right <0..255> | stop | status");
    } else if (cmd == "fwd") {
        if (value >= 0 && value <= 255) moveForward((uint8_t)value);
        else Serial.println("err: value 0..255");
    } else if (cmd == "back") {
        if (value >= 0 && value <= 255) moveBackward((uint8_t)value);
        else Serial.println("err: value 0..255");
    } else if (cmd == "left") {
        if (value >= 0 && value <= 255) turnLeft((uint8_t)value);
        else Serial.println("err: value 0..255");
    } else if (cmd == "right") {
        if (value >= 0 && value <= 255) turnRight((uint8_t)value);
        else Serial.println("err: value 0..255");
    } else if (cmd == "stop") {
        stopMotors();
    } else if (cmd == "status") {
        Serial.printf("state=%u duty=%u\n", (unsigned)motor_state, motor_speed);
    } else if (!cmd.empty()) {
        Serial.print("unknown: ");
        Serial.println(cmd.c_str());
    }
}

// ==================== Setup & Loop ====================
void setup() {
    Serial.begin(115200);
    pinMode(PIN_M_L_EN, OUTPUT);
    pinMode(PIN_M_L_IN1, OUTPUT);
    pinMode(PIN_M_L_IN2, OUTPUT);
    pinMode(PIN_M_R_EN, OUTPUT);
    pinMode(PIN_M_R_IN1, OUTPUT);
    pinMode(PIN_M_R_IN2, OUTPUT);
    ledcSetup(PWM_CH_L, PWM_FREQ, PWM_RES_BITS);
    ledcSetup(PWM_CH_R, PWM_FREQ, PWM_RES_BITS);
    ledcAttachPin(PIN_M_L_EN, PWM_CH_L);
    ledcAttachPin(PIN_M_R_EN, PWM_CH_R);
    stopMotors();
    Serial.println("=== L17 Homework - Motor control + PWM ===");
    Serial.println("Type 'help' for commands.");
}

void loop() {
    // Non-blocking UART line reader
    static std::string line_buf;
    while (Serial.available()) {
        char ch = static_cast<char>(Serial.read());
        if (ch == '\n' || ch == '\r') {
            if (!line_buf.empty()) {
                handleCommand(line_buf);
                line_buf.clear();
            }
        } else if (ch == 0x7F || ch == '\b') {
            if (!line_buf.empty()) line_buf.pop_back();
        } else if (line_buf.length() < 63) {
            line_buf.push_back(ch);
        }
    }

    // Heartbeat: nothing to do here, motors hold their state
    static uint32_t last_hb = 0;
    if (millis() - last_hb >= 1000) {
        last_hb = millis();
        // Optional: print state every second
    }
}
