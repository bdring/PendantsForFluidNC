// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>

class Button {
public:
    void init(uint8_t pin_num, bool activeLow);
    bool read();
    bool changed(bool& value);

private:
    enum debounce_state {
        STABLE      = 0,
        DEGLITCHING = 1,
        DEBOUNCING  = 2,
    };
    const int DEGLITCH_MS = 1;
    const int DEBOUNCE_MS = 30;
    uint8_t   _state;
    uint8_t   _pin_num;
    bool      _active_low;
    bool      _last_value = false;
    int32_t   _end_time   = 0;
    bool      waiting();
    bool      pin_changed();
};
