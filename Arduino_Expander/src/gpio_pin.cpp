// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// This file implements the low-level interface to GPIO pins that is used by
// the intermediate-level interface defined in lib/Expander/src/pin.{c,h}
// The intention is to encapsulate the platform-dependent GPIO API as tightly
// as possible.
// This implementation is for the Arduino framework

#include "gpio_pin.h"
#include "gpiomap.h"

#ifdef __cplusplus
extern "C" {
#endif

int set_gpio(gpio_pin_t* gpio, bool high) {
    digitalWrite(gpio->pin_num, high);
    return true;
}
bool get_gpio(gpio_pin_t* gpio) {
    return digitalRead(gpio->pin_num);
}
int set_pwm(gpio_pin_t* gpio, int32_t numerator, uint32_t denominator) {
    // uint32_t pwm_val = 255 * numerator / denominator; // map 0.0-100.0 to 0 to 255
    //        analogWrite(stm_pin_num, pwm_val);
    return true;
}
void deinit_gpio(gpio_pin_t* gpio) {
    pinMode(gpio->pin_num, INPUT);
}
bool set_gpio_mode(gpio_pin_t* gpio, pin_mode_t pinmode) {
    if (pinmode & PIN_OUTPUT) {
        pinMode(gpio->pin_num, OUTPUT);

        digitalWrite(gpio->pin_num, !!(pinmode & PIN_ACTIVELOW));
    }
    if (pinmode & PIN_PWM) {
        if (!gpio->pwm_capable) {
            return false;
        }
#ifdef INPUT_ANALOG
        pinMode(gpio->pin_num, INPUT_ANALOG);
        return true;
#else
        return false;
#endif
    }
    if (pinmode & PIN_INPUT) {
        uint32_t mode;
        if (pinmode & PIN_PULLUP) {
            mode = INPUT_PULLUP;
        } else if (pinmode & PIN_PULLDOWN) {
#ifdef INPUT_PULLDOWN
            mode = INPUT_PULLDOWN;
#else
            return false;
#endif
        } else {
            mode = INPUT;
        }
        pinMode(gpio->pin_num, mode);
        return true;
    }
    return false;
}

#ifdef __cplusplus
}
#endif
