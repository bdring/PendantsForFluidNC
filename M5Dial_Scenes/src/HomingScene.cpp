// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class HomingScene : public Scene {
private:
    int current_button = 0;

public:
    HomingScene() : Scene("Home") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle || state == Alarm) {
            if (current_button == 0) {
                send_line("$H");
            } else {
                send_line("$H" + axisNumToString(current_button - 1));
            }
        }
    }
    void onRedButtonPress() {
        if (state == Homing) {
            fnc_realtime(Reset);
        }
    }
    void onTouchRelease(m5::touch_detail_t t) {
        rotateNumberLoop(current_button, 1, 0, 3);
        USBSerial.printf("%s\r\n", M5TouchStateName(t.state));
        display();
    }
    void display() {
        canvas.fillSprite(BLACK);
        drawStatus();

        int x      = 50;
        int y      = 65;
        int gap    = 34;
        int width  = 140;
        int height = 30;
        drawButton(x, y, width, height, 12, "Home All", current_button == 0);
        drawButton(x, y += gap, width, height, 12, "Home X", current_button == 1);
        drawLed(x - 16, y + 15, 10, myLimitSwitches[0]);
        drawButton(x, y += gap, width, height, 12, "Home Y", current_button == 2);
        drawLed(x - 16, y + 15, 10, myLimitSwitches[1]);
        drawButton(x, y += gap, width, height, 12, "Home Z", current_button == 3);
        drawLed(x - 16, y + 15, 10, myLimitSwitches[2]);

        menuTitle();

        String redLabel, grnLabel, orangeLabel = "";
        if (state == Homing) {
            redLabel == "E-Stop";
        } else {
            grnLabel = "Home ";
            grnLabel += current_button ? axisNumToString(current_button - 1) : "All";
        }

        buttonLegends(redLabel, grnLabel, "Main");

        refreshDisplaySprite();
    }
};
HomingScene homingScene;
