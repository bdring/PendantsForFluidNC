// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Menu.h"
#include "System.h"
#include "Drawing.h"

void do_nothing(void* foo) {}

void RoundButton::show(const xy_t& where) {
    // Canvas coordinates start at the top.
    int x = where.x + display.width() / 2;
    int y = (-where.y) + display.height() / 2;
    canvas.fillCircle(x, y, _radius, _highlighted ? _hl_fill_color : _fill_color);
    canvas.drawCircle(x, y, _radius, _highlighted ? _hl_outline_color : _outline_color);
    text(name(), x, y, WHITE, MEDIUM);
}
void ImageButton::show(const xy_t& where) {
    display.drawPngFile(LittleFS, _filename, where.x, where.y, display.width(), display.height(), 0, 0, 0.0f, 0.0f, datum_t::middle_center);
    if (_highlighted) {
        // What?
    }
}
void RectangularButton::show(const xy_t& where) {
    drawOutlinedRect(where.x, where.y, _width, _height, _highlighted ? BLUE : _outline_color, _bg_color);
    text(_text, where.x, where.y, _text_color, SMALL);
}

void Menu::reDisplay() {
    canvas.createSprite(display.width(), display.height());
    menuBackground();
    show_items();
    canvas.pushSprite(0, 0);
}
void Menu::rotate(int delta) {
    if (_selected != -1) {
        _items[_selected]->unhighlight();
    }
    _selected += delta;
    while (_selected < 0) {
        _selected += _num_items;
    }
    while (_selected >= _num_items) {
        _selected -= _num_items;
    }
    _items[_selected]->highlight();
    reDisplay();
}

int PieMenu::touchedItem(int x, int y) {
    // Convert from screen coordinates to 0,0 in the center
    x = x - display.width() / 2;
    y = -(y - display.height() / 2);

    if ((x * x + y * y) < _dead_radius_sq) {
        // return -1;  // In middle dead zone
        rotate(1);
        return -1;
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
        debugPort.printf("slope %d [] %d\r\n", slope, _slopes[i]);
        if (slope > _slopes[i]) {
            return x > 0 ? i : num_items() - i;
        }
    }
    // If num_items() is even, return the bottom item  (i == num_items()-i)
    // If it is odd, return one of two bottom items stradding -Y axis
    return x > 0 ? i : num_items() - i;
}
void PieMenu::menuBackground() {
    drawBackground(NAVY);
    text(selectedItem()->name(), display.width() / 2, display.height() / 2, WHITE, MEDIUM);
}

void PieMenu::onTouchFlick(int x, int y) {
    debugPort.printf("Flick %d %d\r\n", x, y);
    if (x > 50) {
        invoke();
    } else if (x < 50) {
        pop_scene();
    }
}
void PieMenu::onTouchHold(int x, int y) {
    x = x - display.width() / 2;
    y = -(y - display.height() / 2);

    if ((x * x + y * y) < _dead_radius_sq) {
        debugPort.printf("Invoking %s\r\n", selectedItem()->name());
        invoke();
    }
}
