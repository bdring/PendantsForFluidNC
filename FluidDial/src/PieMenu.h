// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Menu.h"

// Pie menus were originally invented by Don Hopkins at Sun Microsystems
class PieMenu : public Menu {
private:
    int _item_radius;
    int _num_slopes;

    std::vector<int> _slopes;  // Slopes of lines dividing switch positions

public:
    PieMenu(const char* name, int item_radius) : Menu(name), _item_radius(item_radius) {}
    PieMenu(const char* name, int item_radius, int num_items) : Menu(name, num_items), _item_radius(item_radius) { calculatePositions(); }
    void menuBackground() override;
    void calculatePositions();
    void onEncoder(int delta) override { Menu::onEncoder(delta); }
    void onTouchHold(int x, int y) override;
    void onTouchRelease(int x, int y) override;
    void onTouchFlick(int x, int y, int dx, int dy) override;
    void onDialButtonPress() override;
    void addItem(Item* item) {
        Menu::addItem(item);
        calculatePositions();
    }
    int  touchedItem(int x, int y) override;
    void onStateChange(state_t state) override;
};
