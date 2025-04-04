// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "pin.h"

const char* board_name = "Airedale v1.1";

// This table maps io numbers (the table index) to
// device-specific and board-specific pin numbers

// clang-format off
pin_t gpios[] = {
    //          port   num        capabilities TIM ch     io_num  usage
    { .gpio = { GPIOA, GPIO_PIN_4,  IN|PU|PD,   0, 0 } }, //   0  I0
    { .gpio = { GPIOA, GPIO_PIN_5,  IN|PU|PD,   0, 0 } }, //   1  I1
    { .gpio = { GPIOB, GPIO_PIN_0,  IN|PU|PD,   3, 3 } }, //   2  I2
    { .gpio = { GPIOB, GPIO_PIN_10, IN|PU|PD,   2, 3 } }, //   3  I3
    { .gpio = { GPIOA, GPIO_PIN_12, IN|PU|PD,   0, 0 } }, //   4  I4
    { .gpio = { GPIOB, GPIO_PIN_14, IN|PU|PD,   0, 0 } }, //   5  I5
    { .gpio = { GPIOB, GPIO_PIN_12, IN|PU|PD,   0, 0 } }, //   6  I6
    { .gpio = { GPIOB, GPIO_PIN_13, IN|PU|PD,   0, 0 } }, //   7  I7
    { .gpio = { GPIOA, GPIO_PIN_8,  OUT|PWM,    1, 1 } }, //   8  O0
    { .gpio = { GPIOA, GPIO_PIN_0,  OUT|PWM,    2, 1 } }, //   9  O1
    { .gpio = { GPIOA, GPIO_PIN_6,  OUT|PWM,    3, 1 } }, //  10  O2
    { .gpio = { GPIOB, GPIO_PIN_6,  OUT|PWM,    4, 1 } }, //  11  O3
    { .gpio = { GPIOA, GPIO_PIN_11, OUT|PWM,    1, 4 } }, //  12  O4
    { .gpio = { GPIOB, GPIO_PIN_11, OUT|PWM,    2, 4 } }, //  13  O5
    { .gpio = { GPIOB, GPIO_PIN_1,  OUT|PWM,    3, 4 } }, //  14  O6
    { .gpio = { GPIOB, GPIO_PIN_9,  OUT|PWM,    4, 4 } }, //  15  O7
    { .gpio = { GPIOA, GPIO_PIN_1,  OUT|PWM,    2, 2 } }, //  16  FET0
    { .gpio = { GPIOA, GPIO_PIN_7,  OUT|PWM,    3, 2 } }, //  17  FET1
    { .gpio = { GPIOB, GPIO_PIN_5,  OUT,        0, 0 } }, //  18  RGB_LED RED
    { .gpio = { GPIOB, GPIO_PIN_4,  OUT,        0, 0 } }, //  19  RGB_LED GREEN
    { .gpio = { GPIOB, GPIO_PIN_3,  OUT,        0, 0 } }, //  20  RGB_LED BLUE
};
// PB3, PB4, and PB5 can do PWM but it requires AF remapping which we
// do not support

const int n_pins = sizeof(gpios) / sizeof(gpios[0]);

#define RED_LED 18
#define GREEN_LED 19
#define BLUE_LED 20

static void init_leds() {
    set_pin_mode(18, PIN_OUTPUT|PIN_ACTIVELOW);
    set_pin_mode(19, PIN_OUTPUT|PIN_ACTIVELOW);
    set_pin_mode(20, PIN_OUTPUT|PIN_ACTIVELOW);
}
static void set_leds(bool red, bool green, bool blue) {
    set_output(RED_LED, red, 0);
    set_output(GREEN_LED, green, 0);
    set_output(BLUE_LED, blue, 0);
}

void expander_rst() {
    deinit_all_pins();
    init_leds();
    set_leds(0,0,1); // Blue
}

void ready() {
    init_leds();
    set_leds(1,1,1); // White
}
