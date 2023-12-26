// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class ControlScene : public Scene {
private:
    int current_button = 0;

public:
    ControlScene() : Scene("Control") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle) {
            String line = "";
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
            log_msg(line);
            send_line(line);
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
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        drawStatus();

        int x      = 50;
        int y      = 65;
        int width  = WIDTH - (x * 2);
        int height = 32;

        Stripe button(x, y, width, height, SMALL);
        button.draw("Flood", current_button == 0);
        y = button.y();  // LEDs start with the Home X button
        button.draw("Mist", current_button == 1);
        button.draw("G28", current_button == 2);
        button.draw("G30", current_button == 3);

        String redLabel, grnLabel, orangeLabel = "";
        if (state == Homing) {
            //redLabel = "E-Stop";
        } else {
            // grnLabel = "Home ";
            //grnLabel += current_button ? axisNumToString(current_button - 1) : "All";
        }
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
        drawButtonLegends(redLabel, grnLabel, "Back");
        showError();
        refreshDisplay();
    }
};
ControlScene controlScene;
