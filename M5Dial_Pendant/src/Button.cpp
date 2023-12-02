#include "Button.h"

void Button::init(uint8_t pin, bool activeLow) {
    pinMode(pin, INPUT_PULLUP);
    _pin_num = pin;
    _last_value = active();
    _active_low = activeLow;
}

bool Button::active() {
    if (_active_low)
        _last_value = !digitalRead(_pin_num);
    else
        _last_value = !digitalRead(_pin_num);

    return _last_value;
}

// check, but do not save that value
bool Button::changed() {
    bool newVal;
    if (_active_low)
        newVal = !digitalRead(_pin_num);
    else
        newVal = digitalRead(_pin_num);

    return (_last_value != newVal);
}

bool Button::value() {
    return _last_value;
}