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

    void onDialButtonPress() { activate_scene(&menuScene); }
    void onGreenButtonPress() {}
    void onRedButtonPress() {}
    void onTouchRelease(m5::touch_detail_t t) {}
    void onEncoder(int delta) {}
    void display() {
        drawBackground(BLACK);

        centered_text("Coming soon...", 120, WHITE, MEDIUM);

        drawMenuTitle(current_scene->name());
        drawButtonLegends("", "", "Menu");
        showError();  // if there is one
        refreshDisplay();
    }
};
SetupScene setupScene;
