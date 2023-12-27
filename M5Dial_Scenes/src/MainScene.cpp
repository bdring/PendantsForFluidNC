// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

extern Scene probingScene;
extern Scene homingScene;
extern Scene joggingScene;

class MainScene : public Scene {
private:
    int menu_item = 0;

public:
    MainScene() : Scene("Main") {}

    void onDialButtonPress() {
        if (state == Idle || state == Alarm) {
            push_scene(&joggingScene);
        } else if (state == Cycle) {
            fnc_realtime(FeedOvrReset);
        }
    }

    void onTouchRelease(int x, int y) {
        if (state == Cycle) {
            rotateNumberLoop(menu_item, 1, 0, 2);
        }
        fnc_realtime(StatusReport);  // sometimes you want an extra status
    }

    void onRedButtonPress() {
        switch (state) {
            case Alarm:
                send_line("$X");
                break;
            case Cycle:
            case Homing:
            case Hold:
                fnc_realtime(Reset);
                break;
            case Idle:
                push_scene(&probingScene);
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
            case Idle:
            case Alarm:
                push_scene(&homingScene);
                return;  // no status report
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

        DRO dro(10, 68, 220, 32);
        dro.draw(0, false);
        dro.draw(1, false);
        dro.draw(2, false);

        int y = 170;
        if (state == Cycle || state == Hold) {
            int width = 192;
            if (myPercent > 0) {
                canvas.fillRoundRect(20, y, width, 10, 5, LIGHTGREY);
                width = (float)width * myPercent / 100.0;
                if (width > 0) {
                    canvas.fillRoundRect(20, y, width, 10, 5, GREEN);
                }
            }

            // Feed override
            centered_text("Feed Rate Ovr:" + String(myFro) + "%", y + 23);
        }

        String encoder_button_text = "";
        switch (menu_item) {
            case 0:
                encoder_button_text = "Jog";
                break;
            case 1:
                encoder_button_text = "Home";
                break;
            case 2:
                encoder_button_text = "Probe";
                break;
        }
        String redButtonText   = "";
        String greenButtonText = "";
        switch (state) {
            case Alarm:
            case Homing:
                drawButtonLegends("Reset", "Home", encoder_button_text);
                break;
            case Cycle:
                drawButtonLegends("E-Stop", "Hold", "FRO End");
                break;
            case Hold:
                drawButtonLegends("Quit", "Start", encoder_button_text);
                break;
            case Jog:
                drawButtonLegends("Jog Cancel", "", encoder_button_text);
                break;
            case Idle:
                drawButtonLegends("Probe", "Home", encoder_button_text);
                break;
        }

        refreshDisplay();
    }
};
MainScene mainScene;
