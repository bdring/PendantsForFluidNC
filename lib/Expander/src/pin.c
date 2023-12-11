// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"
#include "gpiomap.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int milliseconds();

void init_pin(uint8_t pin_num) {
    if (pin_num >= n_pins) {
        return;
    }
    pin_t* pin              = &gpios[pin_num];
    pin->initialized        = false;
    pin->active_low         = false;
    pin->type               = pin_type_none;
    pin->last_value         = -1;   // unknown
    pin->debounce_ms        = 100;  // default
    pin->last_change_millis = 0;
}

int set_output(uint8_t pin_num, int32_t numerator, uint32_t denominator) {
    if (pin_num >= n_pins) {
        return fail_invalid_pin;
    }
    pin_t* pin = &gpios[pin_num];
    if (!pin->initialized) {
        return fail_not_initialized;
    }

    if (pin->type == pin_type_output) {
        bool is_high = (numerator != 0) ^ pin->active_low;
        set_gpio(&pin->gpio, is_high);
        return fail_none;
    }
    if (pin->type == pin_type_PWM) {
        if (numerator < 0 || (uint32_t)numerator > denominator) {
            return fail_range;
        }
        set_pwm(&pin->gpio, numerator, denominator);
        return fail_none;
    }
    return fail_not_capable;
}

bool pin_changed(uint8_t pin_num) {  // return true if value has changed
    if (pin_num >= n_pins) {
        return false;
    }
    pin_t* pin = &gpios[pin_num];

    if (pin->type != pin_type_input) {
        return false;
    }

    int new_value = get_gpio(&pin->gpio) ^ pin->active_low;

    if (new_value != pin->last_value) {
        if ((int)(milliseconds() - pin->last_change_millis) > (int)pin->debounce_ms) {
            pin->last_value         = new_value;
            pin->last_change_millis = milliseconds();  // maybe use for debouncing
            return true;
        }
    }

    return false;
}
void force_pin_update(uint8_t pin_num) {
    if (pin_num >= n_pins) {
        return;
    }
    pin_t* pin      = &gpios[pin_num];
    pin->last_value = -1;
}
void deinit_pin(uint8_t pin_num) {
    if (pin_num >= n_pins) {
        return;
    }
    pin_t* pin = &gpios[pin_num];
    if (pin->initialized) {
        pin->initialized        = false;
        pin->active_low         = false;
        pin->type               = pin_type_none;
        pin->last_value         = -1;   // unknown
        pin->debounce_ms        = 100;  // default
        pin->last_change_millis = 0;
        deinit_gpio(&pin->gpio);
    }
}

int set_pin_mode(uint8_t pin_num, pin_mode_t pinmode) {
    if (pin_num >= n_pins) {
        return fail_invalid_pin;
    }
    pin_t* pin = &gpios[pin_num];

    // for now we assume all pins can input and output. Some can do PWM

    pin->active_low = pinmode & PIN_ACTIVELOW;

    if (pinmode & PIN_OUTPUT) {
        pin->type = pin_type_output;
    } else if (pinmode & PIN_PWM) {
        pin->type = pin_type_PWM;
    } else if (pinmode & PIN_INPUT) {
        pin->last_value = -1;  // reset to unknown value
        pin->type       = pin_type_input;
    } else {
        return fail_unknown_parameter;
    }
    if (set_gpio_mode(&pin->gpio, pinmode)) {
        pin->initialized = true;
        return fail_none;
    }
    return fail_not_capable;
}

void init_all_pins() {
    for (size_t pin_num = 0; pin_num < n_pins; pin_num++) {
        init_pin(pin_num);
    }
}
void update_all_pins() {
    for (size_t pin_num = 0; pin_num < n_pins; pin_num++) {
        force_pin_update(pin_num);
    }
}
void deinit_all_pins() {
    for (size_t pin_num = 0; pin_num < n_pins; pin_num++) {
        deinit_pin(pin_num);
    }
}
void read_pin(pin_msg_t send_msg, uint8_t pin_num) {
    if (pin_num >= n_pins) {
        return;
    }
    pin_t* pin = &gpios[pin_num];
    if (pin->type == pin_type_input) {
        if (pin_changed(pin_num)) {
            send_msg(pin_num, pin->last_value == 1);
        }
    }
}
void read_all_pins(pin_msg_t send_msg) {
    for (size_t pin_num = 0; pin_num < n_pins; pin_num++) {
        read_pin(send_msg, pin_num);
    }
}

#ifdef __cplusplus
}
#endif
