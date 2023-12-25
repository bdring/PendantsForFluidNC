// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>
#include "Scene.h"
#include <math.h>
#include <vector>

typedef int color_t;
typedef void (*callback_t)(void*);

struct xy_t {
    int x;
    int y;
};

class Item {
protected:
    bool       _highlighted;
    callback_t _callback;
    String     _name;

public:
    Item(const String& name, callback_t callback = nullptr) : _name(name), _highlighted(false) {}
    Item(const char* name, callback_t callback = nullptr) : _name(name), _highlighted(false) {}
    Item() : Item("") {}

    virtual void show(const xy_t& where) = 0;

    void invoke(void* arg = nullptr) { _callback(arg); };

    String name() { return _name; }
    void   highlight() { _highlighted = true; }
    void   unhighlight() { _highlighted = false; }
    void   set_action(callback_t callback) { _callback = callback; }
};

class RoundButton : public Item {
private:
    int     _radius;
    color_t _fill_color;
    color_t _hl_fill_color;
    color_t _outline_color;
    color_t _hl_outline_color;

public:
    RoundButton(const char* name,
                callback_t  callback,
                int         radius,
                color_t     fill_color,
                color_t     hl_fill_color,
                color_t     outline_color,
                color_t     hl_outline_color) :
        Item(name, callback),
        _radius(radius), _fill_color(fill_color), _hl_fill_color(hl_fill_color), _outline_color(outline_color),
        _hl_outline_color(hl_outline_color) {}
    void show(const xy_t& where) override;
};

class ImageButton : public Item {
private:
    String _filename;

public:
    ImageButton(const char* name, callback_t callback, const char* filename) : Item(name, callback), _filename(filename) {}
    void show(const xy_t& where) override;
};

class RectangularButton : public Item {
private:
    String _text;
    int    _width;
    int    _height;
    int    _radius;
    int    _bg_color;
    int    _text_color;
    int    _outline_color;

public:
    RectangularButton(const char* name,
                      callback_t  callback,
                      String&     text,
                      int         width,
                      int         height,
                      int         radius,
                      color_t     bg_color,
                      color_t     text_color,
                      color_t     outline_color) :
        Item(name, callback),
        _text(text), _width(width), _height(height), _radius(radius), _bg_color(bg_color), _text_color(text_color),
        _outline_color(outline_color) {}
    void show(const xy_t& where) override;
};

class Menu : public Scene {
private:
    std::vector<xy_t>  _positions;
    std::vector<Item*> _items;

    int _num_items = 0;

    void show_items() {
        for (size_t i = 0; i < _items.size(); ++i) {
            _items[i]->show(_positions[i]);
        }
    }

public:
    int _selected = 0;

    Menu(const char* name) : Scene(name) {}

    Menu(const char* name, int num_items) : Scene(name), _num_items(num_items) {
        _items.reserve(num_items);
        _positions.reserve(num_items);
    }

    int  num_items() { return _num_items; }
    void select(int n) { _selected = n; }

    void reDisplay();

    virtual int touchedItem(int x, int y) = 0;

    void onEncoder(int delta) override;
    void setPosition(int item_num, xy_t position) { _positions[item_num] = position; }
    void setItem(int item_num, Item* item) { _items[item_num] = item; }
    void addItem(Item* item, xy_t position = { 0, 0 }) {
        _items.push_back(item);
        _positions.push_back(position);
        ++_num_items;
    }

    void onTouchPress(int x, int y) override {
        debugPort.printf("TouchPress at %d %d\r\n", x, y);
        if (_selected != -1) {
            _items[_selected]->unhighlight();
        }
        int selection = touchedItem(x, y);
        if (selection == -1 || selection >= _num_items) {
            // Leave _selection as-is
            return;
        }
        _selected = selection;
        _items[_selected]->highlight();
    }
    void invoke() { _items[_selected]->invoke(); }
};

// Pie menus were originally invented by Don Hopkins at Sun Microsystems
class PieMenu : public Menu {
private:
    int  _dead_radius_sq;
    int  _item_radius;
    int  _num_slopes;
    bool _odd;

    std::vector<int> _slopes;  // Slopes of lines dividing switch positions

public:
    PieMenu(const char* name, int dead_radius, int item_radius) :
        Menu(name), _dead_radius_sq(dead_radius * dead_radius), _item_radius(item_radius) {}
    PieMenu(const char* name, int dead_radius, int item_radius, int num_items) :
        Menu(name, num_items), _dead_radius_sq(dead_radius * dead_radius), _item_radius(item_radius) {
        calculatePositions();
    }
    void calculatePositions() {
        _odd        = num_items() % 1;
        _num_slopes = num_items() / 2;  // Rounded down

        _slopes.empty();
        float theta      = 2 * M_PI / num_items();
        float half_theta = theta / 2.0;

        float angle = M_PI / 2 - half_theta;
        for (size_t i = 0; i < _num_slopes; i++) {
            _slopes.push_back(tanf(angle) * 1024);
            angle -= theta;
        }

        angle = M_PI / 2;
        for (size_t i = 0; i < num_items(); i++) {
            xy_t center = { (int)(cosf(angle) * _item_radius), (int)(sinf(angle) * _item_radius) };
            setPosition(i, center);
            angle -= theta;
        }
    }
    void onEncoder(int delta) override { Menu::onEncoder(delta); }
    void addItem(Item* item) {
        Menu::addItem(item);
        calculatePositions();
    }
    int touchedItem(int x, int y) override;
    // XXX Need some way to display the selected item's name in
    // the middle.
};
