// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Widgets.h"
#include "System.h"
#include "Drawing.h"

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
    drawBackground(NAVY);
    show_items();
    canvas.pushSprite(0, 0);
}
void Menu::onEncoder(int delta) {
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
    debugPort.printf("touch at %d %d\r\n", x, y);

    if ((x * x + y * y) > _dead_radius_sq) {
        return -1;  // In middle dead zone
    }
    if (x == 0) {
        x = 1;  // Don't divide by zero
    }
    int slope = y * 1024 / x;
    if (slope > _slopes[0]) {
        // Bottom or top item
        return _odd && y < 0 ? _num_slopes : 0;
    }
    size_t i;
    // Side items
    for (i = 1; i < _num_slopes; ++i) {
        if (slope > _slopes[i]) {
            return x > 0 ? i : i + _num_slopes;
        }
    }
    if (_odd) {
        // One of two bottom items straddling the -Y axis
        return x > 0 ? i : i + _num_slopes;
    }
    // Bottom or top item
    return y < 0 ? _num_slopes : 0;
}
