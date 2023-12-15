// Copyright (c) 2023 - Barton Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <Arduino.h>

class Button {
public:
    void init(uint8_t pin_num, bool activeLow);
    bool read();
    bool changed(bool& value);

private:
    uint8_t _pin_num;
    bool    _active_low;
    bool    _last_value = false;
    bool    _delaying   = false;
    int32_t _timestamp  = 0;
};
