#include <Arduino.h>

class Button {
public:
    void init(uint8_t pin_num, bool activeLow);
    bool active();
    bool changed();
    bool value();

private:
    uint8_t _pin_num;
    bool _active_low;
    bool    _last_value;
    bool    _changed;
};