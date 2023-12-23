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

    void feedRateRotator(int& rate, bool up) {
        if (up) {
            if (rate < 10) {
                rate += 1;
            } else if (rate < 100) {
                rate += 10;
            } else if (rate < 1000) {
                rate += 100;
            } else {
                rate += 1000;
            }
        } else {
            if (rate > 1000) {
                rate -= 1000;
            } else if (rate > 100) {
                rate -= 100;
            } else if (rate > 10) {
                rate -= 10;
            } else if (rate > 2) {
                rate -= 1;
            }
        }
    }

public:
    JoggingScene() : Scene("Jog Dial") {}

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle) {
            if (selection % 2) {  // Zero WCO
                String cmd = "G10L20P0" + axisNumToString(jog_axis) + "0";
                log_msg(cmd);
                send_line(cmd);
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
                reDisplay();
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
                reDisplay();
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

    void onTouchRelease(int x, int y) {
        // Rotate through the axis being jogged
        //debugPort.printf("Touch x:%i y:%i\r\n", t.x, t.y);
        //Use dial to break out of continuous mode
        if (y < 70) {
            jog_continuous = !jog_continuous;
        } else if (y < 140) {
            rotateNumberLoop(selection, 1, 0, 5);
            jog_axis = (selection) / 2;
        } else {
            rotateNumberLoop(active_setting, 1, 0, 1);
        }
        debugPort.printf("Selection:%d Axis:%d\r\n", selection, jog_axis);
        reDisplay();
    }

    void onDROChange() { reDisplay(); }
    void onLimitsChange() { reDisplay(); }
    void onAlarm() { reDisplay(); }

    void onEncoder(int delta) {
        if (jog_continuous) {
            if (delta != 0) {
                feedRateRotator(jog_cont_speed[jog_axis], delta > 0);
            }
        } else {
            if (delta != 0) {
                // $J=G91F200Z5.0
                String jogRate      = floatToString(jog_rate_level[jog_axis], 0);
                String jogIncrement = floatToString(jog_increment(), 2);
                String cmd          = "$J=G91F" + jogRate + axisNumToString(jog_axis);
                if (delta < 0) {
                    cmd += "-";
                }
                cmd += jogIncrement;
                log_msg(cmd);
                send_line(cmd);
            }
        }
    }

    void reDisplay() {
        drawBackground(BLACK);
        drawStatus();

        int x      = 9;
        int y      = 69;
        int width  = 180;
        int height = 33;

        DRO dro(x, y, width, height);
        dro.draw(0, jog_axis == 0);
        dro.draw(1, jog_axis == 1);
        dro.draw(2, jog_axis == 2);

        Stripe stripe(x + width + 1, y, 42, height, TINY);
        stripe.draw("zro", selection == 1);
        stripe.draw("zro", selection == 3);
        stripe.draw("zro", selection == 5);

        String legend;
        legend = jog_continuous ? "Bttn Jog" : "Knob Jog";
        centered_text(legend, 12);

        if (jog_continuous) {
            legend = "Jog Rate: " + floatToString(jog_cont_speed[jog_axis], 0);
            centered_text(legend, 185);
        } else {
            legend = "Jog Dist: " + floatToString(jog_increment(), 2);
            centered_text(legend, 177, active_setting == 0 ? WHITE : DARKGREY);
            legend = "Jog Rate: " + floatToString(jog_rate_level[jog_axis], 2);
            centered_text(legend, 193, active_setting == 1 ? WHITE : DARKGREY);
        }

        const char* back = "Back";
        switch (state) {
            case Idle:
                if (jog_continuous) {
                    if (selection % 2) {
                        drawButtonLegends("", "Zero " + axisNumToString(jog_axis), back);
                    } else {
                        drawButtonLegends("Jog-", "Jog+", back);
                    }
                } else {
                    if (selection % 2) {  // if zro selected
                        drawButtonLegends("", "Zero " + axisNumToString(jog_axis), back);
                    } else {
                        drawButtonLegends("Dec", "Inc", back);
                    }
                }
                break;
            case Jog:
                if (jog_continuous) {
                    drawButtonLegends("Jog-", "Jog+", back);
                } else {
                    drawButtonLegends("E-Stop", "Cancel", back);
                }
                break;
            case Alarm:
                drawButtonLegends("", "", back);
                break;
        }

        refreshDisplay();
    }
};
JoggingScene joggingScene;
