// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Config.h"

#ifndef USE_MULTI_JOG
#    include "Scene.h"
#    include <string>
#    include "e4math.h"

class JoggingScene : public Scene {
private:
    static const int MIN_INC = -3;
    static const int MAX_INC = 2;

    int  _active_setting = 0;  // Dist or Rate
    int  _selection      = 0;
    bool _continuous     = false;

    int _axis = 0;  // the axis currently being jogged

    int _cont_speed[2][3] = { { 1000, 1000, 1000, 1000, 1000, 1000 }, { 40, 40, 40 } };

    // Saved to NVS
    int _inc_level[2][6]  = { { 0, 0, -1, -1, -1, -1 },
                             { -1, -1, -2, -2, -2, -2 } };  // exponent -4=0.0001, -3=0.001, -2=0.01 -1=0.1 0=1 1=10  2=100
    int _rate_level[2][6] = { { 1000, 1000, 100, 100, 100, 100 }, { 40, 40, 4, 4, 4, 4 } };

    e4_t _increment() { return e4_power10(_inc_level[inInches][_axis]); }

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
    void cancelJog() {
        if (state == Jog) {
            fnc_realtime(JogCancel);
        }
    }

public:
    JoggingScene() : Scene("MPG Jog", 4) {}

    void onEntry(void* arg) {
        if (initPrefs()) {
            for (size_t axis = 0; axis < n_axes; axis++) {
                getPref(inInches ? "InchIncLevel" : "IncLevel", axis, &_inc_level[inInches][axis]);
                getPref(inInches ? "InchRateLevel" : "RateLevel", axis, &_rate_level[inInches][axis]);
            }
        }
    }
    void onExit() { cancelJog(); }

    void onDialButtonPress() { pop_scene(); }

    static void continuousJogCommand(int speed, int axis, int distance) {
        send_linef("$J=G91%sF%d%c%d", inInches ? "G20" : "G21", speed, axisNumToChar(axis), distance);
    }
    static void incrementalJogCommand(int speed, int axis, int delta) {}

    void onGreenButtonPress() {
        if (state == Idle) {
            if (_continuous) {
                // e.g. $J=G91F1000X1000
                continuousJogCommand(_cont_speed[inInches][_axis], _axis, 2000);
            } else {
                if (_active_setting == 0) {
                    if (_inc_level[inInches][_axis] != MAX_INC) {
                        _inc_level[inInches][_axis]++;
                        setPref(inInches ? "InchIncLevel" : "IncLevel", _axis, _inc_level[inInches][_axis]);
                    }
                } else {
                    feedRateRotator(_rate_level[inInches][_axis], true);
                    setPref(inInches ? "InchRateLevel" : "RateLevel", _axis, _rate_level[inInches][_axis]);
                }
            }
            reDisplay();

            return;
        }
        cancelJog();
    }
    void onGreenButtonRelease() {
        if (_continuous) {
            cancelJog();
        }
    }

    void onRedButtonPress() {
        if (state == Idle) {
            if (_continuous) {
                // $J=G91F1000X-1000
                continuousJogCommand(_cont_speed[inInches][_axis], _axis, 2000);
            } else {
                if (_active_setting == 0) {
                    if (_inc_level[inInches][_axis] > MIN_INC) {
                        _inc_level[inInches][_axis]--;
                        setPref("IncLevel", _axis, _inc_level[inInches][_axis]);
                    }
                } else {
                    feedRateRotator(_rate_level[inInches][_axis], false);
                    setPref("RateLevel", _axis, _rate_level[inInches][_axis]);
                }
            }
            reDisplay();
            return;
        }
        if (state == Jog || state == Cycle) {
            if (!_continuous) {
                fnc_realtime(Reset);
            }
            return;
        }
    }
    void onRedButtonRelease() {
        if (_continuous) {
            cancelJog();
        }
    }

    void onTouchRelease(int x, int y) {
        //Use dial to break out of continuous mode
        if (state == Idle) {
            if (y < 70) {
                _continuous = !_continuous;
            } else if (y < 105) {
                rotateNumberLoop(_axis, 1, 0, n_axes - 1);
            } else if (y < 140) {
                send_linef("G10L20P0%c0", axisNumToChar(_axis));
            } else {
                rotateNumberLoop(_active_setting, 1, 0, 1);
            }
            ackBeep();
        }
        reDisplay();
    }

    void onDROChange() { reDisplay(); }
    void onLimitsChange() { reDisplay(); }
    void onAlarm() { reDisplay(); }

    void onEncoder(int delta) {
        if (state == Idle && _continuous) {
            feedRateRotator(_cont_speed[inInches][_axis], delta > 0);
        } else {
            // $J=G91F200Z5.0
            e4_t distance = delta >= 0 ? _increment() : -_increment();
            int  speed    = _rate_level[inInches][_axis];
            send_linef("$J=G91%sF%d%c%s", inInches ? "G20" : "G21", speed, axisNumToChar(_axis), e4_to_cstr(distance, num_digits()));
        }
        reDisplay();
    }

    void reDisplay() {
        // drawBackground(BLACK);
        background();
        centered_text(_continuous ? "Btn Jog" : "MPG Jog", 12);

        drawStatus();

        std::string legend;
        if (state == Idle || state == Jog) {
            int x      = 14;
            int y      = 67;
            int width  = display.width() - x * 2;
            int height = 38;

            DRO dro(x, y, width, 35);
            dro.draw(_axis, true);

            x = 60;
            y += height + 5;
            width  = display.width() - x * 2;
            height = 48;

            if (state == Idle) {
                Stripe stripe(x, y, width, height, TINY);
                legend = "Zero ";
                legend += axisNumToChar(_axis);
                legend += " Axis";
                stripe.draw(legend.c_str(), true);
            }

            if (_continuous) {
                legend = "Rate: ";
                legend += intToCStr(_cont_speed[inInches][_axis]);
                centered_text(legend.c_str(), 183);
            } else {
                legend = "Increment: ";
                legend += e4_to_cstr(_increment(), num_digits());
                centered_text(legend.c_str(), 174, _active_setting == 0 ? WHITE : DARKGREY);
                legend = "Rate: ";
                legend += intToCStr(_rate_level[inInches][_axis]);
                centered_text(legend.c_str(), 194, _active_setting == 1 ? WHITE : DARKGREY);
            }

            const char* back = "Back";
            switch (state) {
                case Idle:
                    if (_continuous) {
                        if (_selection % 2) {
                            legend = "Zero ";
                            legend += axisNumToChar(_axis);
                            drawButtonLegends("", legend.c_str(), back);
                        } else {
                            drawButtonLegends("Jog-", "Jog+", back);
                        }
                    } else {
                        if (_selection % 2) {  // if zro selected
                            legend = "Zero ";
                            legend += axisNumToChar(_axis);
                            drawButtonLegends("", legend.c_str(), back);
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
        } else {
            centered_text("Invalid State", 105, WHITE, MEDIUM);
            centered_text("For Jogging", 145, WHITE, MEDIUM);

            drawButtonLegends("E-Stop", "", "Menu");
        }

        refreshDisplay();
    }
};
JoggingScene joggingScene;
#endif
