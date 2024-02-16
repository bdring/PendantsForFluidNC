// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Scene.h"

extern Scene probingScene;
extern Scene homingScene;
extern Scene joggingScene;
extern Scene controlScene;
extern Scene menuScene;
//extern Scene fileScene;

class MainScene : public Scene {
private:
    int menu_item = 0;

public:
    MainScene() : Scene("Main") {}

    void onDialButtonPress() {
        if (state == Idle || state == Alarm) {
            push_scene(&menuScene);
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
            case Idle:
            case Alarm:
                send_line("$H");
                return;  // no status report
        }
        fnc_realtime(StatusReport);
    }

    void onEncoder(int delta) {
        menu_item += delta;
        if (state == Cycle) {
            if (delta > 0 && myFro < 200) {
                fnc_realtime(FeedOvrFinePlus);
            } else if (delta < 0 && myFro > 10) {
                fnc_realtime(FeedOvrFineMinus);
            }
            //display();
        } else if (state == Idle) {
            menu_item += delta * 10;
            display();
        }
    }

    void onDROChange() { display(); }
    void onLimitsChange() { display(); }

    void display() {
        canvas.createSprite(240, 240);
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
            char legend[50];
            sprintf(legend, "Feed Rate Ovr:%d%%", myFro);
            centered_text(legend, y + 23);
        }

        const char* encoder_button_text = "Menu";
        const char* redButtonText       = "";
        const char* greenButtonText     = "";
        switch (state) {
            case Alarm:
                drawButtonLegends("Reset", "Home All", encoder_button_text);
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
        drawError();  // if there is one
        refreshDisplay();
    }
};
MainScene mainScene;
