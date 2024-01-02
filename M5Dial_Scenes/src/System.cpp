// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "System.h"

M5Canvas           canvas(&M5Dial.Display);
M5GFX&             display = M5Dial.Display;
m5::Speaker_Class& speaker = M5Dial.Speaker;
m5::Touch_Class&   touch   = M5Dial.Touch;

Stream& debugPort = USBSerial;

// Helpful for debugging touch development.
String M5TouchStateName(m5::touch_state_t state_num) {
    static constexpr const char* state_name[16] = { "none", "touch", "touch_end", "touch_begin", "___", "hold", "hold_end", "hold_begin",
                                                    "___",  "flick", "flick_end", "flick_begin", "___", "drag", "drag_end", "drag_begin" };

    return String(state_name[state_num]);
}

#define FORMAT_LITTLEFS_IF_FAILED true

void init_system() {
    USBSerial.begin(921600);

    init_encoder();

    auto cfg = M5.config();
    // Don't enable the encoder because M5's encoder driver is flaky
    M5Dial.begin(cfg, false, false);
    touch.setFlickThresh(30);

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        log_println("LittleFS Mount Failed");
        return;
    }
    log_println("LittleFS Mounted");
    canvas.createSprite(display.width(), display.height());
}

void log_write(uint8_t c) {
#ifdef DEBUG_TO_USB
    if (debugPort.availableForWrite() > 1) {
        debugPort.write(c);
    }
#endif
}

void log_print(const String& s) {
#ifdef DEBUG_TO_FNC
    extern void send_line(const String& s, int timeout = 2000);
    send_line("$Msg/Uart0=" + s);
#endif
#ifdef DEBUG_TO_USB
    if (debugPort.availableForWrite() > s.length()) {
        debugPort.println(s);
    }
#endif
}

void log_println(const String& s) {
    log_print(s + "\r\n");
}
