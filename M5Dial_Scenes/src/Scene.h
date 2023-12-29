// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "GrblParserC.h"
#include "Button.h"
#include "Drawing.h"
#include "nvs_flash.h"

class Scene {
private:
    String _name;

    nvs_handle_t _prefs {};

public:
    Scene(const char* name) : _name(name) {}

    const String& name() { return _name; }

    virtual void onRedButtonPress() {}
    virtual void onRedButtonRelease() {}
    virtual void onGreenButtonPress() {}
    virtual void onGreenButtonRelease() {}
    virtual void onDialButtonPress() {}
    virtual void onDialButtonRelease() {}
    virtual void onTouchPress(int x, int y) {}
    virtual void onTouchRelease(int x, int y) {}
    virtual void onTouchHold(int x, int y) {}
    virtual void onTouchFlick(int x, int y, int dx, int dy) {}
    virtual void onStateChange(state_t) {}
    virtual void onDROChange() {}
    virtual void onLimitsChange() {}
    virtual void onEncoder(int delta) {}
    virtual void reDisplay() {}
    virtual void init(void* arg = nullptr) {}

    bool initPrefs();

    void setPref(const char* name, int value);
    void getPref(const char* name, int* value);
    void setPref(const char* name, float value);
    void getPref(const char* name, float* value);
    void setPref(const char* name, int axis, int value);
    void getPref(const char* name, int axis, int* value);
};

void activate_scene(Scene* scene, void* arg = nullptr);
void push_scene(Scene* scene, void* arg = nullptr);
void pop_scene(void* arg = nullptr);

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

extern Scene* current_scene;

extern Button greenButton;
extern Button redButton;
extern Button dialButton;

void dispatch_events();
