// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "System.h"

M5Canvas           canvas(&M5Dial.Display);
M5GFX&             display = M5Dial.Display;
m5::Speaker_Class& speaker = M5Dial.Speaker;
m5::Touch_Class&   touch   = M5Dial.Touch;
ENCODER&           encoder = M5Dial.Encoder;

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

    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
        debugPort.println("LittleFS Mount Failed");
        return;
    }
    debugPort.println("LittleFS Mounted");
}
