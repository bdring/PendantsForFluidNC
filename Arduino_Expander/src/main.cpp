#include <Arduino.h>
#include "Expander.h"
#include "gpio_pin.h"
#include "gpiomap.h"

#define FNCSerial Serial  // connects STM32 to ESP32 and FNC
#ifdef USE_DEBUG_SERIAL
#    define DebugSerial Serial2  // connects STM32 to Debug terminal
#endif

// Interface routines for GrblParser

// Receive a byte from the serial port connected to FluidNC
int fnc_getchar() {
    if (FNCSerial.available()) {
        return FNCSerial.read();
    }
    return -1;
}
// Send a byte to the serial port connected to FluidNC
void fnc_putchar(uint8_t c) {
    FNCSerial.write(c);
}

// Return a value that increments every millisecond
int milliseconds() {
    return millis();
}

// Perform extra operations after the normal polling for input from FluidNC
void poll_extra() {
#ifdef DebugSerial
    while (DebugSerial.available()) {
        uint8_t c = DebugSerial.read();
        fnc_putchar(c);
        collect(c);  // for testing from debug terminal
    }
#endif

    expander_poll();
}

void setup() {
    init_gpiomap();

#ifdef LEDPBUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
#endif
    FNCSerial.begin(115200);
#ifdef DebugSerial
    DebugSerial.begin(115200);
#endif
    fnc_wait_ready();  // Synchronize to FluidNC
}
void loop() {
    fnc_poll();
}
