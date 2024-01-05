// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Menu.h"
#include "ConfirmScene.h"

constexpr const int tbRadius = 30;

class TB : public Item {
    color_t     _fill_color;
    color_t     _text_color;
    const char* _text;

public:
    TB(const char* name, callback_t callback, const char* text, color_t fill_color, color_t text_color) :
        Item(name, callback), _text(text), _fill_color(fill_color), _text_color(text_color) {}
    TB(const char* name, Scene* scene, const char* text, color_t fill_color, color_t text_color) :
        Item(name, scene), _text(text), _fill_color(fill_color), _text_color(text_color) {}
    void set_text(const char* text) { _text = text; }
    void show(const Point& where) {
        drawFilledCircle(where, tbRadius, _fill_color);
        text(_text, where, _text_color, MEDIUM);
    }
};

class XB : public Item {
    const char* _text;

public:
    XB(const char* name, callback_t callback, const char* text) : Item(name, callback), _text(text) {}
    XB(const char* name, Scene* scene, const char* text) : Item(name, scene), _text(text) {}
    void set_text(const char* text) { _text = text; }
    void show(const Point& where) { text(_text, where, WHITE, MEDIUM); }
};

class TIB : public Item {
    const char* _filename;

public:
    TIB(const char* text, callback_t callback, const char* filename) : Item(text, callback), _filename(filename) {}
    TIB(const char* text, Scene* scene, const char* filename) : Item(text, scene), _filename(filename) {}
    void show(const Point& where) { drawPngFile(_filename, where); }
};

static enum { RATE = 0, DISTANCE } control = RATE;

static int          _rate[3]           = { 1000, 1000, 1000 };
static float        _distances[]       = { 0.01, 0.1, 1, 10, 100 };
constexpr const int n_distances        = sizeof(_distances) / sizeof(_distances[0]);
static const char*  distance_legends[] = { "0.01", "0.1", "1", "10", "100" };
static int          _distance_index[3] = { 2, 2, 2 };
static int          _axis              = 0;

extern XB switchAxis;  // Forward reference

class JogAxisScene : public PieMenu {
private:
public:
    JogAxisScene() : PieMenu("JogAxis", tbRadius) {}
    static void zero_axis() {
        String cmd = "G10L20P0" + axisNumToString(_axis) + "0";
        log_msg(cmd);
        send_line(cmd);
    }
    void init(void* arg) {
        if (arg && strcmp((const char*)arg, "Confirmed") == 0) {
            zero_axis();
        }
    }
    void menuBackground() {
        drawPngBackground("/JogBG.png");

        String legend = "Rate: " + String(_rate[_axis]);
        text(legend, 75, 80, control == RATE ? YELLOW : WHITE, SMALL, middle_left);

        DRO dro(60, 95, 177, 40);
        dro.draw(_axis, state != Idle);

        legend = "Distance: ";
        legend += distance_legends[_distance_index[_axis]];
        text(legend, 75, 155, control == DISTANCE ? YELLOW : WHITE, SMALL, middle_left);
    }

    static void switch_axis(void* arg) {
        if (++_axis == 3) {
            _axis = 0;
        }
        // switchAxis.set_text(next_axis_text[_axis]);
        current_scene->reDisplay();
    };
    static void confirm_zero_axis(void* arg) { push_scene(&confirmScene, (void*)"Zero axis?"); }
    static void toggle_rd(void* arg) {
        control = (control == RATE) ? DISTANCE : RATE;
        current_scene->reDisplay();
    }
    static void increment_level(void* arg) {
        if (control == RATE) {
            if (_rate[_axis] < 1000) {
                _rate[_axis] *= 10;
            }
        } else {
            if (_distance_index[_axis] < (n_distances - 1)) {
                ++_distance_index[_axis];
            }
        }
        current_scene->reDisplay();
    }
    static void decrement_level(void* arg) {
        if (control == RATE) {
            if (_rate[_axis] > 10) {
                _rate[_axis] /= 10;
            }
        } else {
            if (_distance_index[_axis] > 0) {
                --_distance_index[_axis];
            }
        }
        current_scene->reDisplay();
    }
    static void cancel_jog(void* arg = nullptr) {
        log_msg("Cancel");
        fnc_realtime(JogCancel);
    }

    void onTouchRelease(int x, int y) override {
        int item = touchedItem(x, y);
        if (item == -1 || item == 2) {  // Center or right, over the DRO
            switch_axis(nullptr);
        }
        if (item >= 0) {
            _items[item]->invoke();
        }
    }
    void onDialButtonPress() { pop_scene(); }

    static void start_jog(const char* direction, const char* distance) {
        // e.g. $J=G91F1000X-10000
        String cmd("$J=G91F" + floatToString(_rate[_axis], 0) + axisNumToString(_axis) + direction + distance);
        send_line(cmd);
        log_msg(cmd);
    }

    void onGreenButtonPress() {
        if (state == Idle) {
            start_jog("", "100000");
        }
    }
    void onGreenButtonRelease() { cancel_jog(); }
    void onRedButtonPress() {
        if (state == Idle) {
            start_jog("-", "10000");
        }
    }
    void onRedButtonRelease() { cancel_jog(); }

    void onEncoder(int delta) {
        if (delta != 0) {
            // $J=G91F200Z5.0
            start_jog(delta < 0 ? "-" : "", distance_legends[_distance_index[_axis]]);
        }
    }

    void onDROChange() { reDisplay(); }
    void onLimitsChange() { reDisplay(); }
    void onAlarm() { reDisplay(); }
};

// XB           switchAxis("Axis", JogAxisScene::switch_axis, ">Y");
JogAxisScene jogAxisScene;
EmptyItem    emptyItem;
Scene*       initJogAxisScene() {
    jogAxisScene.addItem(new TIB("Stop", JogAxisScene::cancel_jog, "/stop.png"));
    jogAxisScene.addItem(&emptyItem);
    jogAxisScene.addItem(&emptyItem);
    jogAxisScene.addItem(new XB("Increment", JogAxisScene::increment_level, "++"));
    jogAxisScene.addItem(new XB("ToggleRD", JogAxisScene::toggle_rd, "r/d"));
    jogAxisScene.addItem(new XB("Decrement", JogAxisScene::decrement_level, "--"));
    jogAxisScene.addItem(new TIB("Back", pop_scene, "/back.png"));
    jogAxisScene.addItem(new XB("Zero", JogAxisScene::confirm_zero_axis, "0"));
    return &jogAxisScene;
}
