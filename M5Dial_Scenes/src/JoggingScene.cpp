// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class JoggingScene : public Scene {
private:
    static const int MAX_JOG_INC = 5;

    bool prefsChanged = false;

    int  active_setting = 0;  // Dist or Rate
    int  selection      = 0;
    int  continuous     = 0;  // 0 off 1 = pos, 2 = neg
    bool jog_continuous = false;

    int jog_axis = 0;  // the axis currently being jogged

    int jog_inc_level[3]  = { 2, 2, 1 };  // exponent 0=0.01, 2=0.1 ... 5 = 100.00
    int jog_rate_level[3] = { 1000, 1000, 100 };
    int jog_cont_speed[3] = { 1000, 1000, 1000 };

    float jog_increment() { return pow(10.0, abs(jog_inc_level[jog_axis])) / 100.0; }

public:
    JoggingScene() : Scene("Jog Dial") {}

    void onDialButtonRelease() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle) {
            if (selection % 2) {
                send_line("G10L20P0" + axisNumToString(jog_axis) + "0");
                send_line("$Log/Msg=*G10L20P0" + axisNumToString(jog_axis) + "0");
            } else {
                if (jog_continuous) {
                    // $J=G91F1000X10000
                    send_line("$J=G91F" + floatToString(jog_cont_speed[jog_axis], 0) + axisNumToString(jog_axis) + "10000");
                } else {
                    if (active_setting == 0) {
                        if (jog_inc_level[jog_axis] != MAX_JOG_INC) {
                            jog_inc_level[jog_axis]++;
                        }
                    } else {
                        feedRateRotator(jog_rate_level[jog_axis], true);
                    }
                    prefsChanged = true;
                }
                display();
            }
            return;
        }
        if (state == Jog) {
            if (!jog_continuous) {
                fnc_realtime(JogCancel);  // reset
            }
            return;
        }
    }
    void onGreenButtonRelease() {
        if (jog_continuous) {
            fnc_realtime(JogCancel);  // reset
        }
    }

    void onRedButtonPress() {
        if (state == Idle) {
            if (!(selection % 2)) {
                if (jog_continuous) {
                    // $J=G91F1000X-10000
                    send_line("$J=G91F" + floatToString(jog_cont_speed[jog_axis], 0) + axisNumToString(jog_axis) + "-10000");
                } else {
                    if (active_setting == 0) {
                        if (jog_inc_level[jog_axis] > 0) {
                            jog_inc_level[jog_axis]--;
                        }
                    } else {
                        feedRateRotator(jog_rate_level[jog_axis], false);
                    }
                    prefsChanged = true;
                }
                display();
            }
            return;
        }
        if (state == Jog) {
            if (!jog_continuous) {
                fnc_realtime(Reset);
            }
            return;
        }
    }
    void onRedButtonRelease() {
        if (jog_continuous) {
            fnc_realtime(JogCancel);  // reset
        }
    }

    void onTouchRelease(m5::touch_detail_t t) {
        // Rotate through the axis being jogged
        //USBSerial.printf("Touch x:%i y:%i\r\n", t.x, t.y);
        //Use dial to break out of continuous mode
        if (t.y < 70) {
            jog_continuous = !jog_continuous;
        } else if (t.y < 140) {
            rotateNumberLoop(selection, 1, 0, 5);
            jog_axis = (selection) / 2;
        } else {
            rotateNumberLoop(active_setting, 1, 0, 1);
        }
        USBSerial.printf("Selection:%d Axis:%d\r\n", selection, jog_axis);
        display();
    }

    void onDROChange() { display(); }
    void onLimitsChange() { display(); }
    void onAlarm() { display(); }

    void onEncoder(int delta) {
        if (jog_continuous) {
            if (delta != 0) {
                feedRateRotator(jog_cont_speed[jog_axis], delta > 0);
            }
        } else {
            if (delta != 0) {
                // $J=G91F200Z5.0
                //$Log/Msg = *
                send_line("$Log/Msg=*Jog delta:" + String(delta));
                String jogCmd = "$J=G91F" + floatToString(jog_rate_level[jog_axis], 0) + axisNumToString(jog_axis);
                if (delta < 0) {
                    jogCmd += "-";
                }
                jogCmd += floatToString(jog_increment(), 2);
                send_line(jogCmd);
            }
        }
    }

    void display() {
        canvas.fillSprite(BLACK);

        drawStatus();
        int x      = 9;
        int y      = 69;
        int width  = 180;
        int offset = 33;

        drawDRO(x, y, width, 0, myAxes[0], jog_axis == 0);
        drawDRO(x, y += offset, width, 1, myAxes[1], jog_axis == 1);
        drawDRO(x, y += offset, width, 2, myAxes[2], jog_axis == 2);

        int height = 32;
        x          = x + width + 1;
        y          = 69;
        width      = 42;
        drawButton(x, y, width, height, 9, "zro", selection == 1);
        drawButton(x, y += offset, width, height, 9, "zro", selection == 3);
        drawButton(x, y += offset, width, height, 9, "zro", selection == 5);

        if (jog_continuous) {
            canvas.setFont(&fonts::FreeSansBold9pt7b);
            canvas.setTextDatum(middle_center);
            canvas.setTextColor(WHITE);
            canvas.drawString("Jog Rate: " + floatToString(jog_cont_speed[jog_axis], 0), 120, 185);
        } else {
            canvas.setFont(&fonts::FreeSansBold9pt7b);
            canvas.setTextDatum(middle_center);
            canvas.setTextColor((active_setting == 0) ? WHITE : DARKGREY);
            canvas.drawString("Jog Dist: " + floatToString(jog_increment(), 2), 120, 177);
            canvas.setTextColor((active_setting == 1) ? WHITE : DARKGREY);
            canvas.drawString("Jog Rate: " + floatToString(jog_rate_level[jog_axis], 2), 120, 193);
        }

        canvas.setFont(&fonts::FreeSansBold9pt7b);
        canvas.setTextDatum(middle_center);
        canvas.setTextColor(WHITE);
        canvas.drawString((jog_continuous) ? "Bttn Jog" : "Knob Jog", 120, 12);

        switch (state) {
            case Idle:
                if (jog_continuous) {
                    if (selection % 2) {
                        buttonLegends("", "Zero " + axisNumToString(jog_axis), "Main");
                    } else {
                        buttonLegends("Jog-", "Jog+", "Main");
                    }
                } else {
                    if (selection % 2) {  // if zro selected
                        buttonLegends("", "Zero " + axisNumToString(jog_axis), "Main");
                    } else {
                        buttonLegends("Dec", "Inc", "Main");
                    }
                }
                break;
            case Jog:
                if (jog_continuous) {
                    buttonLegends("Jog-", "Jog+", "Main");
                } else {
                    buttonLegends("E-Stop", "Cancel", "Main");
                }
                break;
            case Alarm:
                buttonLegends("", "", "Main");
                break;
        }

        refreshDisplaySprite();
    }
};
JoggingScene joggingScene;
