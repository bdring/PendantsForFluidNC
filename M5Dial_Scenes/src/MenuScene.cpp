// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "menu_img.h"

// these should not be here...move
extern Scene fileScene;
extern Scene controlScene;
extern Scene mainScene;
extern Scene probingScene;
extern Scene homingScene;
extern Scene joggingScene;
extern Scene setupScene;

class MenuScene : public Scene {
private:
    int current_button = 2;

    int encoderCounter = 0;

    struct menuball {
        String label;
        int    x;
        int    y;
    };

    menuball balls[8];

    String menuLabels[8] = { "Setup", "Files", "Main", "Macros", "Off", "Probe", "Home", "Jog" };

    float _ballOffsetRadius = 87.0;

public:
    MenuScene() : Scene("Menu") {
        balls[0].label = "Homing";
        balls[1].label = "Homing";
    }

    void onDialButtonPress() {
        switch (current_button) {
            case 0:
                activate_scene(&setupScene);
                break;
            case 1:
                activate_scene(&fileScene);
                break;
            case 2:
                activate_scene(&mainScene);
                break;
            case 3:
                activate_scene(&controlScene);
                break;
            case 4:
                // off
                //M5Dial.Power.(GPIO_NUM_42);//
                delay(3000);
                M5Dial.Power.deepSleep(1000000, true);
                break;
            case 5:
                activate_scene(&probingScene);
                break;
            case 6:
                activate_scene(&homingScene);
                break;
            case 7:
                activate_scene(&joggingScene);
                break;
        }
    }
    void onGreenButtonPress() {}
    void onRedButtonPress() {}
    void onTouchRelease(m5::touch_detail_t t) {}
    void onEncoder(int delta) {
        if (abs(delta) > 0) {
            encoderCounter += delta;
            if (encoderCounter % 4 == 0) {
                rotateNumberLoop(current_button, delta > 0 ? 1 : -1, 0, 7);
                display();
            }
        }
    }
    void display() {
        drawBackground(BLACK);

        int x, y;

        x = cosf(0.785 * (float)current_button) * _ballOffsetRadius + 120;
        y = sinf(0.785 * (float)current_button) * _ballOffsetRadius + 120;

        canvas.pushImage(0, 0, 240, 240, menu_img);
        drawThickCircle(x, y, 29, 4, WHITE);
        //canvas.fillCircle(120, 71, 6, WHITE);
        centered_text(menuLabels[current_button], 120, WHITE, SMALL);

        refreshDisplay();
    }
};
MenuScene menuScene;
