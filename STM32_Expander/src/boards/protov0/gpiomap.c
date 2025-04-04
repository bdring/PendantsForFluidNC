// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"

// This table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

// clang-format off
pin_t gpios[] = {
    //          port   num          capabilities   io_num
    { .gpio = { GPIOA, GPIO_PIN_4,  IN|PU|PD } },  //   0
    { .gpio = { GPIOA, GPIO_PIN_5,  IN|PU|PD } },  //   1
    { .gpio = { GPIOA, GPIO_PIN_8,  OUT|PWM  } },  //   2
    { .gpio = { GPIOA, GPIO_PIN_11, IN|PU|PD } },  //   3
    { .gpio = { GPIOA, GPIO_PIN_12, IN|PU|PD } },  //   4
    { .gpio = { GPIOB, GPIO_PIN_6,  OUT|PWM  } },  //   5
    { .gpio = { GPIOB, GPIO_PIN_7,  OUT|PWM  } },  //   6
    { .gpio = { GPIOB, GPIO_PIN_8,  OUT|PWM  } },  //   7
    { .gpio = { GPIOB, GPIO_PIN_9,  OUT|PWM  } },  //   8
    { .gpio = { GPIOB, GPIO_PIN_10, IN|PU|PD } },  //   9
    { .gpio = { GPIOB, GPIO_PIN_11, IN|PU|PD } },  //  10
    { .gpio = { GPIOB, GPIO_PIN_14, IN|PU|PD } },  //  11
    { .gpio = { GPIOB, GPIO_PIN_15, IN|PU|PD } },  //  12
    { .gpio = { GPIOC, GPIO_PIN_13, IN|PU|PD } },  //  13
    { .gpio = { GPIOA, GPIO_PIN_0,  OUT|PWM  } },  //  14
    { .gpio = { GPIOA, GPIO_PIN_1,  OUT|PWM  } },  //  15
    { .gpio = { GPIOA, GPIO_PIN_6,  OUT|PWM  } },  //  16
    { .gpio = { GPIOA, GPIO_PIN_7,  OUT|PWM  } },  //  17
    { .gpio = { GPIOB, GPIO_PIN_0,  OUT|PWM  } },  //  18
    { .gpio = { GPIOB, GPIO_PIN_1,  OUT|PWM  } },  //  19
};

const int n_pins = sizeof(gpios) / sizeof(gpios[0]);
