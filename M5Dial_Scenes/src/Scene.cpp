// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"

Scene* current_scene;

Button greenButton;
Button redButton;
Button dialButton;

std::vector<Scene*> scene_stack;

void activate_scene(Scene* scene) {
    current_scene = scene;
    current_scene->display();
}
void push_scene(Scene* scene) {
    scene_stack.push_back(current_scene);
    activate_scene(scene);
}
void pop_scene() {
    if (scene_stack.size()) {
        Scene* last_scene = scene_stack.back();
        scene_stack.pop_back();
        activate_scene(last_scene);
    }
}

void dispatch_events() {
    static int32_t oldEncoderPos = 0;
    static bool    last_dial     = false;
    static bool    last_red      = false;
    static bool    last_green    = false;

    static m5::touch_state_t last_touch_state = {};

    M5Dial.update();
    int32_t newEncoderPos = M5Dial.Encoder.read();
    int32_t encoderDelta  = newEncoderPos - oldEncoderPos;
    oldEncoderPos         = newEncoderPos;
    if (encoderDelta) {
        current_scene->onEncoder(encoderDelta);
    }

    bool this_dial = dialButton.active();
    if (this_dial != last_dial) {
        last_dial = this_dial;
        if (this_dial) {
            current_scene->onDialButtonPress();
        } else {
            current_scene->onDialButtonRelease();
        }
    }

    bool this_red = redButton.active();
    if (this_red != last_red) {
        last_red = this_red;
        if (this_red) {
            current_scene->onRedButtonPress();
        } else {
            current_scene->onRedButtonRelease();
        }
    }

    bool this_green = greenButton.active();
    if (this_green != last_green) {
        last_green = this_green;
        if (this_green) {
            current_scene->onGreenButtonPress();
        } else {
            current_scene->onGreenButtonRelease();
        }
    }

    auto this_touch = M5Dial.Touch.getDetail();
    if (this_touch.state != last_touch_state) {
        last_touch_state = this_touch.state;
        if (this_touch.state != m5::touch_state_t::touch_end) {
            M5Dial.Speaker.tone(1800, 200);
            current_scene->onTouchPress(this_touch);
        } else {
            current_scene->onTouchRelease(this_touch);
        }
    }
}

// Helpful function to rotate through an aaray of numbers
// Example:  rotateNumberLoop(2, 1, 0, 2); would change the current value to 0
void rotateNumberLoop(int& currentVal, int increment, int min, int max) {
    currentVal += increment;
    if (currentVal > max) {
        currentVal = min + (increment - 1);
    }
    if (currentVal < min) {
        currentVal = max - (increment + 1);
    }
}

// Helpful for debugging touch development.
String M5TouchStateName(m5::touch_state_t state_num) {
    static constexpr const char* state_name[16] = { "none", "touch", "touch_end", "touch_begin", "___", "hold", "hold_end", "hold_begin",
                                                    "___",  "flick", "flick_end", "flick_begin", "___", "drag", "drag_end", "drag_begin" };

    return String(state_name[state_num]);
}

String axisNumToString(int axis) {
    return String("XYZABC").substring(axis, axis + 1);
}

String floatToString(float val, int afterDecimal) {
    char buffer[20];
    dtostrf(val, 1, afterDecimal, buffer);
    String str(buffer);
    return str;
}

void send_line(const String& s, int timeout) {
    send_line(s.c_str(), timeout);
}
void send_line(const char* s, int timeout) {
    fnc_send_line(s, timeout);
}
