// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "menu_img.h"

extern Scene menuScene;

class SetupScene : public Scene {
private:
public:
    SetupScene() : Scene("Setup") {}

    void init() { log_println("init"); }

    void onDialButtonPress() { activate_scene(&menuScene); }
    void onGreenButtonPress() {}
    void onRedButtonPress() {}

    void onTouchRelease(m5::touch_detail_t t) {
        fnc_realtime(StatusReport);
        send_line("$G");
    }

    void onEncoder(int delta) {}
    void onStateChange(state_t state) { reDisplay(); }
    void reDisplay() {
        drawBackground(BLACK);
        drawStatus();

        centered_text("GCode modes:", 73, LIGHTGREY, TINY);
        centered_text(modeString(), 91, GREEN, TINY);

        centered_text("Credits:", 118, LIGHTGREY, TINY);
        centered_text("@bdring", 140, GREEN, TINY);
        centered_text("@MitchBradley", 160, GREEN, TINY);
        centered_text("@bDuthieDev ", 180, GREEN, TINY);
        centered_text("@Design8Studio", 200, GREEN, TINY);

        drawMenuTitle(current_scene->name());
        drawButtonLegends("", "", "Menu");
        showError();  // if there is one
        refreshDisplay();
    }
};
SetupScene setupScene;
