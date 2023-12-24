// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Drawing.h"
#include "alarm.h"
#include <map>

void drawBackground(int color) {
    canvas.fillSprite(color);
}

void drawOutlinedRect(int x, int y, int width, int height, int bgcolor, int outlinecolor) {
    canvas.fillRoundRect(x, y, width, height, 5, bgcolor);
    canvas.drawRoundRect(x, y, width, height, 5, outlinecolor);
}

// clang-format off
std::map<state_t, int> stateColors = {
    { Idle,        WHITE },
    { Alarm,       RED },
    { CheckMode,   WHITE },
    { Homing,      CYAN },
    { Cycle,       GREEN },
    { Hold,        YELLOW },
    { Jog,         CYAN },
    { SafetyDoor,  WHITE },
    { Sleep,       WHITE },
    { ConfigAlarm, WHITE },
    { Critical,    WHITE },
};
// clang-format on

void drawStatus() {
    static constexpr int x      = 100;
    static constexpr int y      = 24;
    static constexpr int width  = 140;
    static constexpr int height = 36;

    canvas.fillRoundRect(CENTER - width / 2, y, width, height, 5, stateColors[state]);
    if (state == Alarm) {
        centered_text(stateString, y + height / 2 - 4, BLACK, SMALL);
        centered_text(alarm_name[lastAlarm], y + height / 2 + 12, BLACK);
    } else {
        centered_text(stateString, y + height / 2 + 3, BLACK, MEDIUM);
    }
}

void showError() {
    if (millis() < errorExpire) {
        //errorCounter--;
        canvas.fillCircle(120, 120, 95, RED);
        drawThickCircle(120, 120, 95, 5, WHITE);
        centered_text("Error " + String(lastError), 130, WHITE, MEDIUM);
    }
}

Stripe::Stripe(int x, int y, int width, int height, fontnum_t font) : _x(x), _y(y), _width(width), _height(height), _font(font) {}

void Stripe::draw(const String& left, const String& right, bool highlighted, int left_color) {
    drawOutlinedRect(_x, _y, _width, _height, highlighted ? BLUE : NAVY, WHITE);
    if (left.length()) {
        text(left, text_left_x(), text_middle_y(), left_color, _font, middle_left);
    }
    if (right.length()) {
        text(right, text_right_x(), text_middle_y(), WHITE, _font, middle_right);
    }
    _y += gap();
}
void Stripe::draw(const String& center, bool highlighted) {
    drawOutlinedRect(_x, _y, _width, _height, highlighted ? BLUE : NAVY, WHITE);
    text(center, text_center_x(), text_middle_y(), WHITE, _font, middle_center);
    _y += gap();
}

#define PUSH_BUTTON_LINE 212
#define DIAL_BUTTON_LINE 228

// This shows on the display what the button currently do.
void drawButtonLegends(const String& red, const String& green, const String& orange) {
    text(red, 80, PUSH_BUTTON_LINE, RED);
    text(green, 160, PUSH_BUTTON_LINE, GREEN);
    centered_text(orange, DIAL_BUTTON_LINE, ORANGE);
}

void LED::draw(bool highlighted) {
    canvas.fillCircle(_x, _y, _radius, (highlighted) ? GREEN : DARKGREY);
    canvas.drawCircle(_x, _y, _radius, WHITE);
    _y += _gap;
}

void drawMenuTitle(const String& name) {
    centered_text(name, 12);
}

void drawThickCircle(int x, int y, int outsideRaius, int thickness, int color) {
    for (int i = 0; i < thickness; i++) {
        canvas.drawCircle(x, y, outsideRaius - i, color);
    }
}

void refreshDisplay() {
    M5Dial.Display.startWrite();
    canvas.pushSprite(0, 0);
    M5Dial.Display.endWrite();
}

void drawMenuView(std::vector<String> labels, int start, int selected) {}
