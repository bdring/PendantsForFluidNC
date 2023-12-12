#include <Arduino.h>
#include "GrblParserC.h"

#define FNCSerial Serial

// Cycle through the files named in this list
const char* playlist[] = { "file1.nc", "file2.nc", "file3.nc" };
size_t      next_file  = 0;

void start_next_run() {
    if (next_file == sizeof(playlist) / sizeof(*playlist)) {
        next_file = 0;
    }
    String msg("$SD/Run=");
    msg += playlist[next_file++];
    fnc_send_line(msg.c_str(), 1000);
}

bool   program_sent  = false;
String last_state    = "";
String last_filename = "";

extern "C" void show_state(const char* state) {
    String this_state = state;
    // Wait until the program has been sent and the state changes
    // from Run to Idle.  After the program has been sent, it
    // usually takes some time before its last few lines finish running.
    if (program_sent && this_state == "Idle" && last_state == "Run") {
        start_next_run();
        program_sent = false;
    }
    last_state = this_state;
}

extern "C" void show_file(const char* filename, file_percent_t percent) {
    // When we stop seeing SD: fields in the status report
    // after having just seen one, we know that the file has
    // been sent.  It might still be running though, since
    // GCode commands can take awhile to finish.
    if (last_filename != "" && *filename == '\0') {
        program_sent = true;
    }
    last_filename = filename;
}
extern "C" int fnc_getchar() {
    if (FNCSerial.available()) {
        return FNCSerial.read();
    }
    return -1;
}
int milliseconds() {
    return millis();
}

extern "C" void fnc_putchar(uint8_t c) {
    FNCSerial.write(c);
}

void setup() {
    FNCSerial.begin(115200);
    fnc_wait_ready();
    fnc_putchar('?');  // Initial status report
    start_next_run();
}
void loop() {
    fnc_poll();
}
