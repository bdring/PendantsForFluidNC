// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Button.h"

const int debounce_ms = 10;

void Button::init(uint8_t pin, bool active_low) {
    _pin_num    = pin;
    _active_low = active_low;
    pinMode(_pin_num, INPUT_PULLUP);
    _last_value = read();
    _delaying   = false;
}

bool Button::read() {
    return digitalRead(_pin_num) ^ _active_low;
}

bool Button::changed(bool& value) {
    if (_delaying) {
        if (millis() - _timestamp < debounce_ms) {
            return false;
        }
        _delaying = false;
    }
    bool new_value = read();
    value          = new_value;
    if (new_value != _last_value) {
        _timestamp  = millis();
        _delaying   = true;
        _last_value = new_value;
        return true;
    }
    return false;
}
