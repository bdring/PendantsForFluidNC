// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "PieMenu.h"
#include "System.h"
#include "Drawing.h"

void PieMenu::calculatePositions() {
    _num_slopes = num_items() / 2;  // Rounded down

    _slopes.clear();
    float theta      = 2 * M_PI / num_items();
    float half_theta = theta / 2.0;

    float angle = M_PI / 2 - half_theta;
    for (size_t i = 0; i < _num_slopes; i++) {
        int slope = tanf(angle) * 1024;
        _slopes.push_back(slope);
        angle -= theta;
    }

    int layout_radius = display.width() / 2 - _item_radius - 3;
    angle             = M_PI / 2;
    for (size_t i = 0; i < num_items(); i++) {
        Point center = { (int)(cosf(angle) * layout_radius), (int)(sinf(angle) * layout_radius) };
        setPosition(i, center);
        angle -= theta;
    }
}

int PieMenu::touchedItem(int x, int y) {
    // Convert from screen coordinates to 0,0 in the center
    Point ctr = Point { x, y }.from_display();

    fnc_realtime(StatusReport);  // used to update if status is out of sync

    x = ctr.x;
    y = ctr.y;

    int dead_radius = display.width() / 2 - _item_radius * 2;

    if ((x * x + y * y) < (dead_radius * dead_radius)) {
        return -1;  // In middle dead zone
    }
    if (x == 0) {
        x = 1;  // Don't divide by zero
    }
    int slope = y * 1024 / x;
    if (x < 0) {
        slope = -slope;
    }
    if (slope > _slopes[0]) {
        // Top item
        return 0;
    }
    // Side items
    size_t i;
    for (i = 1; i < _num_slopes; ++i) {
        if (slope > _slopes[i]) {
            return x > 0 ? i : num_items() - i;
        }
    }
    // If num_items() is even, return the bottom item  (i == num_items()-i)
    // If it is odd, return one of two bottom items stradding -Y axis

    reDisplay();
    return x > 0 ? i : num_items() - i;
}
void PieMenu::menuBackground() {
    background();
    drawStatusTiny(91);
    text(selectedItem()->name(), { 0, -8 }, WHITE, SMALL);
}

void PieMenu::onTouchFlick(int x, int y, int dx, int dy) {
    int item = touchedItem(x, y);
    if (item != -1) {
        select(item);
        ackBeep();
        invoke();
    }
}
void PieMenu::onDialButtonPress() {
    invoke();
}

void PieMenu::onTouchHold(int x, int y) {
    int item = touchedItem(x, y);
    if (item != -1) {
        select(item);
    }
}

void PieMenu::onTouchRelease(int x, int y) {
    int item = touchedItem(x, y);
    if (item != -1) {
        select(item);
        invoke();
    }
}

void PieMenu::onStateChange(state_t state) {
    reDisplay();
}
