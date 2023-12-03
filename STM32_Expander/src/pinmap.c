// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"

// This table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

// This version is for STM32 port/pin numbers, with the
// pin order of the bdring STM Expander board.
pin_t pins[] = {
    //           port num  can_PWM?   io_num
    { .gpio = { GPIOA,  4, false } }, //   0
    { .gpio = { GPIOA,  5, false } }, //   1
    { .gpio = { GPIOA,  8, true  } }, //   2
    { .gpio = { GPIOA, 11, false } }, //   3
    { .gpio = { GPIOA, 12, false } }, //   4
    { .gpio = { GPIOB,  6, true  } }, //   5
    { .gpio = { GPIOB,  7, true  } }, //   6
    { .gpio = { GPIOB,  8, true  } }, //   7
    { .gpio = { GPIOB,  9, true  } }, //   8
    { .gpio = { GPIOB, 10, false } }, //   9
    { .gpio = { GPIOB, 11, false } }, //  10
    { .gpio = { GPIOB, 14, false } }, //  11
    { .gpio = { GPIOB, 15, false } }, //  12
    { .gpio = { GPIOC, 13, false } }, //  13
    { .gpio = { GPIOA,  0, true  } }, //  14
    { .gpio = { GPIOA,  1, true  } }, //  15
    { .gpio = { GPIOA,  6, true  } }, //  16
    { .gpio = { GPIOA,  7, true  } }, //  17
    { .gpio = { GPIOB,  0, true  } }, //  18
    { .gpio = { GPIOB,  1, true  } }, //  19
};

const int n_pins = sizeof(pins) / sizeof(pins[0]);
