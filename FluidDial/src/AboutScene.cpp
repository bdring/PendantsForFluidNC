// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

extern Scene menuScene;
extern const char* version_info;  // auto generated version.cpp

class AboutScene : public Scene {
private:
public:
    AboutScene() : Scene("About") {}

    void onEntry() {}

    void onDialButtonPress() { activate_scene(&menuScene); }
    void onGreenButtonPress() {}
    void onRedButtonPress() {}

    void onTouchClick() override {
        fnc_realtime(StatusReport);
        if (state == Idle) {
            send_line("$G");
        }
    }

    void onEncoder(int delta) {}
    void onStateChange(state_t old_state) { reDisplay(); }
    void reDisplay() {
        background();
        drawStatus();

        const int key_x = 118;
        const int val_x = 122;
        const int y_spacing = 20;
        int y     = 90;
        

        text("Version:", key_x, y, LIGHTGREY, TINY, bottom_right);
        text(version_info, val_x, y, GREEN, TINY, bottom_left);

        text("FNC baud:", key_x, y += y_spacing, LIGHTGREY, TINY, bottom_right);
        text(intToCStr(FNC_BAUD), val_x, y, GREEN, TINY, bottom_left);

#ifdef DEBUG_TO_USB
        text("Debug baud:", key_x, y += y_spacing, LIGHTGREY, TINY, bottom_right);
        text(intToCStr(USB_BAUD), val_x, y, GREEN, TINY, bottom_left);
#endif

        drawMenuTitle(current_scene->name());
        drawButtonLegends("", "", "Menu");
        drawError();  // if there is one
        refreshDisplay();
    }
};
AboutScene aboutScene;
