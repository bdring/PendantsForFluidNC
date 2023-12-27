// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "FileMenu.h"
#include <math.h>

void FileItem::invoke(void* arg) {
    // doFileScreen(_name);
}
void FileItem::show(const xy_t& where) {
    int width  = 180;
    int height = 30;

    if (_highlighted) {
        width += 20;
        height += 15;
    }

    int    color = WHITE;
    String s(name());
    if (s.endsWith("/")) {
        color = YELLOW;
        s     = s.substring(0, s.length() - 1) + " >";
    } else {
        int dotpos = s.lastIndexOf('.');
        if (dotpos != -1) {
            s = s.substring(0, dotpos);
        }
    }

    // Canvas coordinates start at the top.
    int cx = display.width() / 2 + where.x;
    int cy = display.height() / 2 - where.y;

    int x = cx - width / 2;
    int y = cy - height / 2;

    debugPort.printf("show %s %d %d %d %d\r\n", s, x, y, width, height);
    if (_highlighted) {
        canvas.fillRoundRect(x, y, width, height, 20, color);
    }
    text(s, cx, cy, _highlighted ? BLACK : WHITE, _highlighted ? MEDIUM : SMALL, middle_center);
}
void FileMenu::reDisplay() {
    menuBackground();
    if (_selected > 1) {
        _items[_selected - 2]->show({ 0, 70 });
    }
    if (_selected > 0) {
        _items[_selected - 1]->show({ 0, 40 });
    }
    debugPort.printf("nitems %d %d\r\n", num_items(), _selected);
    _items[_selected]->show({ 0, 0 });
    if (_selected < num_items() - 1) {
        _items[_selected + 1]->show({ 0, -40 });
    }
    if (_selected < num_items() - 2) {
        _items[_selected + 2]->show({ 0, -70 });
    }
    canvas.pushSprite(0, 0);
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
    canvas.drawPngFile(LittleFS, "/filesbg.png", 0, 0, display.width(), display.height());
    //    drawBackground(NAVY);
    int center_x = display.width() / 2;
    int center_y = display.height() / 2;
    text(_dirname, center_x, center_y - 100, YELLOW, MEDIUM);

    if (num_items() > 1) {
        int   radius = 110;
        float span   = M_PI / 1.8;
        float dtheta = span * _selected / (num_items() - 1);
        float theta  = M_PI / 3.6 - dtheta;
        int   x      = center_x + (int)(radius * cosf(theta));
        int   y      = center_y - (int)(radius * sinf(theta));
        debugPort.printf("%d %d %f %f \r\n", x, y, dtheta, theta);
        float theta1 = M_PI / 4;
        canvas.fillCircle(x, y, 8, WHITE);
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
