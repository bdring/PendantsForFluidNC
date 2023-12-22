// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "menu_img.h"

class MenuScene : public Scene {
private:
    int current_button = 0;

public:
    MenuScene() : Scene("Menu") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        
    }
    void onRedButtonPress() {
        
    }
    void onTouchRelease(m5::touch_detail_t t) {
        
    }
    void display() {
        drawBackground(BLACK);

        canvas.pushImage(0, 0, 240, 240, menu_img);
        drawThickCircle(120, 33, 29, 4, WHITE);        
        canvas.fillCircle(120, 71, 6, WHITE);
        centered_text("Homing", 130, WHITE, SMALL);

        refreshDisplay();
    }
};
MenuScene menuScene;
