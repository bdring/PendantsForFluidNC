// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once
#include <stdbool.h>
#include "pinmode.h"
#include "stm32f1xx_hal.h"

// The internals of this struct are MCU-specific
typedef struct {
    // Implementation for STM32 HAL
    GPIO_TypeDef* port;
    uint16_t      pin_num;
    uint8_t       capabilities;
    uint8_t       timer_num;
    uint8_t       timer_channel;
} gpio_pin_t;

// This API is MCU-independent
int  set_gpio(gpio_pin_t* gpio, bool high);
bool get_gpio(gpio_pin_t* gpio);
int  set_pwm(gpio_pin_t* gpio, int32_t numerator, uint32_t denominator);
void deinit_gpio(gpio_pin_t* gpio);
bool set_gpio_mode(gpio_pin_t* gpio, pin_mode_t pinmode);
