// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

// This file implements the low-level interface to GPIO pins that is used by
// the intermediate-level interface defined in lib/Expander/src/pin.{c,h}
// The intention is to encapsulate the platform-dependent GPIO API as tightly
// as possible.
// This implementation is for the STM32 HAL driver.

#include "gpio_pin.h"
#include "pwm_pin.h"
#include "gpiomap.h"

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
    PWM_Duty(gpio, numerator);
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
        if (!(gpio->capabilities & OUT)) {
            return false;
        }
        gpiomode.Mode = GPIO_MODE_OUTPUT_PP;
        gpiomode.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(gpio->port, &gpiomode);

        // The initial state of an output pin is its inactive state
        GPIO_PinState pinstate = (pinmode & PIN_ACTIVELOW) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(gpio->port, gpio->pin_num, pinstate);
        return true;
    }
    if (pinmode & PIN_PWM) {
        if (!(gpio->capabilities & PWM)) {
            return false;
        }
        return PWM_Init(gpio, pinmode >> PIN_FREQ_SHIFT, pinmode & PIN_ACTIVELOW);
        // gpiomode.Pull = GPIO_NOPULL;
        // gpiomode.Mode = GPIO_MODE_ANALOG;
        // HAL_GPIO_Init(gpio->port, &gpiomode);
    }
    if (pinmode & PIN_INPUT) {
        if (!(gpio->capabilities & IN)) {
            return false;
        }
        gpiomode.Mode = GPIO_MODE_INPUT;
        if (pinmode & PIN_PULLUP) {
            if (!(gpio->capabilities & PU)) {
                return false;
            }
            gpiomode.Pull = GPIO_PULLUP;
        } else if (pinmode & PIN_PULLDOWN) {
            if (!(gpio->capabilities & PD)) {
                return false;
            }
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
void gpio_clock_enable(GPIO_TypeDef* port) {
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
}

void init_gpio(gpio_pin_t* gpio) {
    gpio_clock_enable(gpio->port);
    if (gpio->capabilities & OUT) {
        set_gpio_mode(gpio, PIN_OUTPUT);
    } else if (gpio->capabilities & IN) {
        set_gpio_mode(gpio, PIN_INPUT);
    }
}
void init_from_gpiomap() {
    for (int i = 0; i < n_pins; i++) {
        gpio_pin_t* gpio = &(gpios[i].gpio);
        if (gpio->capabilities & (IN | OUT)) {
            init_gpio(gpio);
        }
    }
}
