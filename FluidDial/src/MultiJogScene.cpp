// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Config.h"

#ifdef USE_MULTI_JOG
#    include "Scene.h"
#    include "ConfirmScene.h"

class MultiJogScene : public Scene {
private:
    int       _dist_index[3] = { 2, 2, 2 };
    int       max_index() { return num_digits() + 3; }  // 10^3 = 1000;
    int       _selected_mask = 1 << 0;
    const int num_axes       = 3;

public:
    MultiJogScene() : Scene("Jog", 4) {}
    float distance(int axis) { return pow(10.0, _dist_index[axis] - num_digits()); }
    void  unselect_all() { _selected_mask = 0; }
    bool  selected(int axis) { return _selected_mask & (1 << axis); }
    bool  only(int axis) { return _selected_mask == (1 << axis); }

    int  next(int axis) { return (axis < 2) ? axis + 1 : 0; }
    void select(int axis) { _selected_mask |= 1 << axis; }
    void unselect(int axis) { _selected_mask &= ~(1 << axis); }

    int the_selected_axis() {
        if ((_selected_mask & (_selected_mask - 1)) != 0) {
            return -2;  // Multiple axes are selected
        }
        for (size_t axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                return axis;
            }
        }
        return -1;  // No axis is selected
    }

    void reDisplay() {
        drawBackground(BLACK);
        drawMenuTitle(current_scene->name());

        DRO dro(30, 68, 200, 32);
        for (size_t axis = 0; axis < num_axes; axis++) {
            dro.draw(axis, _dist_index[axis], selected(axis));
        }
        drawPngFile("/zero.png", { 20, -85 });
        refreshDisplay();
    }
    void zero_axes() {
        std::string cmd = "G10L20P0";
        for (int axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                cmd += axisNumToChar(axis);
                cmd += "0";
            }
        }
        send_line(cmd.c_str());
    }
    void onEntry(void* arg) {
        if (arg) {
            dbg_printf("Entry %s\n", (const char*)arg);
        }

        if (arg && strcmp((const char*)arg, "Confirmed") == 0) {
            zero_axes();
        }
    }

    int which(int x, int y) {
        if (y > 130) {
            return 2;
        }
        return y > 90 ? 1 : 0;
    }

    void confirm_zero_axes() {
        std::string confirmMsg("Zero ");

        for (int axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                confirmMsg += axisNumToChar(axis);
            }
        }
        confirmMsg += " ?";
        dbg_println(confirmMsg.c_str());
        push_scene(&confirmScene, (void*)confirmMsg.c_str());
    }
    void increment_distance(int axis) {
        if (_dist_index[axis] < max_index()) {
            ++_dist_index[axis];
        }
    }
    void increment_distance() {
        for (int axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                increment_distance(axis);
            }
        }
    }
    void decrement_distance(int axis) {
        if (_dist_index[axis] > 0) {
            --_dist_index[axis];
        }
    }

    void decrement_distance() {
        for (int axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                decrement_distance(axis);
            }
        }
    }
    void rotate_distance() {
        for (int axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                if (++_dist_index[axis] >= max_index()) {
                    _dist_index[axis] = 0;
                }
            }
        }
    }
    void cancel_jog() {
        dbg_println("Cancel");
        fnc_realtime(JogCancel);
    }
    void next_axis() {
        int the_axis = the_selected_axis();
        if (the_axis == -2) {
            unselect_all();
            select(num_axes - 1);
            return;
        }
        if (the_axis == -1) {
            select(num_axes - 1);
            return;
        }
        if (the_axis == num_axes - 1) {
            return;
        }
        unselect(the_axis);
        select(the_axis + 1);
    }
    void prev_axis() {
        int the_axis = the_selected_axis();
        if (the_axis == -2) {
            unselect_all();
            select(0);
            return;
        }
        if (the_axis == -1) {
            select(0);
            return;
        }
        if (the_axis == 0) {
            return;
        }
        unselect(the_axis);
        select(the_axis - 1);
    }
    void onTouchRelease(int x, int y) {
#    if 0
        int axis = which(x, y);
        if (axis < 0) {
            return;
        }

        unselect_all();
        select(axis);
        if (x < 80) {
            if (selected(axis)) {
                unselect(axis);
                select(next(axis));
            } else {
                unselect_all();
                select(axis);
            }
        } else if (x < 80) {
            increment_distance(axis);
        } else {
            decrement_distance(axis);
        }
#    else
        if (x < 120) {
            increment_distance();
        } else {
            decrement_distance();
        }
#    endif
        reDisplay();
    }
    void onTouchHold(int x, int y) {
        if (x < 80) {
            int axis = which(x, y);
            if (selected(axis) && !only(axis)) {
                unselect(axis);
            } else {
                select(axis);
            }
            reDisplay();
            return;
        }
        if (x < 160 && y > 160) {
            confirm_zero_axes();
        }
    }

    void onTouchFlick(int x, int y, int dx, int dy) override {
        int absdx = std::abs(dx);
        int absdy = std::abs(dy);
        if (absdy < 30 && dx < -40) {  // Flick left
            pop_scene();
            return;
        }
        if (absdx < 30) {
            if (dy > 40) {  // Flick down
                next_axis();
                // decrement_distance();
            } else if (dy < -40) {
                // increment_distance();
                prev_axis();
            }
            reDisplay();
            return;
        }
    }
    void onDialButtonPress() { cancel_jog(); }

    void start_mpg_jog(int delta) {
        // e.g. $J=G91F1000X-10000
        std::string cmd(inInches ? "$J=G91F400" : "$J=G91F10000");
        for (int axis = 0; axis < num_axes; ++axis) {
            if (selected(axis)) {
                cmd += axisNumToChar(axis);
                cmd += floatToCStr(delta * distance(axis), inInches ? 3 : 2);
            }
        }
        send_line(cmd.c_str());
    }
    void start_button_jog(bool negative) {
        // e.g. $J=G91F1000X-10000
        float total_distance = 0;
        int   n_axes         = 0;
        for (int axis = 0; axis < num_axes; ++axis) {
            if (selected(axis)) {
                float axis_distance = distance(axis);
                total_distance += axis_distance * axis_distance;
                ++n_axes;
            }
        }
        total_distance = sqrtf(total_distance);

        float feedrate = total_distance * 300;  // go 5x the highlighted distance in 1 second

        std::string cmd("$J=G91");
        cmd += inInches ? "G20" : "G21";
        cmd += "F";
        cmd += floatToCStr(feedrate, 3);
        for (int axis = 0; axis < num_axes; ++axis) {
            if (selected(axis)) {
                float axis_distance;
                if (n_axes == 1) {
                    axis_distance = inInches ? 400 : 10000;
                } else {
                    axis_distance = distance(axis) * 20;
                }
                if (negative) {
                    axis_distance = -axis_distance;
                }
                cmd += axisNumToChar(axis);
                cmd += floatToCStr(axis_distance, 0);
            }
        }
        send_line(cmd.c_str());
    }

    void onGreenButtonPress() {
        if (state == Idle) {
            start_button_jog(false);
        }
    }
    void onGreenButtonRelease() { cancel_jog(); }
    void onRedButtonPress() {
        if (state == Idle) {
            start_button_jog(true);
        }
    }
    void onRedButtonRelease() { cancel_jog(); }

    void onEncoder(int delta) {
        if (delta != 0) {
            start_mpg_jog(delta);
        }
    }

    void onDROChange() { reDisplay(); }
    void onLimitsChange() { reDisplay(); }
    void onAlarm() { reDisplay(); }
} joggingScene;

#endif
