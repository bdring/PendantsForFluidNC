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
            pop_scene();
        } else if (state == Cycle) {
            fnc_realtime(FeedOvrReset);
        }
    }

    void onTouchRelease(int x, int y) {
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
            centered_text("Feed Rate Ovr:" + String(myFro) + "%", y + 23);
        }

        String encoder_button_text = "Menu";    

        String redButtonText   = "";
        String greenButtonText = "";
        switch (state) {
            case Alarm:
                drawButtonLegends("Reset", "Home All", encoder_button_text);
                break;
            case Homing:
                drawButtonLegends("Reset", "", encoder_button_text);
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
                drawButtonLegends("", "", encoder_button_text);
                break;
        }

        refreshDisplay();
    }
};
MainScene mainScene;
