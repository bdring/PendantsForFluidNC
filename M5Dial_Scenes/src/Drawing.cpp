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

std::map<state_t, int> stateColors = {
    { Idle, WHITE }, { Alarm, RED },        { CheckMode, WHITE }, { Homing, CYAN },       { Cycle, GREEN },    { Hold, YELLOW },
    { Jog, CYAN },   { SafetyDoor, WHITE }, { Sleep, WHITE },     { ConfigAlarm, WHITE }, { Critical, WHITE },
};

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

void drawButton(int x, int y, int width, int height, fontnum_t fontnum, const String& msg, bool highlighted) {
    drawOutlinedRect(x, y, width, height, highlighted ? BLUE : NAVY, WHITE);
    text(msg, x + width / 2, y + height / 2 + 2, WHITE, fontnum);
}

#define PUSH_BUTTON_LINE 212
#define DIAL_BUTTON_LINE 228

// This shows on the display what the button currently do.
void drawButtonLegends(const String& red, const String& green, const String& orange) {
    text(red, 80, PUSH_BUTTON_LINE, RED);
    text(green, 160, PUSH_BUTTON_LINE, GREEN);
    centered_text(orange, DIAL_BUTTON_LINE, ORANGE);
}

void drawLed(int x, int y, int radius, bool active) {
    canvas.fillCircle(x, y, radius, (active) ? GREEN : DARKGREY);
    canvas.drawCircle(x, y, radius, WHITE);
}

void drawMenuTitle(const String& name) {
    centered_text(name, 12);
}

void refreshDisplay() {
    M5Dial.Display.startWrite();
    canvas.pushSprite(0, 0);
    M5Dial.Display.endWrite();
}
