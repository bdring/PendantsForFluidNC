// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// Factors for drawing parts of the pendant display

#pragma once
#include "FluidNCModel.h"
#include "Text.h"

class Stripe {
private:
protected:
    int       _x;
    int       _y;
    int       _width;
    int       _height;
    fontnum_t _font;
    const int _text_inset = 5;

    int text_left_x() { return _x + _text_inset; }
    int text_center_x() { return _x + _width / 2; }
    int text_right_x() { return _x + _width - _text_inset; }
    int text_middle_y() { return _y + _height / 2 + 2; }
    int widget_left_x() { return _x; }

public:
    Stripe(int x, int y, int width, int height, fontnum_t font);
    void draw(const String& left, const String& right, bool highlighted, int left_color = WHITE);
    void draw(const String& center, bool highlighted);
    void draw(float n, int n_decimals, int hl_digit, bool highlighted);
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
    void putCharacter(char c, int& x, int& y, int color);
    void putDigit(unsigned int& u, int& x, int& y, int color);
    void fancyNumber(float fn, int n_decimals, int hl_digit, int x, int y, int text_color, int hl_text_color);

public:
    DRO(int x, int y, int width, int height) : Stripe(x, y, width, height, MEDIUM_MONO) {}
    void draw(int axis, bool highlight);
    void draw(int axis, int n_decimals, int hl_digit, bool highlight);
};

// draw stuff
// Routines that take Point as an argument work in a coordinate
// space where 0,0 is at the center of the display and +Y is up

void drawBackground(int color);
void drawStatus();

void drawFilledCircle(int x, int y, int radius, int fillcolor);
void drawFilledCircle(Point xy, int radius, int fillcolor);

void drawCircle(int x, int y, int radius, int thickness, int outlinecolor);
void drawCircle(Point xy, int radius, int thickness, int outlinecolor);

void drawOutlinedCircle(int x, int y, int radius, int fillcolor, int outlinecolor);
void drawOutlinedCircle(Point xy, int radius, int fillcolor, int outlinecolor);

void drawRect(int x, int y, int width, int height, int radius, int bgcolor);
void drawRect(Point xy, int width, int height, int radius, int bgcolor);
void drawRect(Point xy, Point wh, int radius, int bgcolor);

void drawOutlinedRect(int x, int y, int width, int height, int bgcolor, int outlinecolor);
void drawOutlinedRect(Point xy, int width, int height, int bgcolor, int outlinecolor);

void drawButtonLegends(const String& red, const String& green, const String& orange);
void drawMenuTitle(const String& name);

void drawPngFile(const String& filename, int x, int y);
void drawPngFile(const String& filename, Point xy);
void drawPngBackground(const String& filename);

void refreshDisplay();

void showImageFile(const char* name, int x, int y, int width, int height);
void showError();
