#include <Arduino.h>
#include "GrblParser.h"

class Displayer : public GrblParser {
    void show_state(const String& state) { Serial.print(state); }
    void show_dro(const float* axes, bool isMpos, bool* limits) {
        char delim = ' ';
        for (int i = 0; i < _n_axis; i++) {
            Serial.print(delim);
            delim = ',';
            Serial.print(axes[i]);
        }
    }
    void end_status_report() { Serial.println(); }
    int  getchar() {
        if (Serial1.available()) {
            return Serial1.read();
        }
        return -1;
    }
    int  milliseconds() { return millis(); }
    void poll_extra() {
#ifdef SEND_CONSOLE_DATA
        while (Serial.available()) {
            char c = Serial.read();
            if (c != '\r') {
                Serial1.write(c);
            }
        }
#endif
    }

public:
    void putchar(uint8_t c) { Serial1.write(c); }

} displayer;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    displayer.wait_ready();
    displayer.putchar("?");     // Initial status report
    displayer.send_line("$G");  // Initial modes report
}
// #define ECHO_RX_DATA
// #define SEND_CONSOLE_DATA
void loop() {
    displayer.poll();
}
