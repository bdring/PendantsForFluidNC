// Copyright (c) 2023 - Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <Arduino.h>
#include "Scene.h"
#include <math.h>
#include <vector>

typedef int color_t;
typedef void (*callback_t)(void*);

void do_nothing(void* arg);
class Item {
protected:
    String     _name;
    bool       _highlighted = false;
    bool       _disabled    = false;
    bool       _hidden      = false;
    callback_t _callback    = nullptr;
    Scene*     _scene       = nullptr;

public:
    Item(const String& name, callback_t callback = do_nothing) : _name(name), _callback(callback) {}
    Item(const char* name, callback_t callback = do_nothing) : _name(name), _callback(callback) {}
    Item(const char* name, Scene* scene) : _name(name), _callback(nullptr), _scene(scene) {}
    Item() : Item("") {}

    virtual void show(const Point& where) {};

    virtual void invoke(void* arg = nullptr) {
        if (_disabled || _hidden) {
            return;
        }
        if (_scene) {
            push_scene(_scene, arg);
            return;
        }
        if (_callback) {
            _callback(arg);
            return;
        }
    };

    String name() { return _name; }
    void   highlight() { _highlighted = true; }
    void   unhighlight() { _highlighted = false; }
    bool   highlighted() { return _highlighted; }
    void   disable() { _disabled = true; }
    void   enable() { _disabled = false; }
    bool   enabled() { return !_disabled; }
    bool   disabled() { return _disabled; }
    void   hide() { _hidden = true; }
    void   unhide() { _hidden = false; }
    bool   hidden() { return _hidden; }

    void set_action(callback_t callback) { _callback = callback; }
};

class EmptyItem : public Item {
public:
    EmptyItem() { hide(); }
};

class RoundButton : public Item {
private:
    color_t _hl_fill_color;
    color_t _outline_color;
    color_t _hl_outline_color;

public:
    int     _radius;
    color_t _fill_color;
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
    RoundButton(
        const char* name, Scene* scene, int radius, color_t fill_color, color_t hl_fill_color, color_t outline_color, color_t hl_outline_color) :
        Item(name, scene),
        _radius(radius), _fill_color(fill_color), _hl_fill_color(hl_fill_color), _outline_color(outline_color),
        _hl_outline_color(hl_outline_color) {}
    void show(const Point& where) override;
};

class ImageButton : public Item {
private:
    String  _filename;
    int     _radius;
    color_t _outline_color;

public:
    ImageButton(const char* name, callback_t callback, const char* filename, int radius, color_t outline_color = WHITE) :
        Item(name, callback), _filename(filename), _radius(radius), _outline_color(outline_color) {}

    ImageButton(const char* name, Scene* scene, const char* filename, int radius, color_t outline_color = WHITE) :
        Item(name, scene), _filename(filename), _radius(radius), _outline_color(outline_color) {}

    void show(const Point& where) override;
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
    void show(const Point& where) override;
};

class Menu : public Scene {
private:
    void show_items() {
        for (size_t i = 0; i < _items.size(); ++i) {
            _items[i]->show(_positions[i]);
        }
        _items[_selected]->show(_positions[_selected]);
    }

    int       _num_items        = 0;
    int       _encoder_accum    = 0;
    const int encoder_threshold = 4;

public:
    std::vector<Point> _positions;
    std::vector<Item*> _items;

    int _selected = 0;

    Menu(const char* name) : Scene(name) {}

    Menu(const char* name, int num_items) : Scene(name), _num_items(num_items) {
        _items.reserve(num_items);
        _positions.reserve(num_items);
    }

    Item* selectedItem() { return _items[_selected]; }

    int num_items() { return _num_items; }

    void reDisplay();

    virtual void menuBackground() {}
    virtual int  touchedItem(int x, int y) { return -1; }
    virtual void rotate(int delta);

    void onEncoder(int delta) override {
        _encoder_accum += delta;
        if (abs(_encoder_accum) >= encoder_threshold) {
            rotate(_encoder_accum / encoder_threshold);
            // _encoder_accum %= encoder_threshold;
            _encoder_accum = 0;
        }
    }

    void setPosition(int item_num, Point position) { _positions[item_num] = position; }
    void setItem(int item_num, Item* item) { _items[item_num] = item; }
    void addItem(Item* item, Point position = { 0, 0 }) {
        _items.push_back(item);
        _positions.push_back(position);
        ++_num_items;
    }

    void init(void* arg) override {
        if (_selected != -1) {
            _items[_selected]->highlight();
        }
    }

    void select(int item) {
        if (item == -1 || item >= _num_items) {
            return;
        }
        if (_items[item]->hidden()) {
            return;
        }
        if (_selected != -1) {
            _items[_selected]->unhighlight();
        }
        _selected = item;
        _items[item]->highlight();
        reDisplay();
    }
    void onTouchRelease(int x, int y) override { select(touchedItem(x, y)); }
    void invoke() { _items[_selected]->invoke(); }
};

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
    void onTouchRelease(int x, int y) override;
    void onTouchHold(int x, int y) override;
    void onTouchFlick(int x, int y, int dx, int dy) override;
    void onDialButtonPress() override;
    void addItem(Item* item) {
        Menu::addItem(item);
        calculatePositions();
    }
    int touchedItem(int x, int y) override;
};
