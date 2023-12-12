#include <Arduino.h>
#include "GrblParserC.h"

extern "C" void fnc_putchar(uint8_t c) {
    Serial1.write(c);
}

extern "C" int fnc_getchar() {
    if (Serial1.available()) {
        return Serial1.read();
    }
    return -1;
}

extern "C" void debug_putchar(char c) {
    Serial.print(c);
}
extern "C" void debug_print(const char* msg) {
    Serial.print(msg);
}
extern "C" void debug_println(const char* msg) {
    Serial.println(msg);
}

extern "C" void show_state(const char* state) {
    Serial.print(state);
}
extern "C" void show_dro(const pos_t* axes, const pos_t* wcos, bool isMpos, bool* limits, size_t n_axis) {
    char delim = ' ';
    for (size_t i = 0; i < n_axis; i++) {
        debug_putchar(delim);
        delim = ',';
        String a(axes[i]);
        debug_print(a.c_str());
    }
    debug_println("");
}
extern "C" void end_status_report() {
    debug_println("");
}
extern "C" int milliseconds() {
    return millis();
}
extern "C" void poll_extra() {
#ifdef SEND_CONSOLE_DATA
    while (Serial.available()) {
        char c = Serial.read();
        if (c != '\r') {
            fnc_putchar(c);
        }
    }
#endif
}

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    fnc_wait_ready();
    fnc_putchar('?');           // Initial status report
    fnc_send_line("$G", 1000);  // Initial modes report
}
// #define ECHO_RX_DATA
// #define SEND_CONSOLE_DATA
void loop() {
    fnc_poll();
}
