// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

class ControlScene : public Scene {
private:
    int current_button = 0;

public:
    ControlScene() : Scene("Control") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        const char* line = "";
        switch (state) {
            case Idle:
                switch (current_button) {
                    case 0:
                        line = "M7";
                        break;
                    case 1:
                        line = "M8";
                        break;
                    case 2:
                        line = "G28";
                        break;
                    case 3:
                        line = "G30";
                        break;
                    default:
                        break;
                }
                send_line(line);
                break;
            case Cycle:
                fnc_realtime(FeedHold);
                break;
            case Hold:
                fnc_realtime(CycleStart);
                break;
        }
    }
    void onRedButtonPress() {
        switch (state) {
            case Cycle:
            case Hold:
                fnc_realtime(Reset);
                break;
        }
    }

    void onTouchRelease(int x, int y) {
        // Rotate through the items to be adjusted.
        rotateNumberLoop(current_button, 1, 0, 3);
        reDisplay();
        ackBeep();
    }

    void onDROChange() { reDisplay(); }

    void reDisplay() {
        background();
        drawMenuTitle(current_scene->name());
        drawStatus();

        const char* redLabel = "";
        const char* grnLabel = "";

        if (state == Idle) {
            int x      = 50;
            int y      = 65;
            int width  = 240 - (x * 2);
            int height = 32;

            Stripe button(x, y, width, height, SMALL);
            button.draw("Flood", current_button == 0);
            y = button.y();  // LEDs start with the Home X button
            button.draw("Mist", current_button == 1);
            button.draw("G28", current_button == 2);
            button.draw("G30", current_button == 3);

            switch (current_button) {
                case 0:
                    grnLabel = "Flood";
                    break;
                case 1:
                    grnLabel = "Mist";
                    break;
                case 2:
                    grnLabel = "G28";
                    break;
                case 3:
                    grnLabel = "G30";
                    break;
                default:
                    grnLabel = "";
                    break;
            }
        } else {
            centered_text("Invalid State", 105, WHITE, MEDIUM);
            centered_text("For Controls", 145, WHITE, MEDIUM);

            if (state == Cycle) {
                redLabel = "E-Stop";
                grnLabel = "Hold";
            } else if (state == Hold) {
                redLabel = "Reset";
                grnLabel = "Resume";
            }
        }

        drawButtonLegends(redLabel, grnLabel, "Back");
        drawError();
        refreshDisplay();
    }
};
ControlScene controlScene;
