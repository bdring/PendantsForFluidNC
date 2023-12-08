#include "Button.h"

void Button::init(uint8_t pin, bool activeLow) {
    pinMode(pin, INPUT_PULLUP);
    _pin_num    = pin;
    _last_value = active();
    _active_low = activeLow;
}

bool Button::active() {
    _last_value = !digitalRead(_pin_num) ^ _active_low;

    return _last_value;
}

// check, but do not save that value
bool Button::changed() {
    bool newVal = digitalRead(_pin_num) ^ _active_low;

    return (_last_value != newVal);
}

bool Button::value() {
    return _last_value;
}
