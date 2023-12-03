// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "gpio_pin.h"

typedef void (*pin_msg_t)(uint8_t pin_num, bool active);

// This file is independent of any particular MCU or board,
// but the details inside the "gpio_pin_t" type depend on the MCU.

enum pin_type_t {
    pin_type_none   = 0,
    pin_type_input  = 1,
    pin_type_output = 2,
    pin_type_PWM    = 3,
};

enum FailCodes {
    fail_none              = 0,  // no problem
    fail_not_initialized   = 1,
    fail_not_capable       = 2,  // Pin cannot be initialized this way
    fail_range             = 3,  // set value out of range
    fail_unknown_parameter = 4,
    fail_invalid_pin       = 5,
};

typedef struct {
    gpio_pin_t gpio;

    bool    initialized;
    bool    active_low;
    uint8_t type;
    int     last_value;
    int     debounce_ms;
    int     last_change_millis;
} pin_t;

void init_pin(uint8_t pin_num);
void force_pin_update(uint8_t pin_num);
void deinit_pin(uint8_t pin_num);
int  set_pin_mode(uint8_t pin_num, pin_mode_t pinmode);
int  set_output(uint8_t pin_num, int32_t numerator, uint32_t denominator);
bool pin_changed(uint8_t pin_num);
void read_pin(pin_msg_t send_msg, uint8_t pin_num);

void init_all_pins();
void update_all_pins();
void deinit_all_pins();
void read_all_pins(pin_msg_t send_msg);

#ifdef __cplusplus
}
#endif
