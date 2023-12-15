// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "M5Dial.h"
#include "GrblParserC.h"
#include "Button.h"

// Same states as FluidNC.
enum state_t {
    Idle = 0,     // Must be zero.
    Alarm,        // In alarm state. Locks out all g-code processes. Allows settings access.
    CheckMode,    // G-code check mode. Locks out planner and motion only.
    Homing,       // Performing homing cycle
    Cycle,        // Cycle is running or motions are being executed.
    Hold,         // Active feed hold
    Jog,          // Jogging mode.
    SafetyDoor,   // Safety door is ajar. Feed holds and de-energizes system.
    Sleep,        // Sleep state.
    ConfigAlarm,  // You can't do anything but fix your config file.
    Critical,     // You can't do anything but reset with CTRL-x or the reset button
};

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
};

void activate_scene(Scene* scene);
void push_scene(Scene* scene);
void pop_scene();

void send_line(const String& s, int timeout = 2000);
void send_line(const char* s, int timeout = 2000);

extern state_t state;

extern pos_t              myAxes[6];
extern bool               myLimitSwitches[6];
extern bool               myProbeSwitch;
extern String             myFile;
extern file_percent_t     myPercent;
extern override_percent_t myFro;
extern int                lastAlarm;

extern M5Canvas canvas;

// draw stuff
void drawStartScreen();
void drawDRO(int x, int y, int width, int axis, float value, bool highlighted);
void drawStatus();
void drawButton(int x, int y, int width, int height, int charSize, String text, bool highlighted);
void drawLed(int x, int y, int radius, bool active);
void buttonLegends(String red, String green, String orange);
void menuTitle();
void refreshDisplaySprite();

// helper functions
void   rotateNumberLoop(int& currentVal, int increment, int min, int max);
void   feedRateRotator(int& rate, bool up);
String M5TouchStateName(m5::touch_state_t state_num);
String floatToString(float val, int afterDecimal);
String axisNumToString(int axis);
void   debug(const char* info);

extern Scene* current_scene;

extern Button greenButton;
extern Button redButton;
extern Button dialButton;

void dispatch_events();

void log_msg(const String& s);
