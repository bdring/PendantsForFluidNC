// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class ProbingScene : public Scene {
private:
    int  selection   = 0;
    long oldPosition = 0;

    // Saved to NVS
    float _offset  = 0.0;
    float _travel  = -20.0;
    float _rate    = 80.0;
    float _retract = 20.0;
    int   _axis    = 2;  // Z is default

public:
    ProbingScene() : Scene("Probe") {}

    void onDialButtonPress() { pop_scene(); }

    void onGreenButtonPress() {
        // G38.2 G91 F80 Z-20 P8.00
        if (state == Idle) {
            String gcode = "G38.2G91";
            gcode += "F" + floatToString(_rate, 0);
            gcode += axisNumToString(_axis) + floatToString(_travel, 0);
            gcode += "P" + floatToString(_offset, 2);
            log_println(gcode);
            send_line(gcode);
            return;
        }
        if (state == Cycle) {
            fnc_realtime(FeedHold);
            return;
        }
        if (state == Hold) {
            fnc_realtime(CycleStart);
            return;
        }
    }

    void onRedButtonPress() {
        // G38.2 G91 F80 Z-20 P8.00
        if (state == Cycle || state == Alarm) {
            fnc_realtime(Reset);
            //send_line("$X");
            return;
        } else if (state == Idle) {
            String gcode = "$J=G91F1000";
            gcode += axisNumToString(_axis);
            gcode += (_travel < 0) ? "+" : "-";  // retract is opposite travel
            gcode += floatToString(_retract, 0);
            send_line(gcode);
            return;
        } else if (state == Hold) {
            fnc_realtime(Reset);
        }
    }

    void onTouchRelease(int x, int y) {
        // Rotate through the items to be adjusted.
        rotateNumberLoop(selection, 1, 0, 4);
        reDisplay();
        ackBeep();
    }

    void onDROChange() { reDisplay(); }

    void onEncoder(int delta) {
        if (abs(delta) > 0) {
            switch (selection) {
                case 0:
                    _offset += (float)delta / 100;
                    setPref("Offset", _offset);
                    break;
                case 1:
                    _travel += delta;
                    setPref("Travel", _travel);
                    break;
                case 2:
                    _rate += delta;
                    if (_rate < 1) {
                        _rate = 1;
                    }
                    setPref("Rate", _rate);
                    break;
                case 3:
                    _retract += delta;
                    if (_retract < 0) {
                        _retract = 0;
                    }
                    setPref("Retract", _retract);
                    break;
                case 4:
                    rotateNumberLoop(_axis, 1, 0, 2);
                    setPref("Axis", _axis);
            }
            reDisplay();
        }
    }
    void onEntry(void* arg) override {
        if (initPrefs()) {
            getPref("Offset", &_offset);
            getPref("Travel", &_travel);
            getPref("Rate", &_rate);
            getPref("Retract", &_retract);
            getPref("Axis", &_axis);
        }
    }

    void reDisplay() {
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        drawStatus();

        String grnText, redText;

        if (state == Idle) {
            int    x      = 40;
            int    y      = 62;
            int    width  = display.width() - (x * 2);
            int    height = 25;
            int    pitch  = 27;  // for spacing of buttons
            Stripe button(x, y, width, height, TINY);
            button.draw("Offset", floatToString(_offset, 2), selection == 0);
            button.draw("Max Travel", floatToString(_travel, 0), selection == 1);
            y = button.y();  // For LED
            button.draw("Feed Rate", floatToString(_rate, 0), selection == 2);

            button.draw("Retract", floatToString(_retract, 0), selection == 3);
            button.draw("Axis", axisNumToString(_axis), selection == 4);

            //LED led(x - 20, y + height / 2, 10, button.gap());
            //led.draw(myProbeSwitch);

            grnText = "Probe";
            redText = "Retract";
        } else {
            if (state == Jog || state == Alarm) {  // there is no Probing state, so Cycle is a valid state on this
                //centered_text("Invalid State", 105, WHITE, MEDIUM);
                //centered_text("For Probing", 145, WHITE, MEDIUM);
                redText = "Reset";
            } else {
                int x      = 14;
                int height = 35;
                int y      = 82 - height / 2;

                LED led(120, 190, 10, 5);
                led.draw(myProbeSwitch);

                int width = display.width() - x * 2;
                DRO dro(x, y, width, height);
                dro.draw(0, _axis == 0);
                dro.draw(1, _axis == 1);
                dro.draw(2, _axis == 2);

                switch (state) {
                    case Cycle:
                        redText = "E-Stop";
                        grnText = "Hold";
                        break;
                    case Hold:
                        redText = "Reset";
                        grnText = "Resume";
                        break;
                    case Alarm:
                        redText = "Reset";
                        break;
                }
            }
        }

        drawButtonLegends(redText, grnText, "Back");
        showError();  // only if one just happened
        refreshDisplay();
    }
};
ProbingScene probingScene;
