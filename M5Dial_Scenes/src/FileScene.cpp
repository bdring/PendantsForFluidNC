// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "menu_img.h"

class FileScene : public Scene {
private:
    int current_file   = 2;
    int encoderCounter = 0;

    String filenames[8] = { "Short", "Longer", "Very Very Long", "Hello", "World", "Test", "Foo", "Jog" };

public:
    FileScene() : Scene("Files") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {}
    void onRedButtonPress() {}
    void onTouchRelease(m5::touch_detail_t t) {}
    void onEncoder(int delta) {
        if (abs(delta) > 0) {
            encoderCounter += delta;
            if (encoderCounter % 4 == 0) {
                if (delta > 0) {
                    if (current_file < 7) {
                        current_file++;
                    }
                } else {
                    if (current_file > 0) {
                        current_file--;
                    }
                }
                display();
            }
        }
    }
    void display() {
        drawBackground(BLACK);

        String fn = "";

        //top one

        if (current_file < 7) {
            canvas.fillRoundRect(30, 40, 165, 45, 14, DARKGREY);
            canvas.fillRoundRect(60, 43, 40, 13, 4, DARKGREEN);
            fn = filenames[current_file + 1];
            if (fn.length() > 18) {
                fn = fn.substring(0, 14) + "...";
            }
            text(fn, 110, 72, BLACK, TINY, middle_center);
        }

        // bottom one
        //top one

        if (current_file > 0) {
            canvas.fillRoundRect(30, 155, 165, 45, 14, DARKGREY);
            canvas.fillRoundRect(60, 158, 40, 13, 4, DARKGREEN);
            fn = filenames[current_file - 1];
            if (fn.length() > 18) {
                fn = fn.substring(0, 14) + "...";
            }
            text(fn, 110, 187, BLACK, TINY, middle_center);
        }

        // progressbar
        for (int i = 0; i < 6; i++) {
            canvas.drawArc(120, 120, 118 - i, 115 - i, -50, 50, DARKGREY);
        }

        float mx  = 1.745;
        float s   = mx / -2.0;
        float inc = mx / 8.0;

        int x = cosf(s + inc * current_file) * 114.0;
        int y = sinf(s + inc * current_file) * 114.0;

        canvas.fillCircle(x + 120, y+ 120, 5, LIGHTGREY);

        // middle one
        canvas.fillRoundRect(5, 90, 214, 60, 20, LIGHTGREY);
        canvas.fillRoundRect(38, 94, 60, 28, 4, DARKGREEN);
        text("gcode", 42, 109, WHITE, TINY, middle_left);
        text("14.234kB", 110, 109, BLACK, TINY, middle_left);
        fn = filenames[current_file];
        text(fn, 110, 133, BLACK, TINY, middle_center);

        drawMenuTitle(current_scene->name());
        drawButtonLegends("Back", "Sel", "Menu");
        refreshDisplay();
    }
};
FileScene fileScene;
