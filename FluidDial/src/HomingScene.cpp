// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

class HomingScene : public Scene {
private:
    int _current_button = 0;

public:
    HomingScene() : Scene("Home") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle || state == Alarm) {
            std::string line = "$H";
            if (_current_button != 0) {
                line += axisNumToChar(_current_button - 1);
            }
            log_println(line);
            send_line(line);
        } else if (state == Cycle) {
            fnc_realtime(FeedHold);
        } else if (state == Hold) {
            fnc_realtime(CycleStart);
        }
    }
    void onRedButtonPress() {
        if (state == Homing) {
            fnc_realtime(Reset);
        }
    }

    void onTouchRelease(int x, int y) {
        if (state == Idle || state == Homing || state == Alarm) {
            rotateNumberLoop(_current_button, 1, 0, 3);
            reDisplay();
            ackBeep();
        }
    }

    void onDROChange() { reDisplay(); }  // also covers any status change

    void reDisplay() {
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        drawStatus();

        const char* redLabel    = "";
        const char* grnLabel    = "";
        const char* orangeLabel = "";
        std::string green       = "Home ";

        if (state == Idle || state == Homing || state == Alarm) {
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

            if (state == Homing) {
                redLabel = "E-Stop";
            } else {
                grnLabel = _current_button ? axisNumToCStr(_current_button - 1) : "All";
                grnLabel = green.c_str();
            }
        } else {
            centered_text("Invalid State", 105, WHITE, MEDIUM);
            centered_text("For Homing", 145, WHITE, MEDIUM);
            redLabel = "E-Stop";
            if (state == Cycle) {
                grnLabel = "Hold";
            } else if (state == Hold) {
                grnLabel = "Resume";
            }
        }

        drawButtonLegends(redLabel, grnLabel, "Back");

        refreshDisplay();
    }
};
HomingScene homingScene;
