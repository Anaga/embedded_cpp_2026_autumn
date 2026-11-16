/*
 * Homework 16 - UART commands and non-blocking I/O.
 *
 * A UART (USB CDC) command interface. All timing is non-blocking (millis).
 * Commands:
 *   id        - print device id
 *   help      - print command list
 *   led on/off/status - control the status LED
 *   sensor    - print latest readings (simulated)
 *   time      - print uptime
 *
 * Input is line-buffered: bytes are collected until '\n', then parsed.
 * No delay() anywhere.
 */

#include <Arduino.h>
#include <string>

// ==================== Configuration ====================
static const uint32_t SERIAL_BAUD = 115200;
static const uint32_t HEARTBEAT_MS = 1000;
static const uint8_t PIN_LED = LED_BUILTIN;

// ==================== Non-blocking line reader ====================
static std::string line_buf;

static void handle_command(const std::string &cmd) {
    if (cmd == "help") {
        Serial.println("Available commands:");
        Serial.println("  id          - print device id");
        Serial.println("  help        - print this list");
        Serial.println("  led on      - turn status LED on");
        Serial.println("  led off     - turn status LED off");
        Serial.println("  led status  - print LED state");
        Serial.println("  sensor      - print latest readings");
        Serial.println("  time        - print uptime (ms)");
    } else if (cmd == "id") {
        Serial.println("id: ESP32-L16-HW");
    } else if (cmd == "led on") {
        digitalWrite(PIN_LED, LOW);  // builtin LED active low on most boards
        Serial.println("led on");
    } else if (cmd == "led off") {
        digitalWrite(PIN_LED, HIGH);
        Serial.println("led off");
    } else if (cmd == "led status") {
        Serial.print(digitalRead(PIN_LED) == LOW ? "led on" : "led off");
        Serial.println();
    } else if (cmd == "sensor") {
        Serial.println("sensor: temp=23.5 C hum=45.2 %");
    } else if (cmd == "time") {
        Serial.print("uptime_ms=");
        Serial.println(millis());
    } else if (cmd.length() > 0) {
        Serial.print("unknown: ");
        Serial.println(cmd.c_str());
    }
}

// ==================== Setup & Loop ====================
void setup() {
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(SERIAL_BAUD);
    Serial.println("=== L16 Homework - UART commands + non-blocking I/O ===");
    Serial.println("Type 'help' for commands.");
}

void loop() {
    // Non-blocking serial read: accumulate until newline
    while (Serial.available()) {
        char ch = static_cast<char>(Serial.read());
        // echo back so the terminal shows typed text
        Serial.write(ch);
        if (ch == '\n' || ch == '\r') {
            // line complete (ignore empty lines)
            if (line_buf.length() > 0) {
                handle_command(line_buf);
                line_buf.clear();
            }
        } else if (ch == 0x7F || ch == '\b') { // backspace
            if (line_buf.length() > 0) {
                line_buf.pop_back();
            }
        } else if (line_buf.length() < 255) {
            line_buf.push_back(ch);
        }
    }

    // Heartbeat: blink LED every second (non-blocking)
    static uint32_t last_hb = 0;
    if (millis() - last_hb >= HEARTBEAT_MS) {
        last_hb = millis();
        digitalWrite(PIN_LED, (millis() / HEARTBEAT_MS) % 2 == 0 ? LOW : HIGH);
    }
}
