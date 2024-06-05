#include <Arduino.h>
#include "GrblParserC.h"

#define UART Serial1

extern "C" void show_state(const char* state) {
    digitalWrite(LED_BUILTIN, (strcmp(state, "Run") == 0) ? HIGH : LOW);
}

extern "C" int fnc_getchar() {
    if (UART.available()) {
        return UART.read();
    }
    return -1;
}

int milliseconds() {
    return millis();
}

extern "C" void fnc_putchar(uint8_t c) {
    UART.write(c);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    UART.begin(115200);
    fnc_wait_ready();  // Synchronize to FluidNC
    fnc_putchar('?');  // Initial status report
}
void loop() {
    fnc_poll();
}
