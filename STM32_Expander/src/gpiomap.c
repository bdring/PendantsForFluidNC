// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"

// This table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

// This version is for STM32 port/pin numbers, with the
// pin order of the bdring STM Expander board.
pin_t gpios[] = {
    //          port   num         can_PWM?     io_num
    { .gpio = { GPIOA, GPIO_PIN_4, false } },   //   0
    { .gpio = { GPIOA, GPIO_PIN_5, false } },   //   1
    { .gpio = { GPIOA, GPIO_PIN_8, true } },    //   2
    { .gpio = { GPIOA, GPIO_PIN_11, false } },  //   3
    { .gpio = { GPIOA, GPIO_PIN_12, false } },  //   4
    { .gpio = { GPIOB, GPIO_PIN_6, true } },    //   5
    { .gpio = { GPIOB, GPIO_PIN_7, true } },    //   6
    { .gpio = { GPIOB, GPIO_PIN_8, true } },    //   7
    { .gpio = { GPIOB, GPIO_PIN_9, true } },    //   8
    { .gpio = { GPIOB, GPIO_PIN_10, false } },  //   9
    { .gpio = { GPIOB, GPIO_PIN_11, false } },  //  10
    { .gpio = { GPIOB, GPIO_PIN_14, false } },  //  11
    { .gpio = { GPIOB, GPIO_PIN_15, false } },  //  12
    { .gpio = { GPIOC, GPIO_PIN_13, false } },  //  13
    { .gpio = { GPIOA, GPIO_PIN_0, true } },    //  14
    { .gpio = { GPIOA, GPIO_PIN_1, true } },    //  15
    { .gpio = { GPIOA, GPIO_PIN_6, true } },    //  16
    { .gpio = { GPIOA, GPIO_PIN_7, true } },    //  17
    { .gpio = { GPIOB, GPIO_PIN_0, true } },    //  18
    { .gpio = { GPIOB, GPIO_PIN_1, true } },    //  19
};

const int n_pins = sizeof(gpios) / sizeof(gpios[0]);
