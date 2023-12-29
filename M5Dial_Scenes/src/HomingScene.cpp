// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class HomingScene : public Scene {
private:
    int _current_button = 0;

public:
    HomingScene() : Scene("Home") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle || state == Alarm) {
            String line = "$H";
            if (_current_button != 0) {
                line += axisNumToString(_current_button - 1);
            }
            log_msg(line);
            send_line(line);
        }
    }
    void onRedButtonPress() {
        if (state == Homing) {
            fnc_realtime(Reset);
        }
    }
    void onTouchRelease(int x, int y) {
        rotateNumberLoop(_current_button, 1, 0, 3);
        reDisplay();
    }
    void reDisplay() {
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        drawStatus();

        int x      = 50;
        int y      = 65;
        int width  = display.width() - (x * 2);
        int height = 32;

        Stripe button(x, y, width, height, SMALL);
        button.draw("Home All", _current_button == 0);
        y = button.y();  // LEDs start with the Home X button
        button.draw("Home X", _current_button == 1);
        button.draw("Home Y", _current_button == 2);
        button.draw("Home Z", _current_button == 3);

        LED led(x - 16, y + height / 2, 10, button.gap());
        led.draw(myLimitSwitches[0]);
        led.draw(myLimitSwitches[1]);
        led.draw(myLimitSwitches[2]);

        String redLabel, grnLabel, orangeLabel = "";
        if (state == Homing) {
            redLabel = "E-Stop";
        } else {
            grnLabel = "Home ";
            grnLabel += _current_button ? axisNumToString(_current_button - 1) : "All";
        }

        drawButtonLegends(redLabel, grnLabel, "Back");

        refreshDisplay();
    }
};
HomingScene homingScene;
