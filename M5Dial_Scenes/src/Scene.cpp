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
    current_scene->reDisplay();
}
void push_scene(Scene* scene, void* arg) {
    scene_stack.push_back(current_scene);
    activate_scene(scene, arg);
}
void pop_scene(void* arg) {
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
    int32_t encoderDelta = encoder.readAndReset();
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

    auto this_touch = touch.getDetail();
    if (this_touch.state != last_touch_state) {
        last_touch_state = this_touch.state;
        // debugPort.printf("Touch %d\r\n", this_touch.state);
        if (this_touch.state == m5::touch_state_t::touch) {
            speaker.tone(1800, 50);
            current_scene->onTouchPress(this_touch.x, this_touch.y);
        } else if (this_touch.wasClicked()) {
            current_scene->onTouchRelease(this_touch.x, this_touch.y);
        } else if (this_touch.wasHold()) {
            current_scene->onTouchHold(this_touch.x, this_touch.y);
        } else if (this_touch.state == m5::touch_state_t::flick_end) {
            current_scene->onTouchFlick(this_touch.x, this_touch.y, this_touch.distanceX(), this_touch.distanceY());
        }
    }
}
