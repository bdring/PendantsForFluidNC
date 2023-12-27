// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Button.h"

const int debounce_ms = 10;

void Button::init(uint8_t pin, bool active_low) {
    _pin_num    = pin;
    _active_low = active_low;
    pinMode(_pin_num, INPUT_PULLUP);
    _last_value = read();
    _state      = STABLE;
}

bool Button::read() {
    return digitalRead(_pin_num) ^ _active_low;
}

// Waiting from the time to expire
bool Button::waiting() {
    return ((int32_t)millis() - _end_time) < 0;
}
bool Button::pin_changed() {
    bool new_value;
    new_value = read();
    if (new_value != _last_value) {
        _last_value = new_value;
        return true;
    }
    return false;
}

bool Button::changed(bool& value) {
    switch (_state) {
        case STABLE:
            if (pin_changed()) {
                _end_time = (int32_t)millis() + DEGLITCH_MS;
                _state    = DEGLITCHING;
            }
            return false;
        case DEGLITCHING:
            if (waiting()) {
                return false;
            }
            if (pin_changed()) {
                // The pin changed since the start of the deglitch period
                // so it was a glitch.
                _state = STABLE;
                return false;
            }
            // The value is the same as at the start of the deglitch period
            // so it is a true change
            _end_time = (int32_t)millis() + DEBOUNCE_MS;
            _state    = DEBOUNCING;
            value     = _last_value;
            return true;
        case DEBOUNCING:
            if (waiting()) {
                return false;
            }
            // We always return to idle at the end of a debounce
            _state = STABLE;

            if (pin_changed()) {
                // The pin changed since the start of the debounce period
                // so the button is not being held
                value = _last_value;
                return true;
            }
            // The button is at the same state as at the end of the deglitch
            // period.  Since we have returned to STABLE state, on the next
            // pin change the state machine will start again and possibly
            // report the change after a deglitch delay.
            return false;
    }
    return false;
}
