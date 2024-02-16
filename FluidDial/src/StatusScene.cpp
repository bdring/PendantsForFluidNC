// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

class StatusScene : public Scene {
private:
    int menu_item = 0;

public:
    StatusScene() : Scene("Status") {}

    void onDialButtonPress() { pop_scene(); }

    void onTouchRelease(int x, int y) {
        fnc_realtime(StatusReport);  // sometimes you want an extra status
    }

    void onRedButtonPress() {
        switch (state) {
            case Alarm:
                if (lastAlarm == 14) {
                    send_line("$X");
                } else {
                    fnc_realtime(Reset);
                }
                break;
            case Cycle:
            case Homing:
            case Hold:
                fnc_realtime(Reset);
                break;
        }
    }

    void onGreenButtonPress() {
        switch (state) {
            case Cycle:
                fnc_realtime(FeedHold);
                break;
            case Hold:
                fnc_realtime(CycleStart);
                break;
            case Alarm:
                send_line("$H");
                break;
        }
        fnc_realtime(StatusReport);
    }

    void onEncoder(int delta) {
        if (state == Cycle) {
            if (delta > 0 && myFro < 200) {
                fnc_realtime(FeedOvrFinePlus);
            } else if (delta < 0 && myFro > 10) {
                fnc_realtime(FeedOvrFineMinus);
            }
            reDisplay();
        }
    }

    void onDROChange() { reDisplay(); }
    void onLimitsChange() { reDisplay(); }

    void reDisplay() {
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        drawStatus();

        const char* grnText = "";
        const char* redText = "";

        DRO dro(10, 68, 220, 32);
        dro.draw(0, false);
        dro.draw(1, false);
        dro.draw(2, false);

        int y = 170;
        if (state == Cycle || state == Hold) {
            int width  = 192;
            int height = 10;
            if (myPercent > 0) {
                drawRect(20, y, width, height, 5, LIGHTGREY);
                width = (float)width * myPercent / 100.0;
                if (width > 0) {
                    drawRect(20, y, width, height, 5, GREEN);
                }
            }
            // Feed override
            char legend[50];
            sprintf(legend, "Feed Rate Ovr:%d%%", myFro);
            centered_text(legend, y + 23);
        }

        const char* encoder_button_text = "Menu";

        switch (state) {
            case Alarm:
                if (lastAlarm == 14) {
                    redText = "Unlock";
                } else {
                    redText = "Reset";
                }
                grnText = "Home All";
                break;
            case Homing:
                redText = "Reset";
                break;
            case Cycle:
                redText = "E-Stop";
                grnText = "Hold";
                break;
            case Hold:
                redText = "Quit";
                grnText = "Resume";
                break;
            case Jog:
                redText = "Jog Cancel";
                break;
            case Idle:
                break;
        }

        drawButtonLegends(redText, grnText, "Back");
        refreshDisplay();
    }
};
StatusScene statusScene;
