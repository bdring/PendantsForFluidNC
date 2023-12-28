// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class JoggingScene : public Scene {
private:
    static const int MAX_JOG_INC = 5;

    bool skippedLast;
    bool lastJogFwd;

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
    JoggingScene() : Scene("Jog Dial") {
        skippedLast = true;
        lastJogFwd  = true;
    }

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle) {
            {
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
        if (t.y < 70) {
            jog_continuous = !jog_continuous;
        } else if (t.y < 140) {
            rotateNumberLoop(jog_axis, 1, 0, 2);
        } else if (t.y > 116 && t.y < 158) {
            String cmd = "G10L20P0" + axisNumToString(jog_axis) + "0";
            log_msg(cmd);
            send_line(cmd);
        } else {
            rotateNumberLoop(active_setting, 1, 0, 1);
        }
        USBSerial.printf("Selection:%d Axis:%d\r\n", selection, jog_axis);
        M5Dial.Speaker.tone(2000, 20);
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

                USBSerial.print("Delta");
                USBSerial.println(delta);

                // encoder error filtering
                if (abs(delta) > 1) {
                    return;
                }
                bool jogFwd = delta > 0;
                if (lastJogFwd != jogFwd) {  // are we changing direction
                    if (!skippedLast) {
                        skippedLast = true;
                        return;
                    } else {
                        skippedLast = false;
                    }
                }
                lastJogFwd = jogFwd;
                // end filtering

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
        display();
    }

    void display() {
        drawBackground(BLACK);
        drawStatus();

        int x      = 15;
        int y      = 68;
        int width  = 210;
        int height = 40;

        DRO dro(x, y, width, height);
        dro.draw(jog_axis, true);

        String legend;

        legend = "Zero " + String("XYZ").substring(jog_axis, jog_axis + 1) + " Axis";
        Stripe stripe(60, 116, 120, 42, TINY);
        stripe.draw(legend, false);

        legend = jog_continuous ? "Btn Jog" : "MPG Jog";
        centered_text(legend, 12);

        if (jog_continuous) {
            legend = "Speed: " + floatToString(jog_cont_speed[jog_axis], 0);
            centered_text(legend, 184);
        } else {
            legend = "MPG Incr: " + floatToString(jog_increment(), 2);
            centered_text(legend, 173, active_setting == 0 ? WHITE : DARKGREY);
            legend = "Rate: " + floatToString(jog_rate_level[jog_axis], 0);
            centered_text(legend, 195, active_setting == 1 ? WHITE : DARKGREY);
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
        showError();  // if there is one
        refreshDisplay();
    }
};
JoggingScene joggingScene;
