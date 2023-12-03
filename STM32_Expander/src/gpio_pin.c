// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "gpio_pin.h"

int set_gpio(gpio_pin_t* gpio, bool high) {
    GPIO_PinState pinstate = high ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(gpio->port, gpio->pin_num, pinstate);
    return true;
}
bool get_gpio(gpio_pin_t* gpio) {
    GPIO_PinState pinval = HAL_GPIO_ReadPin(gpio->port, gpio->pin_num);
    return pinval == GPIO_PIN_SET;
}
int set_pwm(gpio_pin_t* gpio, int32_t numerator, uint32_t denominator) {
    // uint32_t pwm_val = 255 * numerator / denominator; // map 0.0-100.0 to 0 to 255
    //        analogWrite(stm_pin_num, pwm_val);
    return true;
}
void deinit_gpio(gpio_pin_t* gpio) {
    HAL_GPIO_DeInit(gpio->port, gpio->pin_num);
}
bool set_gpio_mode(gpio_pin_t* gpio, pin_mode_t pinmode) {
    GPIO_InitTypeDef gpiomode;
    gpiomode.Pin   = gpio->pin_num;
    gpiomode.Speed = GPIO_SPEED_FREQ_HIGH;

    if (pinmode & PIN_OUTPUT) {
        gpiomode.Mode = GPIO_MODE_OUTPUT_PP;
        gpiomode.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(gpio->port, &gpiomode);

        GPIO_PinState pinstate = (pinmode & PIN_ACTIVELOW) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(gpio->port, gpio->pin_num, pinstate);
        return true;
    }
    if (pinmode & PIN_PWM) {
        if (!gpio->pwm_capable) {
            return false;
        }
        gpiomode.Pull = GPIO_NOPULL;
        gpiomode.Mode = GPIO_MODE_ANALOG;
        HAL_GPIO_Init(gpio->port, &gpiomode);
        return true;
    }
    if (pinmode & PIN_INPUT) {
        gpiomode.Mode = GPIO_MODE_INPUT;
        if (pinmode & PIN_PULLUP) {
            gpiomode.Pull = GPIO_PULLUP;
        } else if (pinmode & PIN_PULLDOWN) {
            gpiomode.Pull = GPIO_PULLDOWN;
        } else {
            gpiomode.Pull = GPIO_NOPULL;
        }
        HAL_GPIO_Init(gpio->port, &gpiomode);

        // attachInterrupt(digitalPinToInterrupt(stm_pin_num), pin_interrupt, CHANGE);
        return true;
    }
    return false;
}
