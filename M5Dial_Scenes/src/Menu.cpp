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
    text(name().substring(0, 1), x, y, _highlighted ? MAROON : WHITE, MEDIUM);
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

    int layout_radius = display.width() / 2 - _item_radius - 2;
    angle             = M_PI / 2;
    for (size_t i = 0; i < num_items(); i++) {
        xy_t center = { (int)(cosf(angle) * layout_radius), (int)(sinf(angle) * layout_radius) };
        setPosition(i, center);
        angle -= theta;
    }
}

int PieMenu::touchedItem(int x, int y) {
    // Convert from screen coordinates to 0,0 in the center
    x = x - display.width() / 2;
    y = -(y - display.height() / 2);

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
        // debugPort.printf("slope %d [] %d\r\n", slope, _slopes[i]);
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
    int center_x = display.width() / 2;
    int center_y = display.height() / 2;
    text(name(), center_x, center_y - 24, YELLOW, TINY);
    text(selectedItem()->name(), center_x, center_y + 8, WHITE, SMALL);
}

void PieMenu::onTouchFlick(int x, int y, int dx, int dy) {
    // debugPort.printf("Flick %d %d %d %d\r\n", x, y, dx, dy);
    int item = touchedItem(x, y);
    if (item != -1) {
        select(item);
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
        invoke();
    }
}
