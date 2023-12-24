// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

class ProbingScene : public Scene {
private:
    int  selection    = 0;
    bool prefsChanged = false;
    long oldPosition  = 0;

    // prefs
    float probe_offset  = 0.0;
    float probe_travel  = -20.0;
    float probe_rate    = 80.0;
    float probe_retract = 20.0;
    int   probe_axis    = 2;  // Z is default

public:
    ProbingScene() : Scene("Probe") {}

    void onDialButtonPress() { pop_scene(); }

    void onGreenButtonPress() {
        // G38.2 G91 F80 Z-20 P8.00
        if (state == Idle) {
            String gcode = "G38.2G91";
            gcode += "F" + floatToString(probe_rate, 0);
            gcode += axisNumToString(probe_axis) + floatToString(probe_travel, 0);
            gcode += "P" + floatToString(probe_offset, 2);
            USBSerial.println(gcode);
            send_line(gcode);
            return;
        }
        if (state == Cycle) {
            USBSerial.println("Hold");
            fnc_realtime(FeedHold);
            return;
        }
        if (state == Hold) {
            USBSerial.println("Resume");
            fnc_realtime(CycleStart);
            return;
        }
    }

    void onRedButtonPress() {
        // G38.2 G91 F80 Z-20 P8.00
        if (state == Cycle) {
            Serial1.println("Reset");
            fnc_realtime(Reset);
            return;
        }
        if (state == Idle) {
            String gcode = "$J=G91F1000";
            gcode += axisNumToString(probe_axis);
            gcode += (probe_travel < 0) ? "+" : "-";  // retract is opposite travel
            gcode += floatToString(probe_retract, 0);
            send_line(gcode);
            return;
        }
    }

    void onTouchRelease(m5::touch_detail_t t) {
        // Rotate through the items to be adjusted.
        rotateNumberLoop(selection, 1, 0, 4);
        display();
    }

    void onEncoder(int delta) {
        if (abs(delta) > 0) {
            switch (selection) {
                case 0:
                    probe_offset += (float)delta / 100;
                    break;
                case 1:
                    probe_travel += delta;
                    break;
                case 2:
                    probe_rate += delta;
                    if (probe_rate < 1) {
                        probe_rate = 1;
                    }
                    break;
                case 3:
                    probe_retract += delta;
                    if (probe_retract < 0) {
                        probe_retract = 0;
                    }
                    break;
                case 4:
                    rotateNumberLoop(probe_axis, 1, 0, 2);
            }
            display();
            prefsChanged = true;
        }
    }

    void savePrefs() {
        if (prefsChanged) {
            prefsChanged = false;
            // NVS.write(name(), prefsStruct);
        }
    }

    void display() {
        canvas.createSprite(240, 240);
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());
        drawStatus();

        int    x      = 40;
        int    y      = 62;
        int    width  = WIDTH - (x * 2);
        int    height = 25;
        int    pitch  = 27;  // for spacing of buttons
        Stripe button(x, y, width, height, TINY);
        button.draw("Offset", floatToString(probe_offset, 2), selection == 0);
        button.draw("Max Travel", floatToString(probe_travel, 0), selection == 1);
        y = button.y();  // For LED
        button.draw("Feed Rate", floatToString(probe_rate, 0), selection == 2);

        button.draw("Retract", floatToString(probe_retract, 0), selection == 3);
        button.draw("Axis", axisNumToString(probe_axis), selection == 4);

        LED led(x - 20, y + height / 2, 10, button.gap());
        led.draw(myProbeSwitch);

        String grnText = "";
        String redText = "";

        switch (state) {
            case Idle:
                grnText = "Probe";
                redText = "Retract";
                break;
            case Cycle:
                redText = "Reset";
                grnText = "Hold";
                break;
            case Hold:
                redText = "Reset";
                grnText = "Resume";
                break;
        }

        drawButtonLegends(redText, grnText, "Back");
        showError();  // if there is one
        refreshDisplay();
    }
};
ProbingScene probingScene;
