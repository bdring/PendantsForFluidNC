// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"

// This table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

// clang-format off
pin_t gpios[] = {
    //          port   num      capabilities TIM ch      io_num  usage PWM
    { .gpio = { GPIOA, GPIO_PIN_4,  IN|PU|PD, 0, 0 } }, //   0  I0    
    { .gpio = { GPIOA, GPIO_PIN_5,  IN|PU|PD, 0, 0 } }, //   1  I1    
    { .gpio = { GPIOB, GPIO_PIN_0,  IN|PU|PD, 3, 3 } }, //   2  I2    
    { .gpio = { GPIOB, GPIO_PIN_10, IN|PU|PD, 2, 3 } }, //   3  I3    
    { .gpio = { GPIOA, GPIO_PIN_12, IN|PU|PD, 0, 0 } }, //   4  I4    
    { .gpio = { GPIOB, GPIO_PIN_14, IN|PU|PD, 0, 0 } }, //   5  I5    
    { .gpio = { GPIOB, GPIO_PIN_12, IN|PU|PD, 0, 0 } }, //   6  I6    
    { .gpio = { GPIOB, GPIO_PIN_13, IN|PU|PD, 0, 0 } }, //   7  I7    
    { .gpio = { GPIOA, GPIO_PIN_8,  OUT|PWM,  1, 1 } }, //   8  O0    1-1
    { .gpio = { GPIOA, GPIO_PIN_0,  OUT|PWM,  2, 1 } }, //   9  O1    2-1
    { .gpio = { GPIOA, GPIO_PIN_6,  OUT|PWM,  3, 1 } }, //  10  O2    3-1
    { .gpio = { GPIOB, GPIO_PIN_6,  OUT|PWM,  4, 1 } }, //  11  O3    4-1    
    { .gpio = { GPIOA, GPIO_PIN_11, OUT|PWM,  1, 4 } }, //  12  O4    1-4
    { .gpio = { GPIOB, GPIO_PIN_11, OUT|PWM,  2, 4 } }, //  13  O5    2-4
    { .gpio = { GPIOB, GPIO_PIN_1,  OUT|PWM,  3, 4 } }, //  14  O6    3-4
    { .gpio = { GPIOB, GPIO_PIN_9,  OUT|PWM,  4, 4 } }, //  15  O7    4-4    
    { .gpio = { GPIOA, GPIO_PIN_1,  OUT|PWM,  2, 2 } }, //  16  FET0  2-2
    { .gpio = { GPIOA, GPIO_PIN_7,  OUT|PWM,  3, 3 } }, //  17  FET1  3-2
};

const int n_pins = sizeof(gpios) / sizeof(gpios[0]);
