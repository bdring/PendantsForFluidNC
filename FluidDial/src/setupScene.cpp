// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

extern Scene menuScene;

class SetupScene : public Scene {
private:
public:
    SetupScene() : Scene("About") {}

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

        centered_text("Credits:", 118, LIGHTGREY, TINY);
        centered_text("@bdring", 140, GREEN, TINY);
        centered_text("@MitchBradley", 160, GREEN, TINY);
        centered_text("@bDuthieDev ", 180, GREEN, TINY);
        centered_text("@Design8Studio", 200, GREEN, TINY);

        drawMenuTitle(current_scene->name());
        drawButtonLegends("", "", "Menu");
        drawError();  // if there is one
        refreshDisplay();
    }
};
SetupScene setupScene;
