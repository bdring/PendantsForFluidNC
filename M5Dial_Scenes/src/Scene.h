// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "GrblParserC.h"
#include "Button.h"
#include "Drawing.h"

class Scene {
private:
    String _name;

public:
    Scene(const char* name) : _name(name) {}

    const String& name() { return _name; }

    virtual void savePrefs() {}
    virtual void onRedButtonPress() {}
    virtual void onRedButtonRelease() {}
    virtual void onGreenButtonPress() {}
    virtual void onGreenButtonRelease() {}
    virtual void onDialButtonPress() {}
    virtual void onDialButtonRelease() {}
    virtual void onTouchPress(m5::touch_detail_t) {}
    virtual void onTouchRelease(m5::touch_detail_t) {}
    virtual void onStateChange(state_t) {}
    virtual void onDROChange() {}
    virtual void onLimitsChange() {}
    virtual void onEncoder(int delta) {}
    virtual void display() {}
    virtual void init(void* arg) {}
};

void activate_scene(Scene* scene, void* arg = nullptr);
void push_scene(Scene* scene, void* arg = nullptr);
void pop_scene();

// helper functions

// Function to rotate through an aaray of numbers
// Example:  rotateNumberLoop(variable, 1, 0, 2)
// The variable can be integer or float.  If it is float, you need
// to cast increment, min, and max to float otherwise the compiler
// will try to use double and complain

template <typename T>
void rotateNumberLoop(T& currentVal, T increment, T min, T max) {
    currentVal += increment;
    if (currentVal > max) {
        currentVal = min;
    }
    if (currentVal < min) {
        currentVal = max;
    }
}

String M5TouchStateName(m5::touch_state_t state_num);

extern Scene* current_scene;

extern Button greenButton;
extern Button redButton;
extern Button dialButton;

void dispatch_events();
