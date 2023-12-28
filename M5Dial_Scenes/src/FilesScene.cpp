// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include "menu_img.h"

extern Scene mainScene;

class FilesScene : public Scene {
private:
    int current_file   = 2;
    int encoderCounter = 0;
    int animationFrame = 0;

    String filenames[8] = { "Pendant_Top", "Pendant_Case", "Knob", "Cord_Clip", "Screw_Box", "End_Cap", "Foot", "Post" };

public:
    FilesScene() : Scene("Files") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        for (int i = 0; i < 10; i++) {
            animationFrame = i;
            delay(6);
            //M5Dial.Speaker.tone(2000, 20);
            display();
        }
    }
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

                for (int i = 0; i < 10; i++) {
                    animationFrame = i;
                    delay(10);
                    //M5Dial.Speaker.tone(2000, 20);
                    display();
                }
            }
        }
    }
    void display() {
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        centered_text("SD Card", 30, WHITE, TINY);

        String fn = "Long File Name";
        int    x, y;
        y = 50 + (animationFrame * 7);

        if (animationFrame == 9) {
            drawCapsule(y, 230, 70, 0x1b12);
        }
        centered_text(fn, y, DARKGREY, (animationFrame == 9) ? SMALL : TINY);
        y = 120 + (animationFrame * 7);
        centered_text(fn, y, DARKGREY, TINY);

        if (animationFrame == 9) {
            centered_text(fn, 50, DARKGREY, TINY);
        }

        drawButtonLegends("", (animationFrame == 9) ? "Run" : "", "Menu");
        showError();  // if there is one
        refreshDisplay();
    }
};
FilesScene filesScene;
