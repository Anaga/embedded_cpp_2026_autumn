// Lesson 12 - USB <-> UART bridge
// Raspberry Pi Pico (RP2040), Arduino-Pico core
//
// Everything you type in the serial monitor goes out on UART0 to your
// partner's Pico. Everything your partner's Pico sends arrives on UART0 and
// is shown in your monitor.
//
//   Serial  = USB to your PC (not a UART, the baud rate is ignored)
//   Serial1 = UART0, GP0 = TX (pin 1), GP1 = RX (pin 2)
//
// Wiring between two boards: TX -> RX, RX <- TX, GND - GND (pin 3).
// Never connect 3V3 or VBUS between the boards.

#include <Arduino.h>

static const uint32_t USB_BAUD      = 115200U;  // ignored by USB
static const uint32_t UART_BAUD     = 115200U;  // must be the same on both boards!
static const uint32_t USB_SETTLE_MS = 2000U;    // give the USB serial port time to open

void setup() {
    Serial.begin(USB_BAUD);      // USB to your PC
    Serial1.begin(UART_BAUD);    // UART0: GP0 TX, GP1 RX

    delay(USB_SETTLE_MS);
    Serial.println("Lesson 12 - UART bridge. Type a line and press Enter.");
}

void loop() {
    while (Serial.available() > 0) {     // PC -> partner
        Serial1.write(static_cast<uint8_t>(Serial.read()));
    }
    while (Serial1.available() > 0) {    // partner -> PC
        Serial.write(static_cast<uint8_t>(Serial1.read()));
    }
}
