// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class JoggingScene : public Scene {
private:
    static const int MAX_INC = 5;
    static const int n_axes  = 3;

    int  _active_setting = 0;  // Dist or Rate
    int  _selection      = 0;
    bool _continuous     = false;

    int _axis = 0;  // the axis currently being jogged

    int _cont_speed[3] = { 1000, 1000, 1000 };

    // Saved to NVS
    int _inc_level[3]  = { 2, 2, 1 };  // exponent 0=0.01, 2=0.1 ... 5 = 100.00
    int _rate_level[3] = { 1000, 1000, 100 };

    float _increment() { return pow(10.0, abs(_inc_level[_axis])) / 100.0; }

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

    void init(void* arg) {
        if (initPrefs()) {
            for (size_t axis = 0; axis < n_axes; axis++) {
                getPref("IncLevel", axis, &_inc_level[axis]);
                getPref("RateLevel", axis, &_rate_level[axis]);
            }
        }
    }

    void onDialButtonPress() { pop_scene(); }
    void onGreenButtonPress() {
        if (state == Idle) {
            if (_selection % 2) {  // Zero WCO
                String cmd = "G10L20P0" + axisNumToString(_axis) + "0";
                log_msg(cmd);
                send_line(cmd);
            } else {
                if (_continuous) {
                    // $J=G91F1000X10000
                    send_line("$J=G91F" + floatToString(_cont_speed[_axis], 0) + axisNumToString(_axis) + "10000");
                } else {
                    if (_active_setting == 0) {
                        if (_inc_level[_axis] != MAX_INC) {
                            _inc_level[_axis]++;
                            setPref("IncLevel", _axis, _inc_level[_axis]);
                        }
                    } else {
                        feedRateRotator(_rate_level[_axis], true);
                        setPref("RateLevel", _axis, _rate_level[_axis]);
                    }
                }
                reDisplay();
            }
            return;
        }
        if (state == Jog) {
            if (!_continuous) {
                fnc_realtime(JogCancel);  // reset
            }
            return;
        }
    }
    void onGreenButtonRelease() {
        if (_continuous) {
            fnc_realtime(JogCancel);  // reset
        }
    }

    void onRedButtonPress() {
        if (state == Idle) {
            if (!(_selection % 2)) {
                if (_continuous) {
                    // $J=G91F1000X-10000
                    send_line("$J=G91F" + floatToString(_cont_speed[_axis], 0) + axisNumToString(_axis) + "-10000");
                } else {
                    if (_active_setting == 0) {
                        if (_inc_level[_axis] > 0) {
                            _inc_level[_axis]--;
                            setPref("IncLevel", _axis, _inc_level[_axis]);
                        }
                    } else {
                        feedRateRotator(_rate_level[_axis], false);
                        setPref("RateLevel", _axis, _rate_level[_axis]);
                    }
                }
                reDisplay();
            }
            return;
        }
        if (state == Jog) {
            if (!_continuous) {
                fnc_realtime(Reset);
            }
            return;
        }
    }
    void onRedButtonRelease() {
        if (_continuous) {
            fnc_realtime(JogCancel);  // reset
        }
    }

    void onTouchRelease(int x, int y) {
        // Rotate through the axis being jogged
        //debugPort.printf("Touch x:%i y:%i\r\n", t.x, t.y);
        //Use dial to break out of continuous mode
        if (y < 70) {
            _continuous = !_continuous;
        } else if (y < 140) {
            rotateNumberLoop(_selection, 1, 0, 5);
            _axis = (_selection) / 2;
        } else {
            rotateNumberLoop(_active_setting, 1, 0, 1);
        }
        debugPort.printf("Selection:%d Axis:%d\r\n", _selection, _axis);
        reDisplay();
    }

    void onDROChange() { reDisplay(); }
    void onLimitsChange() { reDisplay(); }
    void onAlarm() { reDisplay(); }

    void onEncoder(int delta) {
        if (_continuous) {
            if (delta != 0) {
                feedRateRotator(_cont_speed[_axis], delta > 0);
            }
        } else {
            if (delta != 0) {
                // $J=G91F200Z5.0
                String jogRate      = floatToString(_rate_level[_axis], 0);
                String jogIncrement = floatToString(_increment(), 2);
                String cmd          = "$J=G91F" + jogRate + axisNumToString(_axis);
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
        dro.draw(0, _axis == 0);
        dro.draw(1, _axis == 1);
        dro.draw(2, _axis == 2);

        Stripe stripe(x + width + 1, y, 42, height, TINY);
        stripe.draw("zro", _selection == 1);
        stripe.draw("zro", _selection == 3);
        stripe.draw("zro", _selection == 5);

        String legend;
        legend = _continuous ? "Bttn Jog" : "Knob Jog";
        centered_text(legend, 12);

        if (_continuous) {
            legend = "Jog Rate: " + floatToString(_cont_speed[_axis], 0);
            centered_text(legend, 185);
        } else {
            legend = "Jog Dist: " + floatToString(_increment(), 2);
            centered_text(legend, 177, _active_setting == 0 ? WHITE : DARKGREY);
            legend = "Jog Rate: " + floatToString(_rate_level[_axis], 2);
            centered_text(legend, 193, _active_setting == 1 ? WHITE : DARKGREY);
        }

        const char* back = "Back";
        switch (state) {
            case Idle:
                if (_continuous) {
                    if (_selection % 2) {
                        drawButtonLegends("", "Zero " + axisNumToString(_axis), back);
                    } else {
                        drawButtonLegends("Jog-", "Jog+", back);
                    }
                } else {
                    if (_selection % 2) {  // if zro selected
                        drawButtonLegends("", "Zero " + axisNumToString(_axis), back);
                    } else {
                        drawButtonLegends("Dec", "Inc", back);
                    }
                }
                break;
            case Jog:
                if (_continuous) {
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
