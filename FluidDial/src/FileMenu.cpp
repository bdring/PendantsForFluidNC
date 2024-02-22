#if 0
// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#    include "FileMenu.h"
#    include <math.h>
#    include <string>

void FileItem::invoke(void* arg) {
    // doFileScreen(_name);
}
std::string FileItem::baseName() {
    if (isDirectory()) {
        return name().substr(0, name().length() - 1);
    }
    auto dotpos = name().rfind('.');
    if (dotpos != std::string::npos) {
        return name().substr(0, dotpos);
    }
    return name();
}
void FileItem::show(const Point& where) {
    int         color = WHITE;
    std::string s     = baseName();
    if (isDirectory()) {
        color = YELLOW;
        s += " >";
    }

    if (_highlighted) {
        Point wh { 200, 45 };
        drawRect(where, wh, 20, color);
        text(s.c_str(), where, BLACK, MEDIUM, middle_center);
    } else {
        text(s.c_str(), where, WHITE, SMALL, middle_center);
    }
}
void FileMenu::reDisplay() {
    menuBackground();
    if (_selected > 1) {
        _items[_selected - 2]->show({ 0, 70 });
    }
    if (_selected > 0) {
        _items[_selected - 1]->show({ 0, 40 });
    }
    _items[_selected]->show({ 0, 0 });
    if (_selected < num_items() - 1) {
        _items[_selected + 1]->show({ 0, -40 });
    }
    if (_selected < num_items() - 2) {
        _items[_selected + 2]->show({ 0, -70 });
    }
    refreshDisplay();
}
void FileMenu::rotate(int delta) {
    if (_selected == 0 && delta <= 0) {
        return;
    }
    if (_selected == num_items() && delta >= 0) {
        return;
    }
    if (_selected != -1) {
        _items[_selected]->unhighlight();
    }
    _selected += delta;
    if (_selected < 0) {
        _selected = 0;
    }
    if (_selected >= num_items()) {
        _selected = num_items() - 1;
    }
    _items[_selected]->highlight();
    reDisplay();
}

int FileMenu::touchedItem(int x, int y) {
    return -1;
}
void FileMenu::menuBackground() {
    drawPngBackground("/filesbg.png");
    //    drawBackground(NAVY);

    text(_dirname, { 0, 100 }, YELLOW, MEDIUM);

    // Draw dot showing the selected file
    if (num_items() > 1) {
        int   span = 100; // degrees
        int   dtheta = span * _selected / (num_items() - 1);
        int   theta  = (span/2) - dtheta;
        int dx, dy;
        r_degrees_to_xy(110, theta, &dx, &dy);

        drawFilledCircle({ dx, dy }, 8, WHITE);
    }
}
void FileMenu::onTouchFlick(int x, int y, int dx, int dy) {
    if (dx < -60) {
        pop_scene();
    }
    if (dy < -60) {
        // up_directory();
    }
}
#endif
