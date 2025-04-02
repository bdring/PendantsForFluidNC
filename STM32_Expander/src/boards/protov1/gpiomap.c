// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"

// This table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

pin_t gpios[] = {
    //          port   num      capabilities TIM ch      io_num  usage PWM
    { .gpio = { GPIOB, GPIO_PIN_10, IN | PU | PD, 2, 3 } },  //   0  I0    2-3
    { .gpio = { GPIOB, GPIO_PIN_11, IN | PU | PD, 2, 4 } },  //   1  I1    2-4
    { .gpio = { GPIOB, GPIO_PIN_14, IN | PU | PD, 1, 2 } },  //   2  I2    1-2N
    { .gpio = { GPIOB, GPIO_PIN_15, IN | PU | PD, 1, 3 } },  //   3  I3    1-3N
    { .gpio = { GPIOA, GPIO_PIN_4, IN | PU | PD, 0, 0 } },   //   4  I4    x
    { .gpio = { GPIOA, GPIO_PIN_5, IN | PU | PD, 0, 0 } },   //   5  I5    x
    { .gpio = { GPIOA, GPIO_PIN_11, IN | PU | PD, 1, 4 } },  //   6  I6    1-4
    { .gpio = { GPIOA, GPIO_PIN_12, IN | PU | PD, 0, 0 } },  //   7  I7    x
    { .gpio = { GPIOA, GPIO_PIN_8, OUT | PWM, 1, 1 } },      //   8  O0    1-1
    { .gpio = { GPIOB, GPIO_PIN_6, OUT | PWM, 4, 1 } },      //   9  O1    4-1
    { .gpio = { GPIOB, GPIO_PIN_7, OUT | PWM, 4, 2 } },      //  10  O2    4-2
    { .gpio = { GPIOB, GPIO_PIN_8, OUT | PWM, 4, 3 } },      //  11  O3    4-3
    { .gpio = { GPIOB, GPIO_PIN_9, OUT | PWM, 4, 4 } },      //  12  O4    4-4
    { .gpio = { GPIOA, GPIO_PIN_0, OUT | PWM, 2, 1 } },      //  13  O5    2-1
    { .gpio = { GPIOA, GPIO_PIN_1, OUT | PWM, 2, 2 } },      //  14  O6    2-2
    { .gpio = { GPIOA, GPIO_PIN_6, OUT | PWM, 3, 1 } },      //  15  O7    3-1
    { .gpio = { GPIOA, GPIO_PIN_7, OUT | PWM, 3, 2 } },      //  16  FET0  3-2
    { .gpio = { GPIOB, GPIO_PIN_0, OUT | PWM, 3, 3 } },      //  17  FET1  3-3
};

const int n_pins = sizeof(gpios) / sizeof(gpios[0]);
