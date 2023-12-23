// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "menu_img.h"

class FileScene : public Scene {
private:
    int current_file   = 2;
    int encoderCounter = 0;

    String filenames[8] = { "Setup", "Files", "Run", "Macros", "Off", "Probe", "Home", "Jog" };

public:
    FileScene() : Scene("Menu") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {}
    void onRedButtonPress() {}
    void onTouchRelease(m5::touch_detail_t t) {}
    void onEncoder(int delta) {
        if (abs(delta) > 0) {
            encoderCounter += delta;
            if (encoderCounter % 4 == 0) {
                rotateNumberLoop(current_file, delta > 0 ? 1 : -1, 0, 7);
                display();
            }
        }
    }
    void display() {
        drawBackground(BLACK);


        //top one
        
        canvas.fillRoundRect(30, 40, 165, 45, 15, LIGHTGREY);

        // bottom one
        //top one
        canvas.fillRoundRect(30, 155, 165, 45, 15, LIGHTGREY);

        canvas.drawArc(120, 120, 118, 118, -40, 40, WHITE);

        // middle one
        canvas.fillRoundRect(5, 90, 214, 60, 20, LIGHTGREY);
        canvas.fillRoundRect(38, 94, 60, 28, 4, DARKGREEN);
        text("gcode", 42, 109, WHITE, TINY, middle_left);
        text("14.234kB", 110, 109, BLACK, TINY, middle_left);
        text("verrry_looong_filename", 110, 133, BLACK, TINY, middle_center);


        drawMenuTitle(current_scene->name());
        drawButtonLegends("Back", "Menu", "Sel");
        refreshDisplay();
    }
};
FileScene fileScene;
