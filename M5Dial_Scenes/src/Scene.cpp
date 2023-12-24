// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#define DEBUG_TO_FNC
#define DEBUG_TO_USB

#include <Arduino.h>
#include "Scene.h"

Scene* current_scene;

Button greenButton;
Button redButton;
Button dialButton;

std::vector<Scene*> scene_stack;

void activate_scene(Scene* scene, void* arg) {
    current_scene = scene;
    current_scene->init(arg);
    current_scene->display();
}
void push_scene(Scene* scene, void* arg) {
    scene_stack.push_back(current_scene);
    activate_scene(scene, arg);
}
void pop_scene() {
    if (scene_stack.size()) {
        Scene* last_scene = scene_stack.back();
        scene_stack.pop_back();
        activate_scene(last_scene);
    }
}

void dispatch_events() {
    static int32_t           oldEncoderPos    = 0;
    static m5::touch_state_t last_touch_state = {};

    M5Dial.update();
    int32_t encoderDelta = M5Dial.Encoder.readAndReset();
    if (encoderDelta) {
        current_scene->onEncoder(encoderDelta);
    }

    bool this_button;
    if (dialButton.changed(this_button)) {
        if (this_button) {
            current_scene->onDialButtonPress();
        } else {
            current_scene->onDialButtonRelease();
        }
    }

    if (redButton.changed(this_button)) {
        if (this_button) {
            current_scene->onRedButtonPress();
        } else {
            current_scene->onRedButtonRelease();
        }
    }

    if (greenButton.changed(this_button)) {
        if (this_button) {
            current_scene->onGreenButtonPress();
        } else {
            current_scene->onGreenButtonRelease();
        }
    }

    auto this_touch = M5Dial.Touch.getDetail();
    if (this_touch.state != last_touch_state) {
        last_touch_state = this_touch.state;
        if (this_touch.state != m5::touch_state_t::touch_end) {
            current_scene->onTouchPress(this_touch);
        } else {
            current_scene->onTouchRelease(this_touch);
        }
    }
}


// Helpful for debugging touch development.
String M5TouchStateName(m5::touch_state_t state_num) {
    static constexpr const char* state_name[16] = { "none", "touch", "touch_end", "touch_begin", "___", "hold", "hold_end", "hold_begin",
                                                    "___",  "flick", "flick_end", "flick_begin", "___", "drag", "drag_end", "drag_begin" };

    return String(state_name[state_num]);
}
