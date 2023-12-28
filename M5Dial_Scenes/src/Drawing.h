// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Factors for drawing parts of the pendant display

#pragma once
#include <Arduino.h>
#include "M5Dial.h"
#include "FluidNCModel.h"
#include "Text.h"

class Stripe {
private:
    int       _x;
    int       _y;
    int       _width;
    int       _height;
    fontnum_t _font;
    const int _text_inset = 5;
    int       text_left_x() { return _x + _text_inset; }
    int       text_center_x() { return _x + _width / 2; }
    int       text_right_x() { return _x + _width - _text_inset; }
    int       text_middle_y() { return _y + _height / 2 + 2; }
    int       widget_left_x() { return _x; }

public:
    Stripe(int x, int y, int width, int height, fontnum_t font);
    void draw(const String& left, const String& right, bool highlighted, int left_color = WHITE);
    void draw(const String& center, bool highlighted);
    int  y() { return _y; }
    int  gap() { return _height + 1; }
};
class LED {
private:
    int _x;
    int _y;
    int _radius;
    int _gap;

public:
    LED(int x, int y, int radius, int gap) : _x(x), _y(y), _radius(radius), _gap(gap) {}
    void draw(bool highlighted);
};

class DRO : public Stripe {
public:
    DRO(int x, int y, int width, int height) : Stripe(x, y, width, height, MEDIUM_MONO) {}
    void draw(int axis, bool highlight);
};

// draw stuff
void drawBackground(int color);
void drawStatus();
void drawOutlinedRect(int x, int y, int width, int height, int bgcolor, int outlinecolor);
void drawButtonLegends(const String& red, const String& green, const String& orange);
void drawMenuTitle(const String& name);
void refreshDisplay();
void showError();
void drawThickCircle(int x, int y, int outsideRaius, int thickness, int color);
void drawCapsule(int y, int width, int height, int color);
